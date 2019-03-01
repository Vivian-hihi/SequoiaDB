package com.sequoiadb.transaction.ru;

import java.util.ArrayList;
import java.util.List;

import org.bson.BSONObject;
import org.bson.BasicBSONObject;
import org.testng.Assert;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.DBCursor;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.testcommon.SdbTestBase;
import com.sequoiadb.transaction.TransUtils;

/**
 * @Description Transaction17246.java 插入记录与其他事务中插入的记录重复
 * @author luweikang
 * @date 2019年1月15日
 */
@Test(groups = "ru")
public class Transaction17246 extends SdbTestBase {

    private String clName = "transCL_17246";
    private Sequoiadb sdb = null;
    private Sequoiadb sdb2 = null;
    private DBCollection cl = null;
    private DBCollection cl2 = null;
    private BSONObject data = null;
    private DBCursor recordCur = null;
    private List<BSONObject> expDataList = null;
    private List<BSONObject> actDataList = null;

    @BeforeClass
    public void setUp() {
        sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "");
        cl = sdb.getCollectionSpace(csName).createCollection(clName);
        cl.createIndex("a", "{a:1}", true, false);
        expDataList = new ArrayList<BSONObject>();
        
        data = new BasicBSONObject();
        data.put("a", 1);
        data.put("b", "testTrans_17243");
        data.put("c", 13700000000L);
        data.put("d", "customer transaction type data application.");
        expDataList.add(data);

        sdb2 = new Sequoiadb(SdbTestBase.coordUrl, "", "");
        cl2 = sdb2.getCollectionSpace(csName).createCollection(clName);
    }

    @Test
    public void test1() {
        sdb.beginTransaction();
        sdb2.beginTransaction();
        cl.insert(data);
        try {
            // insert the same record repeatedly
            cl2.insert(data);
            Assert.fail("insert an existing record with an index,should be failed");
        } catch (BaseException e) {
            Assert.assertEquals(e.getErrorCode(), -38, e.getMessage());
        }
        sdb.rollback();
        
        Assert.assertEquals(cl.getCount(), 0);

    }

    @Test
    public void test2() {
        sdb.beginTransaction();
        sdb2.beginTransaction();
        cl.insert(data);
        try {
            // insert the same record repeatedly
            cl2.insert(data);
            Assert.fail("insert an existing record with an index,should be failed");
        } catch (BaseException e) {
            Assert.assertEquals(e.getErrorCode(), -38, e.getMessage());
        }
        sdb.commit();

        recordCur = cl.query("{'a': {'$isnull': 0}}", null, null, "{'': null}");
        actDataList = TransUtils.getReadActList(recordCur);
        Assert.assertEquals(actDataList, expDataList);
        actDataList.clear();

        recordCur = cl.query("{'a': {'$isnull': 0}}", null, null, "{'': '_id'}");
        actDataList = TransUtils.getReadActList(recordCur);
        Assert.assertEquals(actDataList, expDataList);
        actDataList.clear();

        cl.delete("{'a': {'$isnull' :0}}");
        Assert.assertEquals(cl.getCount(), 0);

    }

    @AfterClass
    public void tearDown() {
        sdb.getCollectionSpace(csName).dropCollection(clName);
        if(recordCur != null){
            recordCur.close();
        }
        if( sdb != null ){
            sdb.close();
        }
        if( sdb2 != null ){
            sdb2.close();
        }
    }

}
