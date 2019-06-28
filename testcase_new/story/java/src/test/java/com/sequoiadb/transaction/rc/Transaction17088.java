package com.sequoiadb.transaction.rc;

/**
 * @Description seqDB-17088:事务中批量插入与读并发 
 * @author xiaoni Zhao
 * @date 2019-1-21
 */
import java.util.ArrayList;
import java.util.List;

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

@Test(groups = "rc")
public class Transaction17088 extends SdbTestBase {
    private String clName = "cl_17088";
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
        if (CommLib.isStandAlone(sdb)) {
            throw new SkipException("STANDALONE MODE");
        }
        if (CommLib.OneGroupMode(sdb)) {
            throw new SkipException("ONE GROUP MODE");
        }

        db1 = new Sequoiadb(SdbTestBase.coordUrl, "", "");
        db2 = new Sequoiadb(SdbTestBase.coordUrl, "", "");

        sdb.getCollectionSpace(csName).createCollection(clName + "hash",
                (BSONObject) JSON.parse("{ShardingKey:{b:1}, ShardingType:'hash', AutoSplit:true}"));

        DBCollection mainCL = sdb.getCollectionSpace(csName).createCollection(clName + "mainCL",
                (BSONObject) JSON.parse("{ShardingKey:{b:1}, ShardingType:'range', IsMainCL:true}"));
        sdb.getCollectionSpace(csName).createCollection("sub117088");
        sdb.getCollectionSpace(csName).createCollection("sub217088",
                (BSONObject) JSON.parse("{ShardingKey:{b:1}, ShardingType:'hash', AutoSplit:true}"));
        mainCL.attachCollection(csName + ".sub117088",
                (BSONObject) JSON.parse("{LowBound:{b:{'$minKey':1}}, UpBound:{b:25000}}"));
        mainCL.attachCollection(csName + ".sub217088",
                (BSONObject) JSON.parse("{LowBound:{b:25000}, UpBound:{b:{'$maxKey':1}}}"));
    }

    @DataProvider(name = "getCL")
    private Object[][] getCLName() {
        return new Object[][] { { clName + "hash" }, { clName + "mainCL" } };
    }

    @Test(dataProvider = "getCL")
    public void test(String clName) {
        cl = sdb.getCollectionSpace(csName).getCollection(clName);
        cl1 = db1.getCollectionSpace(csName).getCollection(clName);
        cl2 = db2.getCollectionSpace(csName).getCollection(clName);
        cl.createIndex("a", "{a:1}", false, false);

        // 开启两个并发事务
        db1.beginTransaction();
        db2.beginTransaction();

        // 事务1执行批量插入记录
        expList = TransUtils.insertDatas(cl1, 0, 50000, 1);

        // 事务2表扫描记录
        cursor = cl2.query(null, null, null, "{'':null}");
        actList = TransUtils.getReadActList(cursor);
        Assert.assertTrue(actList.isEmpty());
        actList.clear();

        // 事务2索引扫描记录
        cursor = cl2.query(null, null, null, "{'':'a'}");
        actList = TransUtils.getReadActList(cursor);
        Assert.assertTrue(actList.isEmpty());
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

        db1.commit();

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

        db2.commit();
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
}
