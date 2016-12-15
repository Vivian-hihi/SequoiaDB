package com.sequoiadb.metadataConsistency.data;

import java.text.SimpleDateFormat;
import java.util.Date;

import org.testng.annotations.Test;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.AfterClass;
import org.bson.BSONObject;
import org.bson.BasicBSONObject;
import org.testng.Assert;
import org.testng.SkipException;

import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.metadataConsistency.data.CommLib;
import com.sequoiadb.testcommon.SdbTestBase;

/**
* TestLink: seqDB-10190: concurrency[attachCL, drop subCS]
* @author xiaoni huang init
* @Date   2016.10.14
*/

public class SubCL10190 extends SdbTestBase {
	private CommLib CommLib = new CommLib();
	private SimpleDateFormat dateFm = new SimpleDateFormat("YYYY-MM-dd HH:mm:ss");
	private static Sequoiadb sdb = null;
	private String csName = "cs10190";
	private String clName = "cl10190";
	private String mCSName = csName + "_m";
	private String sCSName = csName + "_s";
	private String mCLName = clName + "_m";
	private String sCLName = clName + "_s";
	
	@BeforeClass
	public void setUp(){
		//start time
		System.out.println("Begin to run " + this.getClass().getName() 
					+ ", begin in: " + dateFm.format(new Date().getTime()));
		try{
			sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "");
			//judge the mode
			if(CommLib.isStandAlone(sdb)){
				throw new SkipException("The mode is standlone, " + "skip the testCase.");
			}
			//clear env
			CommLib.clearCS(sdb, csName);
			//create cs/cl
			this.createMainCL(sdb);
		}catch(BaseException e){
			Assert.fail("Failed to prepare env at th begining. "
					+ "ErrorMsg:\n" +e.getMessage());
		}
	}
	
	@AfterClass
	public void tearDown(){
		try{
			//clear env
			CommLib.clearCS(sdb, csName);
		}catch(BaseException e){
			Assert.fail("ErrorMsg:\n" +e.getMessage());
		}finally{
			System.out.println("End to run " + this.getClass().getName() 
						+ ", end in: " + dateFm.format(new Date().getTime()));
			sdb.disconnect();
		}
	}
	
	@Test(invocationCount = 10, threadPoolSize = 10)
	public void testSubCL10190(){
		Sequoiadb db  = null;
		//-----create cs/mainCL
		try{
			db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
			this.createSubCL(db);
		}catch(BaseException e){
			Assert.fail(e.getMessage());
		}
		
		//-----attach cl-----
		try
		{
			BSONObject options = new BasicBSONObject();
			BSONObject lowBoundObj = new BasicBSONObject();
			BSONObject upBoundObj  = new BasicBSONObject();
			lowBoundObj.put("a", 1);
			upBoundObj.put("a", 100);
			options.put("LowBound", lowBoundObj);
			options.put("UpBound", upBoundObj);
			db.getCollectionSpace(mCSName).getCollection(mCLName).
					attachCollection(sCSName + "." + sCLName, options);
		    
			//check results of catalog
			CommLib.checkCLResult(db, csName, clName);
		}catch(BaseException e){
			if(e.getErrorCode() != -235 //-235:Duplicated attach collection partition
					&& e.getErrorCode() != -23 
					&& e.getErrorCode() != -34){  
				
				db.disconnect();
				Assert.fail(e.getMessage());
			}
		}
		
		//-----drop subCS-----
		try{
			db.dropCollectionSpace(sCSName);
			//check results of catalog
			CommLib.checkCLResult(db, csName, clName);
		}catch(BaseException e){
			if(e.getErrorCode() != -34){  
				Assert.fail(e.getMessage());
			}
		}finally{
			db.disconnect();
		}
		
	}
	
	public void createMainCL(Sequoiadb sdb){
		try{
			sdb.createCollectionSpace(mCSName);
			
			BSONObject mOpt = new BasicBSONObject();
			BSONObject mSubObj = new BasicBSONObject();
			mSubObj.put("a", 1);
			mOpt.put("ShardingKey", mSubObj); 
			mOpt.put("ReplSize", 0);
			mOpt.put("IsMainCL", true);
			sdb.getCollectionSpace(mCSName).createCollection(mCLName, mOpt);
			
		}catch(BaseException e){
			if(e.getErrorCode() != -22 && e.getErrorCode() != -33){  
				sdb.disconnect();
				Assert.fail(e.getMessage());
			}
		}
		
	}
	
	public void createSubCL(Sequoiadb sdb){
		try{
			sdb.createCollectionSpace(sCSName);
			
			BSONObject sOpt = new BasicBSONObject();
			BSONObject sSubObj = new BasicBSONObject();
			sSubObj.put("a", 1);
			sOpt.put("ShardingKey", sSubObj);
			sOpt.put("ReplSize", 0);
			sdb.getCollectionSpace(sCSName).createCollection(sCLName, sOpt);
			
		}catch(BaseException e){
			if(e.getErrorCode() != -22 
					&& e.getErrorCode() != -33
					&& e.getErrorCode() != -34){  
				sdb.disconnect();
				Assert.fail(e.getMessage());
			}
		}
		
	}
	
}