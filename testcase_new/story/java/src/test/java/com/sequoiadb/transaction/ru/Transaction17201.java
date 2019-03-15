package com.sequoiadb.transaction.ru;

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
/**
 * @Description seqDB-17201:   对大记录进行操作与读并发，事务回滚  
 * @author Zhao Xiaoni
 * @date 2019-1-16
 */
import com.sequoiadb.testcommon.SdbTestBase;
import com.sequoiadb.transaction.TransUtils;

@Test(groups = "ru")
public class Transaction17201 extends SdbTestBase {
    private String clName = "cl_17201";
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
        String b = "b";
        for (int i = 0; i < 60 * 1024; i++) {
            b = "aaaaaaaaaaaaaaaaaaaa" + b;
        }

        // 集合中插入带索引的记录
        List<BSONObject> insertR1s = new ArrayList<BSONObject>();
        BSONObject insertR1 = null;
        for(int i=0; i<10; i++)
        {
            insertR1 = (BSONObject) JSON.parse("{_id:"+ i +", a:'aaaaaa', b:'" + b + "'}");
            insertR1s.add(insertR1);
        }
        cl.insert(insertR1s);

        // 开启两个并发事务
        db1.beginTransaction();
        db2.beginTransaction();

        // 事务1执行多个操作
        for(int i=0; i<10; i++){
            BSONObject insertR2 = (BSONObject) JSON.parse("{_id:"+ (10+i) +", a:'aaaaaaaa', b:'" + b + "'}");
            // 事务1对同一条记录执行多个操作
            cl1.insert(insertR2);
            cl1.update("{_id:"+ (10+i) +"}", "{$set:{a:'a'}}", null);
            cl1.delete("{_id:"+ (10+i) +"}");
            // 事务1对不同记录执行多个操作
            cl1.delete("{_id:"+ i +"}");
            cl1.insert(insertR2);
            cl1.update("{_id:"+ (10+i) +"}", "{$set:{a:'a'}}", null);
            cl1.update("{_id:"+ (10+i) +"}", "{$set:{a:'aaaaaaaa'}}", null);
            expList.add(insertR2);
        }

        // 事务2表扫描记录
        cursor = cl2.query(null, null, "{_id:1}", "{'':null}");
        actList = TransUtils.getReadActList(cursor);
        Assert.assertEquals(actList, expList);
        actList.clear();

        // 事务2索引扫描记录
        cursor = cl2.query(null, null, "{_id:1}", "{'':'a'}");
        actList = TransUtils.getReadActList(cursor);
        Assert.assertEquals(actList, expList);
        actList.clear();

        // 非事务表扫描记录
        cursor = cl.query(null, null, "{_id:1}", "{'':null}");
        actList = TransUtils.getReadActList(cursor);
        Assert.assertEquals(actList, expList);
        actList.clear();

        // 非事务索引扫描记录
        cursor = cl.query(null, null, "{_id:1}", "{'':'a'}");
        actList = TransUtils.getReadActList(cursor);
        Assert.assertEquals(actList, expList);
        actList.clear();

        db1.rollback();

        // 事务2表扫描记录
        cursor = cl2.query(null, null, "{_id:1}", "{'':null}");
        actList = TransUtils.getReadActList(cursor);
        Assert.assertEquals(actList, insertR1s);
        actList.clear();

        // 事务2索引扫描记录
        cursor = cl2.query(null, null, "{_id:1}", "{'':'a'}");
        actList = TransUtils.getReadActList(cursor);
        Assert.assertEquals(actList, insertR1s);
        actList.clear();

        // 非事务表扫描记录
        cursor = cl.query(null, null, "{_id:1}", "{'':null}");
        actList = TransUtils.getReadActList(cursor);
        Assert.assertEquals(actList, insertR1s);
        actList.clear();

        // 非事务索引扫描记录
        cursor = cl.query(null, null, "{_id:1}", "{'':'a'}");
        actList = TransUtils.getReadActList(cursor);
        Assert.assertEquals(actList, insertR1s);

        db2.commit();
        cursor.close();
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
