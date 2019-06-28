package com.sequoiadb.transaction.rc;

import java.util.ArrayList;
import java.util.Collections;

import org.bson.BSONObject;
import org.bson.util.JSON;
import org.testng.Assert;
import org.testng.SkipException;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.DataProvider;
import org.testng.annotations.Test;

import com.sequoiadb.base.CollectionSpace;
import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.DBCursor;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.testcommon.CommLib;
import com.sequoiadb.testcommon.SdbTestBase;
import com.sequoiadb.testcommon.SdbThreadBase;
import com.sequoiadb.transaction.TransUtils;

/**
 * @FileName: seqDB-17106:删除并发，事务回滚，过程中读 删除走索引
 * @Author zhaoyu
 * @Date 2019-01-23
 * @Version 1.00
 */
@Test(groups = "rc")
public class Transaction17106 extends SdbTestBase {
    private Sequoiadb sdb = null;
    private String clName = "cl_17106";
    private Sequoiadb db1;
    private Sequoiadb db2;
    private Sequoiadb db3;
    private DBCollection cl = null;
    private DBCollection cl1 = null;
    private DBCollection cl2 = null;
    private DBCollection cl3 = null;
    private ArrayList<BSONObject> expList = new ArrayList<BSONObject>();
    private ArrayList<BSONObject> actList = new ArrayList<BSONObject>();
    private DBCursor cursor = null;
    private String hint = null;
    private int startId = 0;
    private int stopId = 1000;

    @DataProvider(name = "index")
    public Object[][] createIndex() {
        return new Object[][] { { "{'a': 1}", clName + "hash" }, { "{'a': -1, 'b': 1}", clName + "mainCL" }, };
    }

    @BeforeClass
    public void setUp() {
        sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "");
        if (CommLib.isStandAlone(sdb)) {
            throw new SkipException("STANDALONE MODE");
        }
        if (CommLib.OneGroupMode(sdb)) {
            throw new SkipException("ONE GROUP MODE");
        }
        sdb.getCollectionSpace(csName).createCollection(clName + "hash",
                (BSONObject) JSON.parse("{ShardingKey:{b:1}, ShardingType:'hash', AutoSplit:true}"));

