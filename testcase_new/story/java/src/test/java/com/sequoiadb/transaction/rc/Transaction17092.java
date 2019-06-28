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
import com.sequoiadb.testcommon.CommLib;
import com.sequoiadb.testcommon.SdbTestBase;
import com.sequoiadb.transaction.TransUtils;

/**
 * @FileName:seqDB-17092:插入并发，过程中读
 * @Author zhaoyu
 * @Date 2019-01-16
 * @Version 1.00
 */
@Test(groups = "rc")
public class Transaction17092 extends SdbTestBase {
    private Sequoiadb sdb = null;
    private String clName = "cl_17092";
    private Sequoiadb db1;
    private Sequoiadb db2;
    private Sequoiadb db3;
    private DBCollection cl = null;
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
        sdb.getCollectionSpace(csName).createCollection("sub117092");
        sdb.getCollectionSpace(csName).createCollection("sub217092",
                (BSONObject) JSON.parse("{ShardingKey:{b:1}, ShardingType:'hash', AutoSplit:true}"));
        mainCL.attachCollection(csName + ".sub117092",
                (BSONObject) JSON.parse("{LowBound:{b:{'$minKey':1}}, UpBound:{b:1000}}"));
        mainCL.attachCollection(csName + ".sub217092",
                (BSONObject) JSON.parse("{LowBound:{b:1000}, UpBound:{b:{'$maxKey':1}}}"));
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

            // 1 开启3个并发事务
            db1.beginTransaction();
            db2.beginTransaction();
            db3.beginTransaction();
            DBCollection cl1 = db1.getCollectionSpace(csName).getCollection(clName);
            DBCollection cl2 = db2.getCollectionSpace(csName).getCollection(clName);
            DBCollection cl3 = db3.getCollectionSpace(csName).getCollection(clName);

            // 2 事务1插入记录R1
            ArrayList<BSONObject> insertR1s = TransUtils.insertRandomDatas(cl1, startId, stopId);

            // 3 事务2插入记录R2
            ArrayList<BSONObject> insertR2s = TransUtils.insertRandomDatas(cl2, startId + 1000, stopId + 1000);

            // 4 事务1记录读
            expList.addAll(insertR1s);
            hint = "{\"\":null}";
            cursor = cl1.query(null, null, "{a: 1}", hint);
            actList = TransUtils.getReadActList(cursor);
            cursor.close();
            Assert.assertEquals(actList, expList);
            actList.clear();

            // 事务1索引读
            hint = "{\"\":\"a\"}";
            cursor = cl1.query(null, null, "{a: 1}", hint);
            actList = TransUtils.getReadActList(cursor);
            Assert.assertEquals(actList, expList);
            actList.clear();

            // 事务1记录逆序读
            Collections.reverse(expList);
            hint = "{\"\":null}";
            cursor = cl1.query(null, null, "{a: -1}", hint);
            actList = TransUtils.getReadActList(cursor);
            cursor.close();
            Assert.assertEquals(actList, expList);
            actList.clear();

            // 事务1索引逆序读
            hint = "{\"\":\"a\"}";
            cursor = cl1.query(null, null, "{a: -1}", hint);
            actList = TransUtils.getReadActList(cursor);
            Assert.assertEquals(actList, expList);
            actList.clear();

            // 5 事务2记录读
            expList.clear();
            expList.addAll(insertR2s);
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

            // 事务2记录逆序读
            Collections.reverse(expList);
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

            // 6 事务3记录读
            expList.clear();
            hint = "{\"\":null}";
            cursor = cl3.query(null, null, "{a:1}", hint);
            actList = TransUtils.getReadActList(cursor);
            Assert.assertEquals(actList, expList);
            actList.clear();

            // 事务3索引
            hint = "{\"\":\"a\"}";
            cursor = cl3.query(null, null, "{a:1}", hint);
            actList = TransUtils.getReadActList(cursor);
            Assert.assertEquals(actList, expList);
            actList.clear();

            // 事务3记录逆序读
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

            // 7 非事务记录读
            expList.addAll(insertR1s);
            expList.addAll(insertR2s);
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

            // 非事务记录逆序读
            Collections.reverse(expList);
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

            // 8 提交事务1
            db1.commit();

            // 非事务记录读
            Collections.reverse(expList);
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

            // 非事务记录逆序读
            Collections.reverse(expList);
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

            // 9 事务2记录读
            Collections.reverse(expList);
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

            // 9 事务2记录逆序读
            Collections.reverse(expList);
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

            // 10 事务3记录读
            expList.clear();
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

            // 10 事务3记录逆序读
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

            // 11 提交事务2
            db2.commit();
            expList.clear();
            expList.addAll(insertR1s);
            expList.addAll(insertR2s);

            // 非事务记录读
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

            // 非事务记录读
            Collections.reverse(expList);
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

            // 12 事务3记录读
            Collections.reverse(expList);
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

            // 12 事务3记录逆序读
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

            // 提交事务3
            db3.commit();

            // 删除记录
            cl.delete((BSONObject) null);

            // 非事务记录读
            expList.clear();
            hint = "{\"\":null}";
            cursor = cl.query(null, null, null, hint);
            actList = TransUtils.getReadActList(cursor);
            Assert.assertEquals(actList, expList);
            actList.clear();

            // 非事务索引读
            hint = "{\"\":\"a\"}";
            cursor = cl.query(null, null, null, hint);
            actList = TransUtils.getReadActList(cursor);
            Assert.assertEquals(actList, expList);
            actList.clear();
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

}
