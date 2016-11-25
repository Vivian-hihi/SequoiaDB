package com.sequoiadb.consistencyData;

import java.text.SimpleDateFormat;
import java.util.Date;

import org.testng.annotations.Test;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.AfterClass;
import org.bson.BSONObject;
import org.bson.BasicBSONObject;
import org.testng.Assert;

import com.sequoiadb.base.CollectionSpace;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.consistencyData.CommLib;
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.testcommon.SdbTestBase;

/**
* TestLink: seqDB-10196: concurrency[detachCL, drop subCS]
* @author xiaoni huang init
* @Date   2016.10.17
*/

public class SubCL10196 extends SdbTestBase {
	private CommLib CommLib = new CommLib();
	private SimpleDateFormat dateFm = new SimpleDateFormat("YYYY-MM-dd HH:mm:ss");
	private static Sequoiadb sdb = null;
	private String csName = "cs10196";
	private String clName = "cl10196";
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
			//clear env
			CommLib.clearCS(sdb, csName);
			//create cs
			sdb.createCollectionSpace(mCSName);
			sdb.createCollectionSpace(sCSName);
			//create subCL
			SubCL10196.this.createMainCL(sdb);
			SubCL10196.this.createSubCL(sdb);
			SubCL10196.this.attachCL(sdb);
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
	public void testSubCL10196(){
		Sequoiadb db = null;
		try{
			db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
		}catch(BaseException e){
			Assert.fail(e.getMessage());
		}
		
		//-----detachCL-----
		try{
			CollectionSpace csDB = db.getCollectionSpace(mCSName);
			if(csDB.isCollectionExist(mCLName)){
				csDB.getCollection(mCLName).detachCollection(sCSName + "." + sCLName);
			}
			
			SubCL10196.this.checkResult(db);
		}catch(BaseException e){
			if(e.getErrorCode() != -23 && e.getErrorCode() != -34){  //-23:Collection does not exist
				db.disconnect();
				Assert.fail(e.getMessage());
			}
		}
		
		//-----drop subCS-----
		try{
			db.dropCollectionSpace(mCSName);

			SubCL10196.this.checkResult(db);
		}catch(BaseException e){
			if(e.getErrorCode() != -34){  //-34:Collection space does not exist
				db.disconnect();
				Assert.fail(e.getMessage());
			}
		}

		//-----create subCS-----
		try{
			db.createCollectionSpace(sCSName);
		}catch(BaseException e){
			if(e.getErrorCode() != -33){  //-33:Collection space already exist
				db.disconnect();
				Assert.fail(e.getMessage());
			}
		}
		
		//-----create subCL-----
		try{
			SubCL10196.this.createSubCL(db);
		}catch(BaseException e){
			Assert.fail(e.getMessage());
		}

		//-----attachCL-----
		try{
			SubCL10196.this.attachCL(db);
		}catch(BaseException e){
			if(e.getErrorCode() != -23){  //-23:Collection does not exist
				db.disconnect();
				Assert.fail(e.getMessage());
			}
		}finally{
			db.disconnect();
		}
		
	}
	
	public void createMainCL(Sequoiadb sdb){
		try{
			BSONObject mOpt = new BasicBSONObject();
			BSONObject mSubObj = new BasicBSONObject();
			mSubObj.put("a", 1);
			mOpt.put("ShardingKey", mSubObj); 
			mOpt.put("ReplSize", 0);
			mOpt.put("IsMainCL", true);
			sdb.getCollectionSpace(mCSName).createCollection(mCLName, mOpt);
		}catch(BaseException e){
			if(e.getErrorCode() != -22){  //-22:Collection already exists
				sdb.disconnect();
				Assert.fail(e.getMessage());
			}
		}
	}
	
	public void createSubCL(Sequoiadb sdb){
		try{
			BSONObject sOpt = new BasicBSONObject();
			BSONObject sSubObj = new BasicBSONObject();
			sSubObj.put("a", 1);
			sOpt.put("ShardingKey", sSubObj);
			sOpt.put("ReplSize", 0);
			sdb.getCollectionSpace(sCSName).createCollection(sCLName, sOpt);
		}catch(BaseException e){
			if(e.getErrorCode() != -22){  //-22:Collection already exists
				sdb.disconnect();
				Assert.fail(e.getMessage());
			}
		}
		
	}
	
	public void attachCL(Sequoiadb sdb){
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
			sdb.getCollectionSpace(mCSName).getCollection(mCLName).
					attachCollection(sCSName + "." + sCLName, options);
		    
			//check results of catalog
			CommLib.checkCLOfCatalog(sdb, csName, clName);
			CommLib.checkCLOfDataRG(sdb, csName, clName);
			boolean rc = CommLib.compareDataAndCata(sdb, csName, clName);
			Assert.assertTrue(rc);
		}catch(BaseException e){
			int errCode = e.getErrorCode();
			if(errCode != -235 && errCode != -23 && errCode != -34){  
				//-235:Duplicated attach collection partition
				//-23:Collection does not exist
				//-34:Collection space does not exist
				sdb.disconnect();
				Assert.fail(e.getMessage());
			}
		}
	}
		
		public void checkResult(Sequoiadb sdb){
			try{
				CommLib.checkCLOfCatalog(sdb, csName, clName);
				CommLib.checkCLOfDataRG(sdb, csName, clName);
				boolean rc = CommLib.compareDataAndCata(sdb, csName, clName);
				Assert.assertTrue(rc);
			}catch(BaseException e){
				sdb.disconnect();
				Assert.fail(e.getMessage());
			}
		}
}