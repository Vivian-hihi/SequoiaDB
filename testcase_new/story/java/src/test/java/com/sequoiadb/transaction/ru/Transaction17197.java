package com.sequoiadb.transaction.ru;

/**
 * @Description seqDB-17197: 删除记录与读记录并发，事务回滚 
 * @author Zhao Xiaoni
 * @date 2019-1-16
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
import com.sequoiadb.testcommon.SdbTestBase;
import com.sequoiadb.transaction.TransUtils;

@Test(groups = "ru")
public class Transaction17197 extends SdbTestBase {
    private String clName = "cl_17197";
    private Sequoiadb sdb = null;
    private Sequoiadb db1 = null;
    private Sequoiadb db2 = null;
    private DBCollection cl = null;
    private DBCollection cl1 = null;
    private DBCollection cl2 = null;
    private DBCursor cursor = null;
    private List<BSONObject> expList = new ArrayList<BSONObject>();
    private List<BSONObject> actList = new ArrayList<BSONObject>();

    @BeforeClass
    public void setUp() {
        sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "");
        db1 = new Sequoiadb(SdbTestBase.coordUrl, "", "");
        db2 = new Sequoiadb(SdbTestBase.coordUrl, "", "");
        cl = sdb.getCollectionSpace(csName).createCollection(clName);
        cl1 = db1.getCollectionSpace(csName).getCollection(clName);
        cl2 = db2.getCollectionSpace(csName).getCollection(clName);
        cl.createIndex("a", "{a:1}", false, false);
    }

    @Test
    public void test() {
        // 集合中插入带索引的记录
        BSONObject insertR1 = (BSONObject) JSON.parse("{_id:1,a:1,b:1}");
        cl.insert(insertR1);
        expList.add(insertR1);

        db1.beginTransaction();
        db2.beginTransaction();

        // 事务1删除记录R1
        cl1.delete("{a:1}", "{'':null}");

        // 事务2表扫描记录
        cursor = cl2.query(null, null, null, "{'':null}");
        actList = TransUtils.getReadActList(cursor);
        Assert.assertTrue(actList.isEmpty());

        // 事务2索引扫描记录
        cursor = cl2.query(null, null, null, "{'':'a'}");
        actList = TransUtils.getReadActList(cursor);
        Assert.assertTrue(actList.isEmpty());

        // 非事务表扫描记录
        cursor = cl.query(null, null, null, "{'':null}");
        actList = TransUtils.getReadActList(cursor);
        Assert.assertTrue(actList.isEmpty());

        // 非事务索引扫描记录
        cursor = cl.query(null, null, null, "{'':'a'}");
        actList = TransUtils.getReadActList(cursor);
        Assert.assertTrue(actList.isEmpty());

        db1.rollback();

        // 事务2表扫描记录
        cursor = cl2.query(null, null, null, "{'':null}");
        actList = TransUtils.getReadActList(cursor);
        Assert.assertEquals(actList, expList);

        // 事务2索引扫描记录
        cursor = cl2.query(null, null, null, "{'':'a'}");
        actList = TransUtils.getReadActList(cursor);
        Assert.assertEquals(actList, expList);

        // 非事务表扫描记录
        cursor = cl.query(null, null, null, "{'':null}");
        actList = TransUtils.getReadActList(cursor);
        Assert.assertEquals(actList, expList);

        // 非事务索引扫描记录
        cursor = cl.query(null, null, null, "{'':'a'}");
        actList = TransUtils.getReadActList(cursor);
        Assert.assertEquals(actList, expList);

        db2.rollback();
        cursor.close();
    }

    @AfterClass
    public void tearDown() {
        db1.commit();
        db2.commit();
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
