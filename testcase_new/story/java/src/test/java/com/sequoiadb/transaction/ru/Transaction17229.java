package com.sequoiadb.transaction.ru;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.concurrent.CountDownLatch;

import org.bson.BSONObject;
import org.bson.util.JSON;
import org.testng.Assert;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.DataProvider;
import org.testng.annotations.Test;

import com.sequoiadb.base.CollectionSpace;
import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.DBCursor;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.testcommon.SdbTestBase;
import com.sequoiadb.testcommon.SdbThreadBase;
import com.sequoiadb.transaction.TransUtils;

/**
 * @testcase seqDB-17229:并发读写不同的记录
 * @date 2019-1-16
 * @author yinzhen
 *
 */
@Test(groups = "ru")
public class Transaction17229 extends SdbTestBase {
    private Sequoiadb sdb = null;
    private String clName = "cl17229";
    private DBCollection cl = null;
    private Sequoiadb db1 = null;
    private Sequoiadb db2 = null;
    private Sequoiadb db3 = null;
    private Sequoiadb db4 = null;
    private Sequoiadb db5 = null;
    private DBCollection cl1 = null;
    private DBCollection cl2 = null;
    private DBCollection cl3 = null;
    private DBCollection cl4 = null;
    private DBCollection cl5 = null;
    private CountDownLatch latch = null;

    @BeforeClass
    public void setUp() {
        sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "");
    }

    @AfterClass
    public void tearDown() {
        if (!db1.isClosed()) {
            db1.close();
        }
        if (!db2.isClosed()) {
            db2.close();
        }
        if (!db3.isClosed()) {
            db3.close();
        }
        if (!db4.isClosed()) {
            db4.close();
        }
        if (!db5.isClosed()) {
            db5.close();
        }
        CollectionSpace cs = sdb.getCollectionSpace(csName);
        if (cs.isCollectionExist(clName)) {
            cs.dropCollection(clName);
        }
        if (!sdb.isClosed()) {
            sdb.close();
        }
    }

    @DataProvider(name = "index")
    public Object[][] createIndex() {
        return new Object[][] { { "{'a':-1}" }, { "{'a':1}" } };
    }

    @Test(dataProvider="index")
    public void test(String indexKey) {
        latch = new CountDownLatch(4);
        cl = sdb.getCollectionSpace(csName).createCollection(clName);
        cl.createIndex("textIndex17185", indexKey, false, false);
        insertData();
        
        // 开启并发事务
        db1 = new Sequoiadb(SdbTestBase.coordUrl, "", "");
        db2 = new Sequoiadb(SdbTestBase.coordUrl, "", "");
        db3 = new Sequoiadb(SdbTestBase.coordUrl, "", "");
        db4 = new Sequoiadb(SdbTestBase.coordUrl, "", "");
        db5 = new Sequoiadb(SdbTestBase.coordUrl, "", "");
        cl1 = db1.getCollectionSpace(csName).getCollection(clName);
        cl2 = db2.getCollectionSpace(csName).getCollection(clName);
        cl3 = db3.getCollectionSpace(csName).getCollection(clName);
        cl4 = db4.getCollectionSpace(csName).getCollection(clName);
        cl5 = db5.getCollectionSpace(csName).getCollection(clName);
        db1.beginTransaction();
        db2.beginTransaction();
        db3.beginTransaction();
        db4.beginTransaction();
        db5.beginTransaction();

        // 事务1插入记录
        InsertThread insertThread = new InsertThread();
        insertThread.start();
        
        // 事务2更新记录
        UpdateThread updateThread = new UpdateThread();
        updateThread.start();

        // 事务3删除记录
        DeleteThread deleteThread = new DeleteThread();
        deleteThread.start();

        // 事务4读记录走索引扫描
        QueryThread queryThread = new QueryThread();
        queryThread.start();

        // 事务5读记录走表扫描
        QueryThread2 queryThread2 = new QueryThread2();
        queryThread2.start();
        
        Assert.assertTrue(insertThread.isSuccess());
        Assert.assertTrue(updateThread.isSuccess());
        Assert.assertTrue(deleteThread.isSuccess());
        Assert.assertTrue(queryThread.isSuccess());
        
        // 提交事务
        db1.commit();
        db2.commit();
        db3.commit();
        db4.commit();
        
        Assert.assertTrue(queryThread2.isSuccess());
        db5.commit();
        
        try {
            latch.await();
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
        
        CollectionSpace cs = sdb.getCollectionSpace(csName);
        cs.dropCollection(clName);
    }

    private void insertData() {
        List<BSONObject> records = new ArrayList<>();
        for (int i = 1; i <= 4000; i++) {
            BSONObject record = (BSONObject) JSON.parse("{_id:" + i + ",a:" + i + ", b:" + i + "}");
            records.add(record);
        }
        Collections.shuffle(records);
        cl.insert(records);
    }
    
    class InsertThread extends SdbThreadBase{
        @Override
        public void exec() throws Exception {
            try {
                List<BSONObject> records = new ArrayList<>();
                for (int i = 4001; i <= 5000; i++) {
                    BSONObject record = (BSONObject) JSON.parse("{_id:" + i + ",a:" + i + ", b:" + i + "}");
                    records.add(record);
                }
                Collections.shuffle(records);
                cl1.insert(records);
            } catch (Exception e) {
                e.printStackTrace();
                throw e;
            }
            finally {
                latch.countDown();
            }
        }
    }
    
    class UpdateThread extends SdbThreadBase{

        @Override
        public void exec() throws Exception { 
            cl2.update("{a:{$lt:'1001'}}", "{$set:{a:1}}", "{'':'textIndex17185'}");
            latch.countDown();
        }
    }
    
    class DeleteThread extends SdbThreadBase{

        @Override
        public void exec() throws Exception {
            cl3.delete("{$and:[{a:{$gt:1000}},{a:{$lt:2001}}]}", "{'':'textIndex17185'}");
            latch.countDown();
        }
    }
    
    class QueryThread extends SdbThreadBase{

        @Override
        public void exec() throws Exception {
            DBCursor cursor = cl4.query("{$and:[{a:{$gt:2000}},{a:{$lt:3001}}]}", null, "{_id:1}", "{'':'textIndex17185'}");
            List<BSONObject> records = TransUtils.getReadActList(cursor);
            List<BSONObject> expList = cl4Query();
            Assert.assertEquals(records, expList);
            latch.countDown();
        }
    }
    
    class QueryThread2 extends SdbThreadBase{

        @Override
        public void exec() throws Exception {
            DBCursor cursor = cl5.query("{$and:[{a:{$gt:3000}},{a:{$lt:4001}}]}", null, "{_id:-1}", "{'':null}");
            List<BSONObject> records = TransUtils.getReadActList(cursor);
            List<BSONObject> expList = cl5Query();
            Assert.assertEquals(records, expList);
            latch.countDown();
        }
    }

    private List<BSONObject> cl4Query() {
        List<BSONObject> records = new ArrayList<>();
        for (int i = 2001; i <= 3000; i++) {
            BSONObject record = (BSONObject) JSON.parse("{_id:" + i + ",a:" + i + ", b:" + i + "}");
            records.add(record);
        }
        return records;
    }

    private List<BSONObject> cl5Query() {
        List<BSONObject> records = new ArrayList<>();
        for (int i = 3001; i <= 4000; i++) {
            BSONObject record = (BSONObject) JSON.parse("{_id:" + i + ",a:" + i + ", b:" + i + "}");
            records.add(record);
        }
        Collections.reverse(records);
        return records;
    }
}