        DBCollection mainCL = sdb.getCollectionSpace(csName).createCollection(clName + "mainCL",
                (BSONObject) JSON.parse("{ShardingKey:{b:1}, ShardingType:'range', IsMainCL:true}"));
        sdb.getCollectionSpace(csName).createCollection("sub117106");
        sdb.getCollectionSpace(csName).createCollection("sub217106",
                (BSONObject) JSON.parse("{ShardingKey:{b:1}, ShardingType:'hash', AutoSplit:true}"));
        mainCL.attachCollection(csName + ".sub117106",
                (BSONObject) JSON.parse("{LowBound:{b:{'$minKey':1}}, UpBound:{b:500}}"));
        mainCL.attachCollection(csName + ".sub217106",
                (BSONObject) JSON.parse("{LowBound:{b:500}, UpBound:{b:{'$maxKey':1}}}"));
    }

    @AfterClass
    public void tearDown() {
        // 关闭所有游标
        sdb.closeAllCursors();
        db1.closeAllCursors();
        db2.closeAllCursors();
        db3.closeAllCursors();

        // 先关闭事务连接，再删除集合
        CollectionSpace cs = sdb.getCollectionSpace(csName);
        if (!db1.isClosed()) {
            db1.close();
        }
        if (!db2.isClosed()) {
            db2.close();
        }
        if (!db3.isClosed()) {
            db3.close();
        }
        if (cs.isCollectionExist(clName + "hash")) {
            cs.dropCollection(clName + "hash");
        }
        if (cs.isCollectionExist(clName + "mainCL")) {
            cs.dropCollection(clName + "mainCL");
        }
        if (sdb != null) {
            sdb.close();
        }
    }

    @Test(dataProvider = "index")
    public void test(String indexKey, String clName) {
        try {
            cl = sdb.getCollectionSpace(csName).getCollection(clName);
            cl.createIndex("a", indexKey, false, false);

            db1 = new Sequoiadb(SdbTestBase.coordUrl, "", "");
            db2 = new Sequoiadb(SdbTestBase.coordUrl, "", "");
            db3 = new Sequoiadb(SdbTestBase.coordUrl, "", "");

            // 开启3个并发事务
            db1.beginTransaction();
            db2.beginTransaction();
            db3.beginTransaction();
            cl1 = db1.getCollectionSpace(csName).getCollection(clName);
            cl2 = db2.getCollectionSpace(csName).getCollection(clName);
            cl3 = db3.getCollectionSpace(csName).getCollection(clName);

            // 1 插入记录R1
            ArrayList<BSONObject> insertR1s = TransUtils.insertRandomDatas(cl, startId, stopId);

            // 2 事务1匹配R1删除
            hint = "{\"\":\"a\"}";
            cl1.delete(null, hint);

            // 3 事务2匹配R1删除
            DeleteThread deleteThread = new DeleteThread();
            deleteThread.start();
            Assert.assertTrue(deleteThread.matchBlockingMethod(cl2.getClass().getName(), "delete"));

            // 4 事务1记录读
            expList.clear();
            hint = "{\"\":null}";
            cursor = cl1.query(null, null, "{a:1}", hint);
            actList = TransUtils.getReadActList(cursor);
            Assert.assertEquals(actList, expList);
            actList.clear();

            // 事务1索引读
            hint = "{\"\":\"a\"}";
            cursor = cl1.query(null, null, "{a:1}", hint);
            actList = TransUtils.getReadActList(cursor);
            Assert.assertEquals(actList, expList);
            actList.clear();

            // 4 事务1记录逆序读
            hint = "{\"\":null}";
            cursor = cl1.query(null, null, "{a: -1}", hint);
            actList = TransUtils.getReadActList(cursor);
            Assert.assertEquals(actList, expList);
            actList.clear();

            // 事务1索引逆序读
            hint = "{\"\":\"a\"}";
            cursor = cl1.query(null, null, "{a: -1}", hint);
            actList = TransUtils.getReadActList(cursor);
            Assert.assertEquals(actList, expList);
            actList.clear();

            // 5 事务3记录读
            expList.addAll(insertR1s);
            hint = "{\"\":null}";
            cursor = cl3.query(null, null, "{a:1}", hint);
            actList = TransUtils.getReadActList(cursor);
            Assert.assertEquals(actList, expList);
            actList.clear();

            // 事务3索引读
            hint = "{\"\":\"a\"}";
            cursor = cl3.query(null, null, "{a:1}", hint);
            actList = TransUtils.getReadActList(cursor);
            Assert.assertEquals(actList, expList);
            actList.clear();

            // 5 事务3记录逆序读
            Collections.reverse(expList);
            hint = "{\"\":null}";
            cursor = cl3.query(null, null, "{a: -1}", hint);
            actList = TransUtils.getReadActList(cursor);
            Assert.assertEquals(actList, expList);
            actList.clear();

            // 事务3索引逆序读
            hint = "{\"\":\"a\"}";
            cursor = cl3.query(null, null, "{a: -1}", hint);
            actList = TransUtils.getReadActList(cursor);
            Assert.assertEquals(actList, expList);
            actList.clear();

            // 6 非事务记录读
            expList.clear();
            hint = "{\"\":null}";
            cursor = cl.query(null, null, "{a:1}", hint);
            actList = TransUtils.getReadActList(cursor);
            Assert.assertEquals(actList, expList);
            actList.clear();

            // 非事务索引读
            hint = "{\"\":\"a\"}";
            cursor = cl.query(null, null, "{a:1}", hint);
            actList = TransUtils.getReadActList(cursor);
            Assert.assertEquals(actList, expList);
            actList.clear();

            // 6 非事务记录逆序读
            hint = "{\"\":null}";
            cursor = cl.query(null, null, "{a: -1}", hint);
            actList = TransUtils.getReadActList(cursor);
            Assert.assertEquals(actList, expList);
            actList.clear();

            // 非事务索引逆序读
            hint = "{\"\":\"a\"}";
            cursor = cl.query(null, null, "{a: -1}", hint);
            actList = TransUtils.getReadActList(cursor);
            Assert.assertEquals(actList, expList);
            actList.clear();

            // 7 回滚事务1
            db1.rollback();
            Assert.assertTrue(deleteThread.isSuccess(), deleteThread.getErrorMsg());

            // 7 事务2记录读
            hint = "{\"\":null}";
            cursor = cl2.query(null, null, "{a:1}", hint);
            actList = TransUtils.getReadActList(cursor);
            Assert.assertEquals(actList, expList);
            actList.clear();

            // 事务2索引读
            hint = "{\"\":\"a\"}";
            cursor = cl2.query(null, null, "{a:1}", hint);
            actList = TransUtils.getReadActList(cursor);
            Assert.assertEquals(actList, expList);
            actList.clear();

            // 7 事务2记录逆序读
            hint = "{\"\":null}";
            cursor = cl2.query(null, null, "{a: -1}", hint);
            actList = TransUtils.getReadActList(cursor);
            Assert.assertEquals(actList, expList);
            actList.clear();

            // 事务2索引逆序读
            hint = "{\"\":\"a\"}";
            cursor = cl2.query(null, null, "{a: -1}", hint);
            actList = TransUtils.getReadActList(cursor);
            Assert.assertEquals(actList, expList);
            actList.clear();

            // 8 事务3记录读
            expList.addAll(insertR1s);
            hint = "{\"\":null}";
            cursor = cl3.query(null, null, "{a:1}", hint);
            actList = TransUtils.getReadActList(cursor);
            Assert.assertEquals(actList, expList);
            actList.clear();

            // 事务3索引读
            hint = "{\"\":\"a\"}";
            cursor = cl3.query(null, null, "{a:1}", hint);
            actList = TransUtils.getReadActList(cursor);
            Assert.assertEquals(actList, expList);
            actList.clear();

            // 8 事务3记录逆序读
            Collections.reverse(expList);
            hint = "{\"\":null}";
            cursor = cl3.query(null, null, "{a: -1}", hint);
            actList = TransUtils.getReadActList(cursor);
            Assert.assertEquals(actList, expList);
            actList.clear();

            // 事务3索引逆序读
            hint = "{\"\":\"a\"}";
            cursor = cl3.query(null, null, "{a: -1}", hint);
            actList = TransUtils.getReadActList(cursor);
            Assert.assertEquals(actList, expList);
            actList.clear();

            // 9 提交事务2
            db2.commit();

            // 9 非事务记录读
            expList.clear();
            hint = "{\"\":null}";
            cursor = cl.query(null, null, "{a:1}", hint);
            actList = TransUtils.getReadActList(cursor);
            Assert.assertEquals(actList, expList);
            actList.clear();

            // 非事务索引读
            hint = "{\"\":\"a\"}";
            cursor = cl.query(null, null, "{a:1}", hint);
            actList = TransUtils.getReadActList(cursor);
            Assert.assertEquals(actList, expList);
            actList.clear();

            // 9 非事务记录逆序读
            hint = "{\"\":null}";
            cursor = cl.query(null, null, "{a: -1}", hint);
            actList = TransUtils.getReadActList(cursor);
            Assert.assertEquals(actList, expList);
            actList.clear();

            // 非事务索引逆序读
            hint = "{\"\":\"a\"}";
            cursor = cl.query(null, null, "{a: -1}", hint);
            actList = TransUtils.getReadActList(cursor);
            Assert.assertEquals(actList, expList);
            actList.clear();

            // 10 事务3记录读
            hint = "{\"\":null}";
            cursor = cl3.query(null, null, "{a:1}", hint);
            actList = TransUtils.getReadActList(cursor);
            Assert.assertEquals(actList, expList);
            actList.clear();

            // 事务3索引读
            hint = "{\"\":\"a\"}";
            cursor = cl3.query(null, null, "{a:1}", hint);
            actList = TransUtils.getReadActList(cursor);
            Assert.assertEquals(actList, expList);
            actList.clear();

            // 10 事务3记录逆序读
            hint = "{\"\":null}";
            cursor = cl3.query(null, null, "{a: -1}", hint);
            actList = TransUtils.getReadActList(cursor);
            Assert.assertEquals(actList, expList);
            actList.clear();

            // 事务3索引逆序读
            hint = "{\"\":\"a\"}";
            cursor = cl3.query(null, null, "{a: -1}", hint);
            actList = TransUtils.getReadActList(cursor);
            Assert.assertEquals(actList, expList);
            actList.clear();

            // 提交事务3
            db3.commit();
        } finally {
            db1.commit();
            db2.commit();
            db3.commit();
            if (cl.isIndexExist("a")) {
                cl.dropIndex("a");
            }
            cl.truncate();
        }
    }

    private class DeleteThread extends SdbThreadBase {
        @Override
        public void exec() throws BaseException {
            hint = "{\"\":\"a\"}";
            cl2.delete(null, hint);
        }
    }

}
