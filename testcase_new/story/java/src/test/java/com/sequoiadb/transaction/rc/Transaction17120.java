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
 * @Description Transaction17120.java 插入记录与本事务中插入的记录重复  
 * @author luweikang
 * @date 2019年1月15日
 */
@Test(groups = "rc")
public class Transaction17120 extends SdbTestBase {
    
    private String clName = "transCL_17120";
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
        data.put("a", 1);
        data.put("b", "testTrans_17120");
        data.put("c", 13700000000L);
        data.put("d", "customer transaction type data application.");
        cl.insert(data);
        cl.createIndex("a", "{a:1}", true, false);
        expDataList = new ArrayList<BSONObject>();
        expDataList.add(data);
    }
    
    @Test
    public void test(){
        
        try(Sequoiadb transDB = new Sequoiadb(SdbTestBase.coordUrl, "", "")){
            BSONObject testData = new BasicBSONObject();
            testData.put("a", 17120);
            testData.put("b", "testTrans_17120" );
            testData.put("c", 13700017120L);
            testData.put("d", "customer transaction type data application. :17120");
            transDB.beginTransaction();
            DBCollection transCL = transDB.getCollectionSpace(csName).getCollection(clName);
            transCL.insert(testData);
            //insert the same record repeatedly
            transCL.insert(testData);
            Assert.fail("insert an existing record with an index,should be failed");
        }catch (BaseException e) {
            Assert.assertEquals(e.getErrorCode(), -38, e.getMessage());
        }
        
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