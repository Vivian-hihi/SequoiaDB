package com.sequoiadb.cappedcl;

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
* FileName: InsertCommonCLAndCappedCL11782.java
* test content:test concurrentcy insert for commonCL and cappedCL 
* @author liuxiaoxuan
    * @Date    2017.7.18
*/

public class InsertCommonCLAndCappedCL11782 extends SdbTestBase{

   private Sequoiadb sdb = null;
   private SimpleDateFormat sdf = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss.S");
   private String cappedCSName = "story_java_cappedCS_11782";
   private String commonCSName = "story_java_commonCS_11782";
   private String cappedClName = "cappedCL_11782";
   private String commonClName = "commonCL_11782";
	
   @BeforeClass
   public void setUp() {
      System.out.println(this.getClass().getName()+" begin at "+sdf.format(new Date()));
      boolean isCapped = true;
      sdb = new Sequoiadb(SdbTestBase.coordUrl, "","");
      sdb.setSessionAttr((BSONObject)JSON.parse("{PreferedInstance:'M'}"));
      CappedCLUtils.createCL(sdb, cappedCSName, cappedClName ,isCapped);
      CappedCLUtils.createCL(sdb, commonCSName, commonClName ,false);
   }
	
   @Test
   public void testGreatConcurrencyInsert(){
      StringBuffer strBuffer =  new StringBuffer();
      InsertThread insertThread = new InsertThread(strBuffer);
		
      int threadNum = 20;
      insertThread.start(threadNum);
		
      Assert.assertTrue(insertThread.isSuccess(),insertThread.getErrorMsg());

      //check primary and slave datas
      sdb.setSessionAttr((BSONObject)JSON.parse("{PreferedInstance:'M'}"));
      DBCollection primaryCL = sdb.getCollectionSpace(cappedCSName).getCollection(cappedClName);
      sdb.setSessionAttr((BSONObject)JSON.parse("{PreferedInstance:'S'}"));
      DBCollection slaveCL = sdb.getCollectionSpace(cappedCSName).getCollection(cappedClName);
		
      Assert.assertTrue(CappedCLUtils.checkLogicalID(primaryCL, strBuffer.length(),this.getClass().getName()));
      System.out.println("-------sueccess to check primary node-------");
      Assert.assertTrue(CappedCLUtils.checkLogicalID(slaveCL, strBuffer.length(),this.getClass().getName()));
      System.out.println("-------sueccess to check slave node-------");
   }
	
   @AfterClass
   public void tearDown() {
      try {
         CollectionSpace cappedCS = sdb.getCollectionSpace(cappedCSName);
         CollectionSpace commonCS = sdb.getCollectionSpace(commonCSName);
         if(cappedCS != null ) {
            sdb.dropCollectionSpace(cappedCSName);	
         }
         if( commonCS != null) {
            sdb.dropCollectionSpace(commonCSName);
         }
      }catch (BaseException e) {
         e.printStackTrace();
      }finally {
         sdb.close();
         System.out.println(this.getClass().getName()+" end at "+sdf.format(new Date()));
      }
   }
	
   private class InsertThread extends SdbThreadBase {
		
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
      public void exec() throws BaseException{
         Sequoiadb db = null;
         DBCollection capCl = null;
         DBCollection commCl = null;
         try{
            db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
            capCl = db.getCollectionSpace(cappedCSName).getCollection(cappedClName);
            commCl = db.getCollectionSpace(commonCSName).getCollection(commonClName);
            //insert records in cappedCL and commonCL
            CappedCLUtils.insertRecords(capCl,obj);
            CappedCLUtils.insertRecords(commCl,obj);
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
