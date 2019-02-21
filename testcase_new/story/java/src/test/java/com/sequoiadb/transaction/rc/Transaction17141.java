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
 * @Description Transaction17141.java  query.remove操作时，事务提交，与读并发 
 * @author luweikang
 * @date 2019年1月15日
 */
@Test(groups = "rc")
public class Transaction17141 extends SdbTestBase {
    
    private String clName = "transCL_17141";
    private Sequoiadb sdb = null;
    private Sequoiadb sdb2 = null;
    private DBCollection cl = null;
    private BSONObject data = null;
    private BSONObject data2 = null;
    private DBCursor recordCur = null;
    private List<BSONObject> expDataList = null;
    private List<BSONObject> actDataList = null;
    
    @BeforeClass
    public void setUp(){
        sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "");
        sdb2 = new Sequoiadb(SdbTestBase.coordUrl, "", "");
        cl = sdb.getCollectionSpace(csName).createCollection(clName);
        cl.createIndex("a", "{a:1}", false, false);
        expDataList = new ArrayList<BSONObject>();
        data = new BasicBSONObject();
        data.put("_id", "insertID17141");
        data.put("a", 1);
        data.put("b", 1);
        data.put("c", 13700000000L);
        data.put("d", "customer transaction type data application.");
        cl.insert(data);
        
        data2 = new BasicBSONObject();
        data2.put("_id", "updateID17141");
        data2.put("a", 2);
        data2.put("b", "update2");
        data2.put("c", 13700000000L);
        data2.put("d", "customer transaction type data application.");
        cl.insert(data2);
    }
    
    @Test
    public void test(){
        
        sdb.beginTransaction();
        sdb2.beginTransaction();
        DBCollection CLTrans1 = sdb.getCollectionSpace(csName).getCollection(clName);
        DBCollection CLTrans2 = sdb2.getCollectionSpace(csName).getCollection(clName);
        
        //2 query.update
        DBCursor tbCur = CLTrans1.queryAndRemove(new BasicBSONObject("a", 1), null, null, null, -1, -1, 0);
        BSONObject actData = tbCur.getNext();
        try {
            Assert.assertEquals(actData, data);
        } finally {
            if(tbCur != null){
                tbCur.close();
            }
        }
        
        expDataList.add(data2);
        //trans1 query 
        recordCur = CLTrans1.query("{'a': {'$isnull': 0}}", null, "{a:1}", "{'': null}");
        actDataList = TransUtils.getReadActList(recordCur);
        Assert.assertEquals(actDataList, expDataList);
        actDataList.clear();
        
        recordCur = CLTrans1.query("{'a': {'$isnull': 0}}", null, "{a:1}", "{'': 'a'}");
        actDataList = TransUtils.getReadActList(recordCur);
        Assert.assertEquals(actDataList, expDataList);
        actDataList.clear();
        
        expDataList.clear();
        expDataList.add(data);
        expDataList.add(data2);
        //3 trans2 query 
        recordCur = CLTrans2.query("{'a': {'$isnull': 0}}", null, "{a:1}", "{'': null}");
        actDataList = TransUtils.getReadActList(recordCur);
        Assert.assertEquals(actDataList, expDataList);
        actDataList.clear();
        
        recordCur = CLTrans2.query("{'a': {'$isnull': 0}}", null, "{a:1}", "{'': 'a'}");
        actDataList = TransUtils.getReadActList(recordCur);
        Assert.assertEquals(actDataList, expDataList);
        actDataList.clear();
        
        expDataList.clear();
        expDataList.add(data2);
        //4 commit trans1 and query
        sdb.commit();
        recordCur = cl.query("{'a': {'$isnull': 0}}", null, "{a:1}", "{'': null}");
        actDataList = TransUtils.getReadActList(recordCur);
        Assert.assertEquals(actDataList, expDataList);
        actDataList.clear();
        
        recordCur = cl.query("{'a': {'$isnull': 0}}", null, "{a:1}", "{'': 'a'}");
        actDataList = TransUtils.getReadActList(recordCur);
        Assert.assertEquals(actDataList, expDataList);
        actDataList.clear();
        
        //5 trans2 query
        recordCur = CLTrans2.query("{'a': {'$isnull': 0}}", null, "{a:1}", "{'': null}");
        actDataList = TransUtils.getReadActList(recordCur);
        Assert.assertEquals(actDataList, expDataList);
        actDataList.clear();
        
        recordCur = CLTrans2.query("{'a': {'$isnull': 0}}", null, "{a:1}", "{'': 'a'}");
        actDataList = TransUtils.getReadActList(recordCur);
        Assert.assertEquals(actDataList, expDataList);
        actDataList.clear();

        //6 commit trans2 and query
        sdb2.commit();
        recordCur = cl.query("{'a': {'$isnull': 0}}", null, "{a:1}", "{'': null}");
        actDataList = TransUtils.getReadActList(recordCur);
        Assert.assertEquals(actDataList, expDataList);
        actDataList.clear();
        
        recordCur = cl.query("{'a': {'$isnull': 0}}", null, "{a:1}", "{'': 'a'}");
        actDataList = TransUtils.getReadActList(recordCur);
        Assert.assertEquals(actDataList, expDataList);
        actDataList.clear();
    }
    
    @AfterClass(enabled=false)
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
    
}
