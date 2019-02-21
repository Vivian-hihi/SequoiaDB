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
import com.sequoiadb.testcommon.SdbTestBase;
import com.sequoiadb.testcommon.SdbThreadBase;
import com.sequoiadb.transaction.TransUtils;

/**
 * @Description Transaction17260.java  upsert操作时，事务提交，与读并发 
 * @author luweikang
 * @date 2019年1月15日
 */
@Test(groups = "ru")
public class Transaction17260 extends SdbTestBase {
    
    private String clName = "transCL_17260";
    private Sequoiadb sdb = null;
    private Sequoiadb sdb2 = null;
    private DBCollection cl = null;
    private int recordNum = 1000;
    private DBCursor recordCur = null;
    private List<BSONObject> dataList = null;
    private List<BSONObject> expDataList = null;
    private List<BSONObject> actDataList = null;
    
    @BeforeClass
    public void setUp(){
        sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "");
        sdb2 = new Sequoiadb(SdbTestBase.coordUrl, "", "");
        cl = sdb.getCollectionSpace(csName).createCollection(clName);
        cl.createIndex("a", "{a:1}", false, false);
        dataList = prepareData( recordNum );
        cl.insert(dataList);
        expDataList = new ArrayList<BSONObject>();
        
    }
    
    @Test
    public void test(){
        
        sdb.beginTransaction();
        UpsertThread upsertThread = new UpsertThread();
        upsertThread.start();
        
        sdb2.beginTransaction();
        QueryThread queryThread = new QueryThread();
        queryThread.start();
        
        Assert.assertTrue(upsertThread.isSuccess(), upsertThread.getErrorMsg());
        Assert.assertTrue(queryThread.isSuccess(), queryThread.getErrorMsg());
        
        sdb.commit();
        
        recordCur = cl.query("{'a': {'$isnull': 0}}", null, "{a:1}", "{'': null}");
        actDataList = TransUtils.getReadActList(recordCur);
        Assert.assertEquals(actDataList, expDataList);
        actDataList.clear();
        
        recordCur = cl.query("{'a': {'$isnull': 0}}", null, "{a:1}", "{'': 'a'}");
        actDataList = TransUtils.getReadActList(recordCur);
        Assert.assertEquals(actDataList, expDataList);
        actDataList.clear();
        
        sdb2.commit();
        
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
            if( sdb2 != null ){
                sdb2.close();
            }
        }
    }
    
    private List<BSONObject> prepareData(int recordNum){
        List<BSONObject> dataList = new ArrayList<BSONObject>();
        BSONObject data = null;
        for (int i = 0; i < recordNum; i++) {
            data = new BasicBSONObject();
            data.put("a", i * 2);
            data.put("b", "testTrans_17260");
            data.put("c", 13700000000L);
            data.put("d", "customer transaction type data application.");
            dataList.add(data);
        }
        return dataList;
    }
    
//    private List<BSONObject> expData(){
//        List<BSONObject> dataList = new ArrayList<BSONObject>();
//        BSONObject data = null;
//        for (int i = 0; i < recordNum * 2; i++) {
//            data = new BasicBSONObject();
//            data.put("a", i);
//            data.put("b", "test_update_" + i);
//            data.put("c", 13700000000L);
//            data.put("c", "customer transaction type data application.");
//            dataList.add(data);
//        }
//        return dataList;
//    }
    
    private class UpsertThread extends SdbThreadBase {
        
        public void exec() {
            BSONObject modifier = null;
            BSONObject data = null;
            for (int i = 0; i < recordNum * 2; i++) {
                modifier = new BasicBSONObject();
                data = new BasicBSONObject();
                data.put("_id", "upsert17260_" + i);
                data.put("a", i);
                data.put("b", "test_update_" + i);
                data.put("c", 13700000000L);
                data.put("d", "customer transaction type data application.");
                modifier.put("$set", data);
                cl.upsert(new BasicBSONObject("a", i), modifier, null);
                expDataList.add(data);
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
