package com.sequoiadb.transaction.rc;

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
 * @Description Transaction17133.java   更新已提交记录与其他事务删除的记录重复 
 * @author luweikang
 * @date 2019年1月15日
 */
@Test(groups = "rc")
public class Transaction17133 extends SdbTestBase {
    
    private String clName = "transCL_17133";
    private Sequoiadb sdb = null;
    private DBCollection cl = null;
    private BSONObject data = null;
    private BSONObject data2 = null;
    private BSONObject updateData = null;
    private BSONObject modifier = null;
    private DBCursor recordCur = null;
    private List<BSONObject> expDataList = null;
    private List<BSONObject> actDataList = null;
    
    @BeforeClass
    public void setUp(){
        sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "");
        cl = sdb.getCollectionSpace(csName).createCollection(clName);
        cl.createIndex("a", "{a:1}", true, false);
        expDataList = new ArrayList<BSONObject>();
        data = new BasicBSONObject();
        data.put("_id", "insert1713301");
        data.put("a", 1);
        data.put("b", "testTrans_17133");
        data.put("c", 13700000000L);
        data.put("d", "customer transaction type data application.");
        expDataList.add(data);
        
        data2 = new BasicBSONObject();
        data2.put("_id", "insert1713302");
        data2.put("a", 2);
        data2.put("b", 1024);
        data2.put("c", 13700000000L);
        data2.put("d", "customer transaction type data application.");
        expDataList.add(data2);
        cl.insert(expDataList);
        
        updateData = new BasicBSONObject();
        updateData.put("_id", "insert1713301");
        updateData.put("a", 3);
        updateData.put("b", 2048);
        updateData.put("c", 13700000000L);
        updateData.put("d", "customer transaction type data application.");
        
        modifier = new BasicBSONObject();
        modifier.put("$set", updateData);
        
    }
    
    //TODO:SEQUOIADBMAINSTREAM-4113
    @Test(enabled=false)
    public void test1(){
        Sequoiadb transDB1 = null;
        Sequoiadb transDB2 = null;
        try{
            transDB1 = new Sequoiadb(SdbTestBase.coordUrl, "", "");
            transDB2 = new Sequoiadb(SdbTestBase.coordUrl, "", "");
            transDB1.beginTransaction();
            DBCollection transCL1 = transDB1.getCollectionSpace(csName).getCollection(clName);
            transCL1.delete("{'a':1}");
            try {
                transDB2.beginTransaction();
                DBCollection transCL2 = transDB2.getCollectionSpace(csName).getCollection(clName);
                transCL2.update(new BasicBSONObject("a", 2), modifier, null);
                transDB2.commit();
                //Assert.fail("insert an existing record with an index,should be failed");
            } catch (BaseException e) {
                Assert.assertEquals(e.getErrorCode(), -38, e.getMessage());
            }
            transDB1.rollback();
        }finally {
            if(transDB1 != null){
                transDB1.close();
            }
            if(transDB2 != null){
                transDB2.close();
            }
        }
        
        recordCur = cl.query("{'a': {'$isnull': 0}}", null, null, "{'': null}");
        actDataList = TransUtils.getReadActList(recordCur);
        Assert.assertEquals(actDataList, expDataList);
        actDataList.clear();
        
        recordCur = cl.query("{'a': {'$isnull': 0}}", null, null, "{'': 'a'}");
        actDataList = TransUtils.getReadActList(recordCur);
        Assert.assertEquals(actDataList, expDataList);
        actDataList.clear();
        
    }
    
    //TODO:SEQUOIADBMAINSTREAM-4113
    @Test(enabled =false)
    public void test2(){
        Sequoiadb transDB1 = null;
        Sequoiadb transDB2 = null;
        try{
            transDB1 = new Sequoiadb(SdbTestBase.coordUrl, "", "");
            transDB2 = new Sequoiadb(SdbTestBase.coordUrl, "", "");
            transDB1.beginTransaction();
            DBCollection transCL1 = transDB1.getCollectionSpace(csName).getCollection(clName);
            transCL1.delete("{'a':1}");
            try {
                transDB2.beginTransaction();
                DBCollection transCL2 = transDB2.getCollectionSpace(csName).getCollection(clName);
                transCL2.update(new BasicBSONObject("a", 2), modifier, null);
                Assert.fail("insert an existing record with an index,should be failed");
            } catch (BaseException e) {
                Assert.assertEquals(e.getErrorCode(), -38, e.getMessage());
            }
            transDB1.commit();
            expDataList.clear();
            expDataList.add(data2);
        }finally {
            if(transDB1 != null){
                transDB1.close();
            }
            if(transDB2 != null){
                transDB2.close();
            }
        }
        
        recordCur = cl.query("{'a': {'$isnull': 0}}", null, "{'a':1}", "{'': null}");
        actDataList = TransUtils.getReadActList(recordCur);
        Assert.assertEquals(actDataList, expDataList);
        actDataList.clear();
        
        
        recordCur = cl.query("{'a': {'$isnull': 0}}", null, "{'a':1}", "{'': 'a'}");
        actDataList = TransUtils.getReadActList(recordCur);
        Assert.assertEquals(actDataList, expDataList);
        actDataList.clear();
        
        cl.delete("{'a': {'$isnull' :0}}");
        Assert.assertEquals(cl.getCount(),0 );
        
    }
    
    @AfterClass
    public void tearDown(){
        try {
            sdb.getCollectionSpace(csName).dropCollection(clName);
        } finally {
            if(recordCur != null){
                recordCur.close();
            }
            if( sdb != null ){
                sdb.close();
            }
        }
    }
    
}
