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
 * @Description seqDB-17129 : 更新已提交记录与本事务中插入的记录唯一索引重复 
 * @author luweikang
 * @date 2019年1月15日
 */
@Test(groups = "rc")
public class Transaction17129 extends SdbTestBase {
    
    private String clName = "transCL_17129";
    private Sequoiadb sdb = null;
    private DBCollection cl = null;
    private BSONObject data = null;
    private BSONObject data2 = null;
    private BSONObject modifier = null;
    private DBCursor recordCur = null;
    private List<BSONObject> expDataList = null;
    private List<BSONObject> actDataList = null;
    
    @BeforeClass
    public void setUp(){
        sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "");
        cl = sdb.getCollectionSpace(csName).createCollection(clName);
        data = new BasicBSONObject();
        data.put("a", 1);
        data.put("b", "testTrans_17129");
        data.put("c", 13700000000L);
        data.put("d", "customer transaction type data application.");
        
        data2 = new BasicBSONObject();
        data2.put("a", 2);
        data2.put("b", 1024);
        data2.put("c", 13700000000L);
        data2.put("d", "customer transaction type data application.");
        
        expDataList = new ArrayList<BSONObject>();
        expDataList.add(data);
        expDataList.add(data2);
        cl.insert(expDataList);
        cl.createIndex("a", "{a:1}", true, false);
        
        BSONObject updateData = new BasicBSONObject();
        updateData.put("_id", "id17129");
        updateData.put("a", "testTrans_17129Update");
        updateData.put("b", 1);
        updateData.put("c", 13700000000L);
        updateData.put("d", "customer transaction type data application.");
        
        modifier = new BasicBSONObject();
        modifier.put("$set", updateData);
        
    }
    
    @Test
    public void test(){
        try(Sequoiadb transDB = new Sequoiadb(SdbTestBase.coordUrl, "", "");){
            transDB.beginTransaction();
            DBCollection transCL = transDB.getCollectionSpace(csName).getCollection(clName);
            //update record R1 to R3
            transCL.update(new BasicBSONObject("a", 1), modifier, null);
            //update record R2 to R4 same as the R3
            transCL.update(new BasicBSONObject("a", 2), modifier, null);
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
