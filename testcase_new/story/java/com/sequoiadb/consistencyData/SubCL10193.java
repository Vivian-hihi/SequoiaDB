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
* TestLink: seqDB-10193: concurrency[detachCL, drop mainCL]
* @author xiaoni huang init
* @Date   2016.10.11
*/

public class SubCL10193 extends SdbTestBase {
	private CommLib CommLib = new CommLib();
	private SimpleDateFormat dateFm = new SimpleDateFormat("YYYY-MM-dd HH:mm:ss");
	private static Sequoiadb sdb = null;
	private String csName = "cs10193";
	private String clName = "cl10193";
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
			SubCL10193.this.createMainCL(sdb);
			SubCL10193.this.createSubCL(sdb);
			SubCL10193.this.attachCL(sdb);
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
	public void testSubCL10193(){
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
			
			SubCL10193.this.checkResult(db);
		}catch(BaseException e){
			if(e.getErrorCode() != -23){  //-23:Collection does not exist
				db.disconnect();
				Assert.fail(e.getMessage());
			}
		}
		
		//-----drop mainCL-----
		try{
			db.getCollectionSpace(mCSName).dropCollection(mCLName);

			SubCL10193.this.checkResult(db);
		}catch(BaseException e){
			if(e.getErrorCode() != -23){  
				db.disconnect();
				Assert.fail(e.getMessage());
			}
		}
		
		//-----create mainCL-----
		try{
			SubCL10193.this.createMainCL(db);
		}catch(BaseException e){
			Assert.fail(e.getMessage());
		}

		//-----attachCL-----
		try{
			SubCL10193.this.attachCL(db);
			SubCL10193.this.checkResult(db);
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
			Assert.fail(e.getMessage());
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
			CollectionSpace csDB = sdb.getCollectionSpace(mCSName);
			if(csDB.isCollectionExist(mCLName)){
				csDB.getCollection(mCLName).attachCollection(sCSName + "." + sCLName, options);
			}
		}catch(BaseException e){
			if(e.getErrorCode() != -235 
					&& e.getErrorCode() != -23){  //-235:Duplicated attach collection partition
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