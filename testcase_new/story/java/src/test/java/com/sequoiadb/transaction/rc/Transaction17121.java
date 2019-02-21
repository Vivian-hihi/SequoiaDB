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
 * @Description Transaction17121.java 插入记录与本事务中更新的记录重复   
 * @author luweikang
 * @date 2019年1月15日
 */
@Test(groups="rc")
public class Transaction17121 extends SdbTestBase {
    
    private String clName = "transCL_17121";
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
        data.put("b", "testTrans_17121");
        data.put("c", 13700000000L);
        data.put("d", "customer transaction type data application.");
        cl.insert(data);
        expDataList = new ArrayList<BSONObject>();
        expDataList.add(data);
    }
    
    @Test
    public void test(){
        
        try(Sequoiadb transDB = new Sequoiadb(SdbTestBase.coordUrl, "", "")){
            transDB.beginTransaction();
            DBCollection transCL = transDB.getCollectionSpace(csName).getCollection(clName);
            BSONObject value = new BasicBSONObject();
            value.put("_id", "id17121");
            value.put("a", 17121);
            value.put("b", "testTrans_17121" );
            value.put("c", 13700000000L);
            value.put("d", "customer transaction type data application.");
            BSONObject modifier = new BasicBSONObject("$set", value);
            //update record R1 to R2
            transCL.update(new BasicBSONObject("a", data.get("a")), modifier, null);
            //insert record R3 same as the R2  
            transCL.insert(value);
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
