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
import com.sequoiadb.testcommon.SdbTestBase;
import com.sequoiadb.transaction.TransUtils;

/**
 * @Description Transaction17122.java 插入记录与本事务中删除的记录重复 
 * @author luweikang
 * @date 2019年1月15日
 */
@Test(groups = "rc")
public class Transaction17122 extends SdbTestBase {
    
    private String clName = "transCL_17122";
    private Sequoiadb sdb = null;
    private DBCollection cl = null;
    private BSONObject data = null;
    private DBCursor recordCur = null;
    private List<BSONObject> expDataList = null;
    private List<BSONObject> actDataList = null;
    
    @BeforeClass
    public void setUp(){
        sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "");
        cl = sdb.getCollectionSpace(csName).createCollection(clName);
        data = new BasicBSONObject();
        data.put("_id", "testId17122");
        data.put("a", 1);
        data.put("b", "testTrans_17122");
        data.put("c", 13700000000L);
        data.put("d", "customer transaction type data application.");
        cl.insert(data);
        cl.createIndex("a", "{a:1}", true, false);
        expDataList = new ArrayList<BSONObject>();
    }
    
    //TODO:count num error, return -1 SEQUOIADBMAINSTREAM-4182
    @Test(enabled=false)
    public void test1(){
        
        sdb.beginTransaction();
        DBCollection transCL = sdb.getCollectionSpace(csName).getCollection(clName);
        BSONObject matcher = new BasicBSONObject("_id", data.get("_id"));
        transCL.delete(matcher);
        
        BSONObject testData = new BasicBSONObject();
        testData.put("_id", data.get("_id"));
        testData.put("a", data.get("a"));
        testData.put("b", 17122);
        testData.put("flag", "flag17122");
        //insert 
        transCL.insert(testData);
        
        sdb.rollback();
        
        expDataList.add(data);
        recordCur = cl.query("{'a': {'$isnull': 0}}", null, null, "{'': null}");
        actDataList = TransUtils.getReadActList(recordCur);
        Assert.assertEquals(actDataList, expDataList);
        actDataList.clear();
        
        recordCur = cl.query("{'a': {'$isnull': 0}}", null, null, "{'': 'a'}");
        actDataList = TransUtils.getReadActList(recordCur);
        Assert.assertEquals(actDataList, expDataList);
        actDataList.clear();
        
        cl.delete("{'a': {'$isnull' :0}}");
        Assert.assertEquals(cl.getCount(),0 );
        
    }
    
    //TODO:count num error, return -1 SEQUOIADBMAINSTREAM-4182
    @Test(enabled=false)
    public void test2(){
        
        sdb.beginTransaction();
        DBCollection transCL = sdb.getCollectionSpace(csName).getCollection(clName);
        BSONObject matcher = new BasicBSONObject("_id", data.get("_id"));
        transCL.delete(matcher);
        
        BSONObject testData = new BasicBSONObject();
        testData.put("_id", data.get("_id"));
        testData.put("a", data.get("a"));
        testData.put("b", 17122);
        testData.put("flag", "flag17122");
        //insert 
        transCL.insert(testData);
        
        sdb.commit();
        
        expDataList.clear();
        expDataList.add(testData);
        recordCur = cl.query("{'a': {'$isnull': 0}}", null, null, "{'': null}");
        actDataList = TransUtils.getReadActList(recordCur);
        Assert.assertEquals(actDataList, expDataList);
        actDataList.clear();
        
        recordCur = cl.query("{'a': {'$isnull': 0}}", null, null, "{'': 'a'}");
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
