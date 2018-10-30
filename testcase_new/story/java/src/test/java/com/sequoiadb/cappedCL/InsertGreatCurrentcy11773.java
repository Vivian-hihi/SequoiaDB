package com.sequoiadb.cappedCL;

import java.text.SimpleDateFormat;
import java.util.Date;

import org.bson.BSONObject;
import org.bson.BasicBSONObject;
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
* FileName: InsertGreatCurrentcy11773.java
* test content:test concurrentcy insert for cappedCL
* @author liuxiaoxuan
    * @Date    2017.7.18
*/
public class InsertGreatCurrentcy11773 extends SdbTestBase{

   private Sequoiadb sdb = null;
   private DBCollection cappedCL_11773 = null;
   private String cappedCSName = "story_java_cappedCS_11773";
   private String cappedCLName = "cappedCL_11773";
   private SimpleDateFormat sdf = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss.S");
	
   @BeforeClass
   public void setUp() {
      System.out.println(this.getClass().getName()+" begin at "+sdf.format(new Date()));
      boolean isCapped = true;
      sdb = new Sequoiadb(SdbTestBase.coordUrl, "","");
      //create cl
      cappedCL_11773 = CappedCLUtils.createCL(sdb, cappedCSName, cappedCLName, isCapped);
   }
	
   @Test
   public void testGreatConcurrencyInsert() {
      StringBuffer strBuffer =  new StringBuffer();
		
      int threadNums = 20;
      InsertThread insertThread = new InsertThread(strBuffer);
      insertThread.start(threadNums);
		
      Assert.assertTrue(insertThread.isSuccess(),insertThread.getErrorMsg());
		
      //check primary and slave datas
      sdb.setSessionAttr((BSONObject)JSON.parse("{PreferedInstance:'M'}"));
      DBCollection primaryCL = sdb.getCollectionSpace(cappedCSName).getCollection(cappedCLName);
      sdb.setSessionAttr((BSONObject)JSON.parse("{PreferedInstance:'S'}"));
      DBCollection slaveCL = sdb.getCollectionSpace(cappedCSName).getCollection(cappedCLName);
		
      Assert.assertTrue(CappedCLUtils.checkLogicalID(primaryCL, strBuffer.length(),this.getClass().getName()));
      System.out.println("-------sueccess to check primary node-------");
      Assert.assertTrue(CappedCLUtils.checkLogicalID(slaveCL, strBuffer.length(),this.getClass().getName()));
      System.out.println("-------sueccess to check slave node-------");
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
         System.out.println(this.getClass().getName()+" end at "+sdf.format(new Date()));
      }
   }
	
   private class InsertThread extends SdbThreadBase{
    	
      StringBuffer strBuffer = null;
      BSONObject obj = null;
    	
      public InsertThread(StringBuffer strBuffer) {
         this.obj = new BasicBSONObject();
         this.strBuffer = strBuffer;
         int stringLength = CappedCLUtils.getRandomStringLength();
         for(int len = 0; len < stringLength; len++) {    
            strBuffer.append("a");
         }
         obj.put("a", strBuffer.toString()); 
      }
    	
      @Override
      public void exec() throws Exception{
         Sequoiadb db = null;
         DBCollection cl = null;
         try{
            db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
            cl = db.getCollectionSpace(cappedCSName).getCollection(cappedCLName);
            // insert records in cappedCL
            CappedCLUtils.insertRecords(cl,obj);
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

