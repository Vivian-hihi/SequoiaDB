package com.sequoiadb.transaction.rc;

import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;

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
import com.sequoiadb.transaction.TransUtils;

/**
 * @FileName: seqDB-17107:删除与插入并发，事务提交，过程中读 更新/删除走索引扫描
 * @Author zhaoyu
 * @Date 2019-01-21
 * @Version 1.00
 */
@Test(groups = "rc")
public class Transaction17107M extends SdbTestBase {
    private Sequoiadb sdb = null;
    private String clName = "cl_17107m";
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
    private int startId2 = 1000;
    private int stopId2 = 2000;

    @DataProvider(name = "index")
    public Object[][] createIndex() {
        return new Object[][] { { "{'a': 1}" }, { "{'a': -1, 'b': 1}" }, };
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

        cl = sdb.getCollectionSpace(csName).createCollection(clName,
                (BSONObject) JSON.parse("{ShardingKey:{b:1}, ShardingType:'range', IsMainCL:true}"));
        sdb.getCollectionSpace(csName).createCollection("sub117107");
        sdb.getCollectionSpace(csName).createCollection("sub217107",
                (BSONObject) JSON.parse("{ShardingKey:{b:1}, ShardingType:'hash', AutoSplit:true}"));
        cl.attachCollection(csName + ".sub117107",
                (BSONObject) JSON.parse("{LowBound:{b:{'$minKey':1}}, UpBound:{b:1000}}"));
        cl.attachCollection(csName + ".sub217107",
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
        if (cs.isCollectionExist(clName)) {
            cs.dropCollection(clName);
        }
        if (!sdb.isClosed()) {
            sdb.close();
        }
    }

    @Test(dataProvider = "index")
    public void test(String indexKey) {
        try {
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

            // 3 事务2插入R2，与R1相同
            ArrayList<BSONObject> insertR2s = this.insertRandomDatas(cl2, startId2, stopId2, 0);

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

            // 5 事务2记录读
            expList.addAll(insertR1s);
            expList.addAll(insertR2s);
            Collections.sort(expList, new OrderBy());
            hint = "{\"\":null}";
            cursor = cl2.query(null, null, "{a:1, b:1}", hint);
            actList = TransUtils.getReadActList(cursor);
            Assert.assertEquals(actList, expList);
            actList.clear();

            // 事务2索引读
            hint = "{\"\":\"a\"}";
            cursor = cl2.query(null, null, "{a:1, b:1}", hint);
            actList = TransUtils.getReadActList(cursor);
            Assert.assertEquals(actList, expList);
            actList.clear();

            // 5 事务2记录逆序读
            Collections.reverse(expList);
            hint = "{\"\":null}";
            cursor = cl2.query(null, null, "{a: -1, b: -1}", hint);
            actList = TransUtils.getReadActList(cursor);
            Assert.assertEquals(actList, expList);
            actList.clear();

            // 事务2索引逆序读
            hint = "{\"\":\"a\"}";
            cursor = cl2.query(null, null, "{a: -1, b: -1}", hint);
            actList = TransUtils.getReadActList(cursor);
            Assert.assertEquals(actList, expList);
            actList.clear();

            // 6 事务3记录读
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

            // 6 事务3记录读
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

            // 7 非事务记录读
            expList.clear();
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

            // 7 非事务记录逆序读
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

            // 8 非事务记录读
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

            // 8 非事务记录逆序读
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

            // 11 提交事务2
            db2.commit();

            // 11 非事务记录读
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

            // 11 非事务记录逆序读
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
            cursor = cl3.query(null, null, "{a: 1}", hint);
            actList = TransUtils.getReadActList(cursor);
            Assert.assertEquals(actList, expList);
            actList.clear();

            // 事务3索引读
            hint = "{\"\":\"a\"}";
            cursor = cl3.query(null, null, "{a: 1}", hint);
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

    private ArrayList<BSONObject> insertRandomDatas(DBCollection cl, int startId, int endId, int startValue)
            throws BaseException {
        ArrayList<BSONObject> insertDatas = new ArrayList<BSONObject>();
        ArrayList<BSONObject> expDatas = new ArrayList<BSONObject>();
        for (int i = startId; i < endId; i++) {
            BSONObject data = (BSONObject) JSON
                    .parse("{_id:" + i + ",a:" + (startValue + i) + ",b:" + (startId + i) + "}");
            insertDatas.add(data);
            expDatas.add(data);
        }
        Collections.shuffle(insertDatas);
        cl.insert(insertDatas);
        return expDatas;
    }

    public class OrderBy implements Comparator<BSONObject> {

        @Override
        public int compare(BSONObject obj1, BSONObject obj2) {
            int flag = 0;
            int a1 = (int) obj1.get("a");
            int b1 = (int) obj1.get("b");
            int a2 = (int) obj2.get("a");
            int b2 = (int) obj2.get("b");
            if (a1 > a2) {
                flag = 1;
            } else if (a1 < a2) {
                flag = -1;
            } else {
                if (b1 > b2) {
                    flag = 1;
                } else if (b1 < b2) {
                    flag = -1;
                }
            }

            return flag;
        }

    }
}
