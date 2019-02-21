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
import com.sequoiadb.testcommon.SdbThreadBase;
import com.sequoiadb.transaction.TransUtils;

/**
 * @Description seqDB-17360.java  插入与删除并发， 删除的记录同时匹配已提交记录及其他事务插入的记录，删除走索引，事务提交，过程中读  
 * @author luweikang
 * @date 2019年1月15日
 */
@Test(groups = "rc")
public class Transaction17360 extends SdbTestBase {
    
    private String clName = "transCL_17360";
    private Sequoiadb sdb = null;
    private Sequoiadb sdb1 = null;
    private Sequoiadb sdb2 = null;
    private Sequoiadb sdb3 = null;
    private DBCollection CLTrans1 = null;
    private DBCollection CLTrans2 = null;
    private DBCollection CLTrans3 = null;
    private DBCollection cl = null;
    private BSONObject data = null;
    private BSONObject data2 = null;
    private DBCursor recordCur = null;
    private List<BSONObject> expDataList = null;
    private List<BSONObject> actDataList = null;
    
    @BeforeClass
    public void setUp(){
        sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "");
        sdb1 = new Sequoiadb(SdbTestBase.coordUrl, "", "");
        sdb2 = new Sequoiadb(SdbTestBase.coordUrl, "", "");
        sdb3 = new Sequoiadb(SdbTestBase.coordUrl, "", "");
        cl = sdb.getCollectionSpace(csName).createCollection(clName);
        cl.createIndex("a", "{a: 1}", true, false);
        expDataList = new ArrayList<BSONObject>();
        data = new BasicBSONObject();
        data.put("_id", "insertID17360_2");
        data.put("a", 2);
        data.put("b", 2);
        data.put("c", 13700000000L);
        data.put("d", "customer transaction type data application.");
        cl.insert(data);
        
        data2 = new BasicBSONObject();
        data2.put("_id", "insertID17360_1");
        data2.put("a", 1);
        data2.put("b", 1);
        data2.put("c", 13700000000L);
        data2.put("d", "customer transaction type data application.");
        
