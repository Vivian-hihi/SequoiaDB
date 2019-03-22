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
 * @Description seqDB-17269 : 插入记录与其他事务中更新后的记录重复
 * @author luweikang
 * @date 2019年1月15日
 */
@Test(groups = "ru")
public class Transaction17269 extends SdbTestBase {

    private String clName = "transCL_17269";
    private Sequoiadb sdb = null;
    private Sequoiadb sdb2 = null;
    private DBCollection cl = null;
    private DBCollection cl2 = null;
    private BSONObject data = null;
    private BSONObject data2 = null;
    private BSONObject data3 = null;
    private BSONObject matcher = null;
    private BSONObject modifier = null;
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
        data.put("_id", "insertId17269_1");
        data.put("a", 1);
        data.put("b", "testTrans_17269");
        data.put("c", 13700000000L);
        data.put("d", "customer transaction type data application.");
        cl.insert(data);
        expDataList.add(data);

        modifier = new BasicBSONObject();
        data2 = new BasicBSONObject();
        data2.put("_id", "insertId17269_1");
        data2.put("a", 17269);
        data2.put("b", "testTrans_17269Update");
        data2.put("c", 13700017269L);
        data2.put("d", "customer transaction type data application.");
        modifier.put("$set", data2);
        
        data3 = new BasicBSONObject();
        data3.put("_id", "insertId17269_2");
        data3.put("a", 17269);
        data3.put("b", 1);
        data3.put("c", 13700000000L);
        data3.put("d", "customer transaction type data application.");
        
        matcher = new BasicBSONObject("a", new BasicBSONObject("$isnull", 0));
        
        sdb2 = new Sequoiadb(SdbTestBase.coordUrl, "", "");
        cl2 = sdb2.getCollectionSpace(csName).getCollection(clName);
    }

    @Test
    public void test1() {
        sdb.beginTransaction();
        sdb2.beginTransaction();
        try {
            // 1 trans1 update R1 to R2
            cl.update(matcher, modifier, null);
            
            // 2 trans2 insert record R3 same as the R2
            cl2.insert(data3);
            Assert.fail("insert an existing record with an index,should be failed");
        } catch (BaseException e) {
            Assert.assertEquals(e.getErrorCode(), -38, e.getMessage());
        }
        sdb.rollback();

        recordCur = cl.query(null, null, null, "{'': null}");
        actDataList = TransUtils.getReadActList(recordCur);
        Assert.assertEquals(actDataList, expDataList);
        actDataList.clear();

        recordCur = cl.query(null, null, null, "{'': 'a'}");
        actDataList = TransUtils.getReadActList(recordCur);
        Assert.assertEquals(actDataList, expDataList);
        actDataList.clear();

    }

    @Test
    public void test2() {
        sdb.beginTransaction();
        sdb2.beginTransaction();
        try {
            // 1 trans1 update R1 to R2
            cl.update(matcher, modifier, null);
            
            // 2 trans2 insert record R3 same as the R2
            cl2.insert(data3);
            Assert.fail("insert an existing record with an index,should be failed");
        } catch (BaseException e) {
            Assert.assertEquals(e.getErrorCode(), -38, e.getMessage());
        }
        sdb.commit();
        expDataList.clear();
        expDataList.add(data2);

        recordCur = cl.query(null, null, null, "{'': null}");
        actDataList = TransUtils.getReadActList(recordCur);
        Assert.assertEquals(actDataList, expDataList);
        actDataList.clear();

        recordCur = cl.query(null, null, null, "{'': 'a'}");
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
