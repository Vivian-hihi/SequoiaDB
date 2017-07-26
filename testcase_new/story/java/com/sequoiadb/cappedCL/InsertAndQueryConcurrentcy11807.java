package com.sequoiadb.cappedCL;

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
	private DBCollection cappedCL_11807 = null;
	private String cappedCSName_11807 = "story_java_cappedCS_11807";
	private String cappedCLName_11807 = "cappedCL_11807";
	private int stringLength = 0;
    private final int INIT_RECORDNUMS_5000 = 5000;
	private SimpleDateFormat sdf = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss.S");
	
	
	@BeforeClass
	public void setUp() {
		System.out.println(this.getClass().getName()+" begin at "+sdf.format(new Date()));
		try {
			boolean isCapped = true;
			sdb = new Sequoiadb(SdbTestBase.coordUrl, "","");
			sdb.setSessionAttr((BSONObject)JSON.parse("{PreferedInstance:'M'}"));
			cappedCL_11807 = Commlib.createCL(sdb, cappedCSName_11807, cappedCLName_11807, isCapped);
			stringLength = Commlib.getRandomStringLength();
			Commlib.insertRecords(cappedCL_11807,stringLength,INIT_RECORDNUMS_5000);//init insert 5000 records 
		}catch(BaseException e) {
			Assert.fail(e.getMessage());
		}
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
			CollectionSpace cs = sdb.getCollectionSpace(cappedCSName_11807);
			if(cs != null && cs.isCollectionExist(cappedCLName_11807)) {
				sdb.dropCollectionSpace(cappedCSName_11807);
			}
		}catch (BaseException e) {
			Assert.fail(e.getMessage());
		}finally {
			sdb.close();
			System.out.println(this.getClass().getName()+" end at "+sdf.format(new Date()));
		}
	}
	
	private class QueryThread extends SdbThreadBase{
		
    	@Override
        public void exec() throws Exception{
            Sequoiadb db = null;
            DBCollection cl = null;
            try{
                db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
                db.setSessionAttr((BSONObject)JSON.parse("{PreferedInstance:'M'}"));
                cl = db.getCollectionSpace(cappedCSName_11807).getCollection(cappedCLName_11807);
                // find records in cappedCL
                
                Commlib.checkLogicalID(cl, stringLength, Thread.currentThread().getName());
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
	                db.setSessionAttr((BSONObject)JSON.parse("{PreferedInstance:'M'}"));
	                cl = db.getCollectionSpace(cappedCSName_11807).getCollection(cappedCLName_11807);
	                // insert records in cappedCL	              
	                int insert_new_recordNum = 10;
	                Commlib.insertRecords(cl,stringLength,insert_new_recordNum);
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