        sdb1.beginTransaction();
        sdb2.beginTransaction();
        sdb3.beginTransaction();
        CLTrans1 = sdb1.getCollectionSpace(csName).getCollection(clName);
        CLTrans2 = sdb2.getCollectionSpace(csName).getCollection(clName);
        CLTrans3 = sdb3.getCollectionSpace(csName).getCollection(clName);
    }
    
    //TODO:SEQUOIADBMAINSTREAM-4193
    @Test(enabled=false)
    public void test(){
        
        //2 trans1 insert data2 R1 > R2
        CLTrans1.insert(data2);
        
        //3 trans2 delete r1 and r2
        DeleteThread deleteThread = new DeleteThread();
        deleteThread.start();
        Assert.assertTrue(deleteThread.matchBlockingMethod(CLTrans2.getClass().getName(), "delete"));
        
      //6 no trans read
        expDataList.clear();
//        expDataList.add(data);
        expDataList.add(data2);
        recordCur = cl.query("{'a': {'$isnull': 0}}", null, "{a: -1}", "{'': null}");
        actDataList = TransUtils.getReadActList(recordCur);
        Assert.assertEquals(actDataList, expDataList);
        actDataList.clear();
        
        recordCur = cl.query("{'a': {'$isnull': 0}}", null, "{a: -1}", "{'': 'a'}");
        actDataList = TransUtils.getReadActList(recordCur);
        Assert.assertEquals(actDataList, expDataList);
        actDataList.clear();
        
        expDataList.add(data);
        expDataList.add(data2);
        //4 trans1 read
        recordCur = CLTrans1.query("{'a': {'$isnull': 0}}", null, "{a:-1}", "{'': null}");
        actDataList = TransUtils.getReadActList(recordCur);
        Assert.assertEquals(actDataList, expDataList);
        actDataList.clear();

        recordCur = CLTrans1.query("{'a': {'$isnull': 0}}", null, "{a:-1}", "{'': 'a'}");
        actDataList = TransUtils.getReadActList(recordCur);
        Assert.assertEquals(actDataList, expDataList);
        actDataList.clear();
        
        expDataList.clear();
        expDataList.add(data);
        //5 trans3 read
        recordCur = CLTrans3.query("{'a': {'$isnull': 0}}", null, "{a: -1}", "{'': null}");
        actDataList = TransUtils.getReadActList(recordCur);
        Assert.assertEquals(actDataList, expDataList);
        actDataList.clear();
        
        recordCur = CLTrans3.query("{'a': {'$isnull': 0}}", null, "{a: -1}", "{'': 'a'}");
        actDataList = TransUtils.getReadActList(recordCur);
        Assert.assertEquals(actDataList, expDataList);
        actDataList.clear();
        
        //6 no trans read
        expDataList.clear();
//        expDataList.add(data);
        expDataList.add(data2);
        recordCur = cl.query("{'a': {'$isnull': 0}}", null, "{a: -1}", "{'': null}");
        actDataList = TransUtils.getReadActList(recordCur);
        Assert.assertEquals(actDataList, expDataList);
        actDataList.clear();
        
        recordCur = cl.query("{'a': {'$isnull': 0}}", null, "{a: -1}", "{'': 'a'}");
        actDataList = TransUtils.getReadActList(recordCur);
        Assert.assertEquals(actDataList, expDataList);
        actDataList.clear();
        
        //trans1 commit check trans2 success 
        sdb1.commit();
        Assert.assertTrue(deleteThread.isSuccess(), deleteThread.getErrorMsg());
        Assert.assertFalse(deleteThread.matchBlockingMethod(CLTrans2.getClass().getName(), "delete"));
        
        //7 no trans read
        Assert.assertEquals(cl.getCount(new BasicBSONObject("a", new BasicBSONObject("$isnull", 0)), new BasicBSONObject("", null)), 0);
        Assert.assertEquals(cl.getCount(new BasicBSONObject("a", new BasicBSONObject("$isnull", 0)), new BasicBSONObject("", "a")), 0 );
        
        //8 trans2 read
        Assert.assertEquals(CLTrans2.getCount(new BasicBSONObject("a", new BasicBSONObject("$isnull", 0)), new BasicBSONObject("", null)), 0);
        Assert.assertEquals(CLTrans2.getCount(new BasicBSONObject("a", new BasicBSONObject("$isnull", 0)), new BasicBSONObject("", "a")), 0 );
        
        expDataList.clear();
        expDataList.add(data);
        expDataList.add(data2);
        //9 trans3 read
        recordCur = CLTrans3.query("{'a': {'$isnull': 0}}", null, "{a: -1}", "{'': null}");
        actDataList = TransUtils.getReadActList(recordCur);
        Assert.assertEquals(actDataList, expDataList);
        actDataList.clear();
        
        recordCur = CLTrans3.query("{'a': {'$isnull': 0}}", null, "{a: -1}", "{'': 'a'}");
        actDataList = TransUtils.getReadActList(recordCur);
        Assert.assertEquals(actDataList, expDataList);
        actDataList.clear();
        
        //10 read after trans2 commit 
        sdb2.commit();
        Assert.assertEquals(cl.getCount(new BasicBSONObject("a", new BasicBSONObject("$isnull", 0)), new BasicBSONObject("", null)), 0);
        Assert.assertEquals(cl.getCount(new BasicBSONObject("a", new BasicBSONObject("$isnull", 0)), new BasicBSONObject("", "a")), 0 );
        
        //11 trans3 read
        Assert.assertEquals(CLTrans3.getCount(new BasicBSONObject("a", new BasicBSONObject("$isnull", 0)), new BasicBSONObject("", null)), 0);
        Assert.assertEquals(CLTrans3.getCount(new BasicBSONObject("a", new BasicBSONObject("$isnull", 0)), new BasicBSONObject("", "a")), 0 );
        
        sdb3.commit();
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
            if( sdb1 != null ){
                sdb1.close();
            }
            if( sdb2 != null ){
                sdb2.close();
            }
            if( sdb3 != null ){
                sdb2.close();
            }
        }
    }
    
    private class DeleteThread extends SdbThreadBase {
        
        @Override
        public void exec() throws BaseException {
            CLTrans2.delete("{'a': {'$isnull': 0}}", "{'': 'a'}");
        }
    }
    
    
}
