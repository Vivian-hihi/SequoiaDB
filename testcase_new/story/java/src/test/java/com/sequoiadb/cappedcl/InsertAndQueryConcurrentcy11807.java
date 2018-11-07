package com.sequoiadb.cappedcl;

import java.text.SimpleDateFormat;
import java.util.Date;

import org.bson.BSONObject;
import org.bson.util.JSON;
import org.testng.Assert;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import com.sequoiadb.base.CollectionSpace;
import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.testcommon.SdbTestBase;
import com.sequoiadb.testcommon.SdbThreadBase;

/**
* FileName: InsertAndQueryConcurrentcy11807.java
* test content:test concurrentcy  insert and query at the same time for cappedCL
* @author liuxiaoxuan
    * @Date    2017.7.24
*/
public class InsertAndQueryConcurrentcy11807 extends SdbTestBase{

   private Sequoiadb sdb = null;
   private DBCollection cappedCL = null;
   private String cappedCSName = "story_java_cappedCS_11807";
   private String cappedCLName = "cappedCL_11807";
   private int stringLength = 0;
   private final int INITRECORDNUMS = 5000;
   private SimpleDateFormat sdf = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss.S");
	
   @BeforeClass
   public void setUp() {
      System.out.println(this.getClass().getName()+" begin at "+sdf.format(new Date()));
      boolean isCapped = true;
      sdb = new Sequoiadb(SdbTestBase.coordUrl, "","");
      cappedCL = CappedCLUtils.createCL(sdb, cappedCSName, cappedCLName, isCapped);
      stringLength = CappedCLUtils.getRandomStringLength();
      CappedCLUtils.insertRecords(cappedCL,stringLength,INITRECORDNUMS);//init insert 5000 records 
   }
	
   @Test
   public void testGreatConcurrencyQuery() {
      QueryThread queryThread = new QueryThread();
      InsertThread insertThread = new InsertThread();
		
      queryThread.start();	
      insertThread.start();
		
      Assert.assertTrue(queryThread.isSuccess(),queryThread.getErrorMsg());
      Assert.assertTrue(insertThread.isSuccess(),queryThread.getErrorMsg());
   }
	
   @AfterClass
   public void tearDown() {
      try {
         CollectionSpace cs = sdb.getCollectionSpace(cappedCSName);
         if(cs != null && cs.isCollectionExist(cappedCLName)) {
            sdb.dropCollectionSpace(cappedCSName);
         }
      }catch (BaseException e) {
         e.printStackTrace();
      }finally {
         sdb.close();
      }
   }
	
   private class QueryThread extends SdbThreadBase{
		
      @Override
      public void exec() throws Exception{
         Sequoiadb db = null;
         DBCollection cl = null;
         try{
            db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
            // check records from primary and slave node
            cl = db.getCollectionSpace(cappedCSName).getCollection(cappedCLName);		 
            CappedCLUtils.checkLogicalID(cl, stringLength, Thread.currentThread().getName());  
         }catch(BaseException e){
            if(e.getErrorCode() != -23 || e.getErrorCode() != -34){
               throw e;
            }
         }finally{
            db.close();
         }
      }
   }
	
   private class InsertThread extends SdbThreadBase{
	    	
      @Override
      public void exec() throws Exception{
         Sequoiadb db = null;
         DBCollection cl = null;
         try{
            db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
            cl = db.getCollectionSpace(cappedCSName).getCollection(cappedCLName);
            // insert records in cappedCL	              
            int insert_new_recordNum = 10;
            CappedCLUtils.insertRecords(cl,stringLength,insert_new_recordNum);
         }catch(BaseException e){
            if(e.getErrorCode() != -23 || e.getErrorCode() != -34){
               throw e;
            }
         }finally{
            db.close();
         }
      }
   }
}
