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
* FileName: QueryGreatConcurrentcy11806.java
* test content:test concurrentcy query for cappedCL
* @author liuxiaoxuan
    * @Date    2017.7.24
*/
public class QueryGreatConcurrentcy11806 extends SdbTestBase{
	
	private Sequoiadb sdb = null;
	private DBCollection cappedCL_11806 = null;
	private String cappedCSName_11806 = "story_java_cappedCS_11806";
	private String cappedCLName_11806 = "cappedCL_11806";
	private SimpleDateFormat sdf = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss.S");
	private int stringLength = 0;
	
	@BeforeClass
	public void setUp() {
		System.out.println(this.getClass().getName()+" begin at "+sdf.format(new Date()));
		try {
			boolean isCapped = true;
			sdb = new Sequoiadb(SdbTestBase.coordUrl, "","");
			sdb.setSessionAttr((BSONObject)JSON.parse("{PreferedInstance:'M'}"));
			cappedCL_11806 = Commlib.createCL(sdb, cappedCSName_11806, cappedCLName_11806, isCapped);
			stringLength = Commlib.getRandomStringLength();
			Commlib.insertRecords(cappedCL_11806,stringLength,2000);//init insert 2000 records
		}catch(BaseException e) {
			Assert.fail(e.getMessage());
		}
	}
	
	@Test
	public void testGreatConcurrencyQuery() {
		QueryThread queryThread = new QueryThread();
		int threadNum = 10;
		queryThread.start(threadNum);
		
		Assert.assertTrue(queryThread.isSuccess(),queryThread.getErrorMsg());
	}
	
	@AfterClass
	public void tearDown() {
		try {
			CollectionSpace cs = sdb.getCollectionSpace(cappedCSName_11806);
			if(cs != null && cs.isCollectionExist(cappedCLName_11806)) {
				sdb.dropCollectionSpace(cappedCSName_11806);
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
	                cl = db.getCollectionSpace(cappedCSName_11806).getCollection(cappedCLName_11806);
	                // find records in cappedCL
	                Commlib.checkLogicalID(cl,stringLength,Thread.currentThread().getName());
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
