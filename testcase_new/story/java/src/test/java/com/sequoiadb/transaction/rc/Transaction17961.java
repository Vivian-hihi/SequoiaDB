package com.sequoiadb.transaction.rc;

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

/**
 * @testcase seqDB-17961:删除记录与$id索引扫描并发
 * @date 2019-3-1
 * @author yinzhen
 *
 */
@Test(groups = "rc")
public class Transaction17961 extends SdbTestBase {
    private Sequoiadb sdb = null;
    private String clName = "cl17961";
    private DBCollection cl = null;
    private List<BSONObject> expList = new ArrayList<BSONObject>();
    private List<BSONObject> actList = new ArrayList<BSONObject>();
    private Sequoiadb db1 = null;
    private Sequoiadb db2 = null;
    private DBCollection cl1 = null;
    private DBCollection cl2 = null;

    @BeforeClass
    public void setUp() {
        sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "");
        cl = sdb.getCollectionSpace(csName).createCollection(clName);
        BSONObject record = (BSONObject) JSON.parse("{_id:1, a:1, b:1}");
        cl.insert(record);
        expList.add(record);
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

    @Test
    public void test() {
        // 开启2个并发事务
        db1 = new Sequoiadb(SdbTestBase.coordUrl, "", "");
        db2 = new Sequoiadb(SdbTestBase.coordUrl, "", "");
        cl1 = db1.getCollectionSpace(csName).getCollection(clName);
        cl2 = db2.getCollectionSpace(csName).getCollection(clName);
        db1.beginTransaction();
        db2.beginTransaction();

        // 事务1删除记录
        cl1.delete("{_id:1}", "{'':'$id'}");

        // 事务2读记录走表扫描
        DBCursor recordsCursor = cl2.query(null, null, null, "{'':null}");
        actList = TransUtils.getReadActList(recordsCursor);
        Assert.assertEquals(actList, expList);

        // 事务2读记录走$id索引扫描
        recordsCursor = cl2.query("{a:{$exists:1}}", null, null, "{'':'$id'}");
        actList = TransUtils.getReadActList(recordsCursor);
        Assert.assertEquals(actList, expList);

        // 事务1、2提交
        db1.commit();
        db2.commit();

        // 非事务表扫描
        recordsCursor = cl.query(null, null, null, "{'':null}");
        actList = TransUtils.getReadActList(recordsCursor);
        expList.clear();
        Assert.assertEquals(actList, expList);

        // 非事务索引扫描
        recordsCursor = cl.query("{a:{$exists:1}}", null, null, "{'':'$id'}");
        actList = TransUtils.getReadActList(recordsCursor);
        Assert.assertEquals(actList, expList);
        recordsCursor.close();
    }
}
