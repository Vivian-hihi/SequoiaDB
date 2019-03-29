package com.sequoiadb.transaction.rc;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.List;
import java.util.concurrent.CountDownLatch;

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
import com.sequoiadb.testcommon.SdbTestBase;
import com.sequoiadb.testcommon.SdbThreadBase;
import com.sequoiadb.transaction.TransUtils;

/**
 * @testcase seqDB-18046:并发事务中读写记录存在交集，使用复合索引查询
 * @date 2019-3-26
 * @author yinzhen
 *
 */
@Test(groups = "rc")
public class Transaction18046 extends SdbTestBase {
    private Sequoiadb sdb = null;
    private String clName = "cl18046";
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
    private List<BSONObject> expList = new ArrayList<>();

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
        return new Object[][] { { "{'a':1, 'b':1}" }, { "{'a':1, 'b':-1}" }, { "{'a':-1, 'b':1}" },
                { "{'a':-1, 'b':-1}" } };
    }

    @Test(dataProvider = "index")
    public void test(String indexKey) {
        try {
            System.out.println("--复合索引 " + indexKey + " 开始事务--");
            latch = new CountDownLatch(5);
            cl = sdb.getCollectionSpace(csName).createCollection(clName);
            cl.createIndex("textIndex18046", indexKey, false, false);
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

            // 先判断表扫描和索引扫描记录
            Assert.assertTrue(queryThread.isSuccess());
            Assert.assertTrue(queryThread2.isSuccess());
            db4.commit();
            db5.commit();

            // 提交插入事务
            Assert.assertTrue(insertThread.isSuccess(), insertThread.getErrorMsg());
            db1.commit();

            int doTimes = 0;
            while (true) {
                doTimes ++;
                boolean update = updateThread.matchBlockingMethod(DBCollection.class.getName(), "update");
                boolean delete = deleteThread.matchBlockingMethod(DBCollection.class.getName(), "delete");
                boolean updateFlag = false;
                boolean deleteFlag = false;
                if (!update) {
                    if (updateThread.isSuccess()) {
                        db2.commit();
                        updateFlag = true;
                    }
                }
                if (!delete) {
                    if (deleteThread.isSuccess()) {
                        db3.commit();
                        deleteFlag = true;
                    }
                }
                if (updateFlag && deleteFlag) {
                    break;
                }
                if (doTimes == 120) {
                    throw new SkipException("Transactions deadlocks skip this testcase");
                }
                try {
                    Thread.sleep(1000);
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
            }

            // 提交事务
            Assert.assertTrue(updateThread.isSuccess(), updateThread.getErrorMsg());
            Assert.assertTrue(deleteThread.isSuccess(), deleteThread.getErrorMsg());

            // 非事务表扫描
            DBCursor cursor = cl.query("{$and:[{a:{$in:" + Arrays.toString(getAllRandArray()) + "}},{b:{$in:"
                    + Arrays.toString(getAllRandArray()) + "}}]}", "", "{a:1, b:-1, _id:1}", "{'':null}");
            List<BSONObject> tbScanActList = TransUtils.getReadActList(cursor);

            // 非事务索引扫描
            cursor = cl.query(
                    "{$and:[{a:{$in:" + Arrays.toString(getAllRandArray()) + "}},{b:{$in:"
                            + Arrays.toString(getAllRandArray()) + "}}]}",
                    "", "{a:1, b:-1, _id:1}", "{'':'textIndex18046'}");
            List<BSONObject> ixScanActList = TransUtils.getReadActList(cursor);
            Assert.assertEquals(tbScanActList, ixScanActList);

            latch.await();
        } catch (InterruptedException e) {
            e.printStackTrace();
        } finally {
            tearDownCommit();
            CollectionSpace cs = sdb.getCollectionSpace(csName);
            cs.dropCollection(clName);
            System.out.println("--复合索引 " + indexKey + " 结束事务--");
        }
    }

    public void tearDownCommit() {
        db1.commit();
        db2.commit();
        db3.commit();
        db4.commit();
        db5.commit();
    }

    // 构造记录 a 字段相等 b 字段不相等，a b 字段都不相等的记录
    public void insertData() {
        int a = 0;
        List<BSONObject> records = new ArrayList<BSONObject>();
        for (int i = 0; i <= 40000; i++) {
            if (i < 20000) {
                a = 100;
            } else {
                a = i;
            }
            BSONObject object = (BSONObject) JSON.parse("{_id:" + i + ", a:" + a + ", b:" + (40000 - i) + "}");
            records.add(object);
        }
        expList.clear();
        expList.addAll(records);
        Collections.shuffle(records);
        cl.insert(records);
    }

    public Integer[] getAllRandArray() {
        List<Integer> randList = new ArrayList<>();
        for (int i = 0; i <= 50000; i++) {
            randList.add(i);
        }
        Collections.shuffle(randList);
        Integer[] rangArray = randList.toArray(new Integer[randList.size()]);
        return rangArray;
    }

    // 事务1插入记录,插入记录的索引值与事务2中更新后的值存在重复
    class InsertThread extends SdbThreadBase {
        @Override
        public void exec() throws Exception {
            try {
                List<BSONObject> records = new ArrayList<>();
                for (int i = 40001; i <= 50000; i++) {
                    BSONObject record = (BSONObject) JSON.parse("{_id:" + i + ", a:" + i + ", b:" + i + "}");
                    records.add(record);
                }
                Collections.shuffle(records);
                cl1.insert(records);
            } catch (Exception e) {
                e.printStackTrace();
                throw e;
            } finally {
                latch.countDown();
            }
        }
    }

    // 事务2更新记录，更新匹配到集合中原有的记录及事务1中插入的记录
    class UpdateThread extends SdbThreadBase {

        @Override
        public void exec() throws Exception {
            cl2.update("{$and:[{b:{$gt:35000}},{b:{$lt:45001}}]}", "{$inc:{a:10, b:10}}", "{'':'textIndex18046'}");
            latch.countDown();
        }
    }

    // 事务3删除记录，删除的记录匹配集合中原有记录，事务1中插入的记录，事务2中更新的记录
    class DeleteThread extends SdbThreadBase {

        @Override
        public void exec() throws Exception {
            cl3.delete("{$and:[{b:{$gt:32000}},{b:{$lt:42001}}]}", "{'':'textIndex18046'}");
            latch.countDown();
        }
    }

    public Integer[] getRandomArray() {
        List<Integer> randList = new ArrayList<>();
        for (int i = 0; i <= 40000; i++) {
            randList.add(i);
        }
        Collections.shuffle(randList);
        Integer[] randArray = randList.toArray(new Integer[randList.size()]);
        return randArray;
    }

    class QueryThread extends SdbThreadBase {

        @Override
        public void exec() throws Exception {
            Integer[] randArray = getRandomArray();
            DBCursor cursor = cl4.query("{$and:[{a:{$in:" + Arrays.toString(randArray) + "}},{b:{$in:"
                    + Arrays.toString(randArray) + "}}]}", null, "{a:1, b:-1}", "{'':'textIndex18046'}");
            List<BSONObject> records = TransUtils.getReadActList(cursor);
            System.out.println("索引扫描  Records : " + records.size() + ", ExpRecords : " + expList.size());
            Assert.assertEquals(records, expList);
            latch.countDown();
        }
    }

    class QueryThread2 extends SdbThreadBase {

        @Override
        public void exec() throws Exception {
            Integer[] randArray = getRandomArray();
            DBCursor cursor = cl5.query("{$and:[{a:{$in:" + Arrays.toString(randArray) + "}},{b:{$in:"
                    + Arrays.toString(randArray) + "}}]}", null, "{a:1, b:-1}", "{'':null}");
            List<BSONObject> records = TransUtils.getReadActList(cursor);
            System.out.println("表扫描  Records : " + records.size() + ", ExpRecords : " + expList.size());
            Assert.assertEquals(records, expList);
            latch.countDown();
        }
    }

    public List<BSONObject> getExpRecords() {
        List<BSONObject> expRecords = new ArrayList<>();
        int a = 500;
        for (int i = 1500; i < 2000; i++) {
            BSONObject object = (BSONObject) JSON.parse("{_id:" + i + ", a:500, b:400}");
            expRecords.add(object);
        }
        for (int i = 27000; i < 37000; i++) {
            String str = "{_id:" + i + ", a:" + a + ", b:" + i + "}";
            BSONObject object = (BSONObject) JSON.parse(str);
            expRecords.add(object);
        }
        return expRecords;
    }
}
