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
import com.sequoiadb.testcommon.SdbThreadBase;
import com.sequoiadb.transaction.TransUtils;

/**
 * @Description Transaction17137.java  创建/删除索引与事务操作并发  
 * @author luweikang
 * @date 2019年1月15日
 */
@Test(groups = "rc")
public class Transaction17137 extends SdbTestBase {
    
    private String clName = "transCL_17137";
    private Sequoiadb sdb = null;
    private Sequoiadb sdb2 = null;
    private DBCollection cl = null;
    private int recordNum = 10000;
    private DBCursor recordCur = null;
    private List<BSONObject> expDataList = null;
    private List<BSONObject> actDataList = null;
    
    @BeforeClass
    public void setUp(){
        sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "");
        sdb2 = new Sequoiadb(SdbTestBase.coordUrl, "", "");
        cl = sdb.getCollectionSpace(csName).createCollection(clName);
        expDataList = prepareData( recordNum );
        
    }
    
    @Test
    public void test(){
        
        sdb.beginTransaction();
        CRUDThread crudThread = new CRUDThread();
        crudThread.start();
        
        IndexThread indexThread = new IndexThread();
        indexThread.start();
        
        sdb2.beginTransaction();
        QueryThread queryThread = new QueryThread();
        queryThread.start();
        
        Assert.assertTrue(crudThread.isSuccess(), crudThread.getErrorMsg());
        Assert.assertTrue(indexThread.isSuccess(), indexThread.getErrorMsg());
        Assert.assertTrue(queryThread.isSuccess(), queryThread.getErrorMsg());
        
        sdb.commit();
        
        expDataList.clear();
        expDataList = expData();
        
        recordCur = cl.query("{'a': {'$isnull': 0}}", "{'_id': {'$include': 0}}", null, "{'': null}");
        actDataList = TransUtils.getReadActList(recordCur);
        Assert.assertEquals(actDataList, expDataList);
        actDataList.clear();
        
        recordCur = cl.query("{'a': {'$isnull': 0}}", "{'_id': {'$include': 0}}", null, "{'': 'a'}");
        actDataList = TransUtils.getReadActList(recordCur);
        Assert.assertEquals(actDataList, expDataList);
        actDataList.clear();
        
        sdb2.commit();
        Assert.assertFalse(cl.isIndexExist("a"));
    }
    
    @AfterClass(enabled = false)
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
            if( sdb2 != null ){
                sdb2.close();
            }
        }
    }
    
    private List<BSONObject> prepareData(int recordNum){
        List<BSONObject> dataList = new ArrayList<BSONObject>();
        for (int i = 0; i < recordNum; i++) {
            BSONObject data = new BasicBSONObject();
            data.put("a", i);
            data.put("b", "testTrans_17137_" + i);
            data.put("c", 13700000000L);
            data.put("d", "customer transaction type data application.");
            dataList.add(data);
        }
        return dataList;
    }
    
    private List<BSONObject> expData(){
        List<BSONObject> dataList = new ArrayList<BSONObject>();
        BSONObject data = null;
        for (int i = 0; i < recordNum /2; i++) {
            data = new BasicBSONObject();
            data.put("a", 1024);
            data.put("b", "test_update_1024");
            data.put("c", 13700000000L);
            data.put("d", "customer transaction type data application.");
            dataList.add(data);
        }
        return dataList;
    }
    
    private class CRUDThread extends SdbThreadBase {
        
        public void exec() {
            cl.insert(expDataList);
            
            String modifier = "{'$set':{ 'a': 1024, 'b': 'test_update_1024'}}"; 
            cl.update("{'a':{'$gte':0, '$lt': " + recordNum / 2 + "}}", modifier, null);
            
            cl.delete("{'a':{'$gte':" + recordNum / 2 + ", '$lt': " + recordNum + "}}");
        }
    }
    
    private class IndexThread extends SdbThreadBase {
        
        public void exec() {
            try( Sequoiadb db = new Sequoiadb(SdbTestBase.coordUrl, "", "") ) {
                DBCollection dbcl = db.getCollectionSpace(csName).getCollection(clName);
                for(int i=0; i<10; i++){
                    dbcl.createIndex("a", "{a:1, b:-1}", false, false);
                    dbcl.dropIndex("a");
                }
            }catch (Exception e) {
                e.printStackTrace();
                throw e;
            }
        }
    }
    
    private class QueryThread extends SdbThreadBase {
        
        public void exec() {
            DBCursor cur = null;
            try {
                DBCollection dbcl = sdb2.getCollectionSpace(csName).getCollection(clName);
                cur = dbcl.query("{'a': {'$isnull':0}}", null, null, "{'': null}");
                while(cur.hasNext()){
                    cur.getNext();
                }
                cur.close();
                
                cur = dbcl.query("{'a': {'$isnull':0}}", null, null, "{'': 'a'}");
                while(cur.hasNext()){
                    cur.getNext();
                }
            }finally{
                if( cur != null){
                    cur.close();
                }
            }
        }
    }
    
}
