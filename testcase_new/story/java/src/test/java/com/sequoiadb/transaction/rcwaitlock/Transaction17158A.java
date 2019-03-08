package com.sequoiadb.transaction.rcwaitlock;

/**
 * @Description seqDB-17158:  多个原子操作与读并发，事务提交 
 * @author xiaoni Zhao
 * @date 2019-1-23
 */
import java.util.ArrayList;
import java.util.List;

import org.bson.BSONObject;
import org.bson.util.JSON;
import org.testng.Assert;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;
import com.sequoiadb.base.CollectionSpace;
import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.DBCursor;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.testcommon.SdbTestBase;
import com.sequoiadb.testcommon.SdbThreadBase;
import com.sequoiadb.transaction.TransUtils;

@Test(groups = "rcwaitlock")
public class Transaction17158A extends SdbTestBase {
    private String clName = "cl_17158A";
    private Sequoiadb sdb = null;
    private Sequoiadb db1 = null;
    private Sequoiadb db2 = null;
    private DBCollection cl = null;
    private DBCollection cl1 = null;
    private DBCollection cl2 = null;
    private DBCursor cursor = null;
    private List<BSONObject> expList = new ArrayList<BSONObject>();

    @BeforeClass
    public void setUp() {
        sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "");
        db1 = new Sequoiadb(SdbTestBase.coordUrl, "", "");
        db2 = new Sequoiadb(SdbTestBase.coordUrl, "", "");
        cl = sdb.getCollectionSpace(csName).createCollection(clName);
        cl1 = db1.getCollectionSpace(csName).getCollection(clName);
        cl2 = db2.getCollectionSpace(csName).getCollection(clName);
        cl.createIndex("a", "{a:1}", false, false);
        expList = TransUtils.insertDatas(cl, 0, 10000, 0);
    }

    @Test
    public void test() {
        db1.beginTransaction();

        // 事务1对同一条记录执行多个原子操作
        BSONObject insertR3 = (BSONObject) JSON.parse("{a:3,b:3}");
        for(int i=0; i<10000; i++){
            cl1.insert(insertR3);
            cl1.update("{a:3}", "{$set:{a:33}}", "{'':'a'}");
            cl1.delete("{a:33}", "{'':'a'}");
        }

        // 事务2表扫描记录
        Read read1 = new Read("{'':null}");
        read1.start();
        Assert.assertTrue(read1.matchBlockingMethod(DBCursor.class.getName(), "hasNext"));

        // 事务2索引扫描记录
        cursor = cl2.query(null, null, "{_id:1}", "{'':null}");
        Assert.assertEquals(TransUtils.getReadActList(cursor), expList);

        // 非事务表扫描记录
        cursor = cl.query(null, null, "{_id:1}", "{'':null}");
        Assert.assertEquals(TransUtils.getReadActList(cursor), expList);

        // 非事务索引扫描记录
        cursor = cl.query(null, null, "{_id:1}", "{'':'a'}");
        Assert.assertEquals(TransUtils.getReadActList(cursor), expList);

        db1.commit();

        // 校验阻塞线程返回的记录
        if (!read1.isSuccess()) {
            Assert.fail(read1.getErrorMsg());
        }
        try {
            Assert.assertEquals(read1.getExecResult(), expList);
        } catch (InterruptedException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        }

        db2.commit();
        cursor.close();
    }

    private class Read extends SdbThreadBase {
        private Sequoiadb db = null;
        private Sequoiadb db2 = null;
        private DBCollection cl = null;
        private DBCollection cl2 = null;
        private String hint = null;

        public Read(String hint) {
            // TODO Auto-generated constructor stub
            this.hint = hint;
        }

        @Override
        public void exec() throws Exception {
            // TODO Auto-generated method stub
            db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
            db2 = new Sequoiadb(SdbTestBase.coordUrl, "", "");
            cl = db.getCollectionSpace(csName).getCollection(clName);
            cl2 = db2.getCollectionSpace(csName).getCollection(clName);

            // 开启并发事务2
            db2.beginTransaction();

            try {
                DBCursor cursor = cl2.query(null, null, "{_id:1}", hint);
                List<BSONObject> records = TransUtils.getReadActList(cursor);
                setExecResult(records);

                // 事务2扫描记录
                cursor = cl2.query(null, null, "{_id:1}", hint);
                Assert.assertEquals(TransUtils.getReadActList(cursor), expList);

                // 非事务扫描记录
                cursor = cl.query(null, null, "{_id:1}", hint);
                Assert.assertEquals(TransUtils.getReadActList(cursor), expList);

                db2.commit();
            } catch (BaseException e) {
                e.printStackTrace();
                throw e;
            } finally {
                db2.rollback();
                db2.close();
            }
        }
    }

    @AfterClass
    public void tearDown() {
        if (!db1.isClosed()) {
            db1.close();
        }
        if (!db2.isClosed()) {
            db2.close();
        }
        CollectionSpace cs = sdb.getCollectionSpace(csName);
        if (cs.isCollectionExist(clName)) {
            cs.dropCollection(clName);
        }
        if (!sdb.isClosed()) {
            sdb.close();
        }
    }
}
