package com.sequoiadb.consistencyData;

import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.Random;

import org.testng.annotations.Test;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.AfterClass;
import org.bson.BSONObject;
import org.bson.BasicBSONObject;
import org.testng.Assert;
import org.testng.SkipException;

import com.sequoiadb.base.CollectionSpace;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.consistencyData.CommLib;
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.testcommon.SdbTestBase;

/**
* TestLink: seqDB-10188: concurrency[attachCL, drop subCL]
* @author xiaoni huang init
* @Date   2016.10.11
*/

public class SubCL10188 extends SdbTestBase {
	private SimpleDateFormat dateFm = new SimpleDateFormat("YYYY-MM-dd HH:mm:ss");
	private CommLib CommLib = new CommLib();
	private static Sequoiadb sdb = null;
	private String csName = "cs10188";
	private String clName = "cl10188";
	private String mCLName = clName + "_m";
	private String sCLName = clName + "_s";
	private Random random = new Random();
	private int number = 20;
	
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
			this.readyCL();
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
	public void testAttachCL(){
		Sequoiadb db  = null;
		CollectionSpace csDB = null;
		try{
			db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
			csDB = db.getCollectionSpace(csName);
		}catch(BaseException e){
			Assert.fail(e.getMessage());
		}
		
		try
		{
			db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
			BSONObject options = new BasicBSONObject();
			BSONObject lowBoundObj = new BasicBSONObject();
			BSONObject upBoundObj  = new BasicBSONObject();
			int k = random.nextInt(10000);
			lowBoundObj.put("a", 0 + k);
			upBoundObj.put("a", 100 + k);
			options.put("LowBound", lowBoundObj);
			options.put("UpBound", upBoundObj);
			String tmpMR = mCLName + random.nextInt(number);
			String tmpSR = sCLName + random.nextInt(number);
			if(csDB.isCollectionExist(tmpSR)){
				csDB.getCollection(tmpMR).attachCollection(csName + "." + tmpSR, options);
			}
			//check results of catalog
			CommLib.checkCLResult(db, csName, clName);
		}catch(BaseException e){
			if(e.getErrorCode() != -235 //-235:Duplicated attach collection partition
					&& e.getErrorCode() != -237  //-237:New boundary is conflict with the existing boundary
					&& e.getErrorCode() != -23){ 
				Assert.fail(e.getMessage());
			}
		}finally{
			db.disconnect();
		}
		
	}
	
	@Test(invocationCount = 10, threadPoolSize = 10)
	public void testSubCL10188(){
		Sequoiadb db  = null;
		CollectionSpace csDB = null;
		try{
			db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
			csDB = db.getCollectionSpace(csName);
		}catch(BaseException e){
			Assert.fail(e.getMessage());
		}
		
		try{
			csDB.dropCollection(sCLName + random.nextInt(number));
			//check results of catalog
			CommLib.checkCLResult(db, csName, clName);
		}catch(BaseException e){
			if(e.getErrorCode() != -23){  //-23:Collection does not exist
				db.disconnect();
				Assert.fail(e.getMessage());
			}
		}finally{
			db.disconnect();
		}
		
	}
	
	public void readyCL(){
		CollectionSpace csDB = null;
		
		//-----create cs-----
		try{
			sdb.createCollectionSpace(csName);
			csDB = sdb.getCollectionSpace(csName);
		}catch(BaseException e){
			Assert.fail(e.getMessage());
		}
		
		//-----create mainCL-----
		try{
			BSONObject mOpt = new BasicBSONObject();
			BSONObject mSubObj = new BasicBSONObject();
			mSubObj.put("a", 1);
			mOpt.put("ShardingKey", mSubObj); 
			mOpt.put("ReplSize", 0);
			mOpt.put("IsMainCL", true);
			for(int i = 0; i < number; i++){
				csDB.createCollection( mCLName + i, mOpt );
			}
		}catch(BaseException e){
			Assert.fail(e.getMessage());
		}
		
		//-----create subCL-----
		try{
			BSONObject sOpt = new BasicBSONObject();
			BSONObject sSubObj = new BasicBSONObject();
			sSubObj.put("a", 1);
			sOpt.put("ShardingKey", sSubObj);
			sOpt.put("ReplSize", 0);
			for(int i = 0; i < number; i++){
				csDB.createCollection( sCLName + i, sOpt );
			}
		}catch(BaseException e){
			Assert.fail(e.getMessage());
		}
		
	}
		
}