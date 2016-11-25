package com.story.consistencyData;

import java.text.SimpleDateFormat;
import java.util.Date;

import org.testng.annotations.Test;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.AfterClass;
import org.bson.BSONObject;
import org.bson.BasicBSONObject;
import org.testng.Assert;

import com.sequoiadb.base.CollectionSpace;
import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.testcommon.SdbTestBase;
import com.story.consistencyData.CommLib;

/**
* TestLink: seqDB-10195: concurrency[detachCL, drop mainCS]
* @author xiaoni huang init
* @Date   2016.10.17
*/

public class SubCL10195 extends SdbTestBase {
	private CommLib CommLib = new CommLib();
	private SimpleDateFormat dateFm = new SimpleDateFormat("YYYY-MM-dd HH:mm:ss");
	private static Sequoiadb sdb = null;
	private String csName = "cs10195";
	private String clName = "cl10195";
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
			createMainCL(sdb);
			createSubCL(sdb);
			attachCL(sdb);
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
	
	@Test(invocationCount = 100, threadPoolSize = 100)
	public void testSubCL10195(){
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
			CommLib.checkCLResult(db, csName, clName);
		}catch(BaseException e){
			if(e.getErrorCode() != -23 && e.getErrorCode() != -34){  
				db.disconnect();
				Assert.fail(e.getMessage());
			}
		}
		
		//-----drop mainCS-----
		try{
			db.dropCollectionSpace(mCSName);
			CommLib.checkCLResult(db, csName, clName);
		}catch(BaseException e){
			if(e.getErrorCode() != -34){  
				db.disconnect();
				Assert.fail(e.getMessage());
			}
		}

		//-----create mainCS-----
		try{
			db.createCollectionSpace(mCSName);
		}catch(BaseException e){
			if(e.getErrorCode() != -33){
				db.disconnect();
				Assert.fail(e.getMessage());
			}
		}
		
		//-----create mainCL-----
		try{
			createMainCL(db);
		}catch(BaseException e){
			Assert.fail(e.getMessage());
		}

		//-----attachCL-----
		try{
			attachCL(db);
			CommLib.checkCLResult(db, csName, clName);
		}catch(BaseException e){
			if(e.getErrorCode() != -23){  
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
			if(e.getErrorCode() != -22 && e.getErrorCode() != -34){
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
			if(e.getErrorCode() != -22)
				Assert.fail(e.getMessage());
		}
		
	}
	
	public void attachCL(Sequoiadb sdb){
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
			if(csDB != null){
				DBCollection clDB = csDB.getCollection(mCLName);
				if(clDB != null)
					clDB.attachCollection(sCSName + "." + sCLName, options);
			}
		}catch(BaseException e){
			if(e.getErrorCode() != -235  //-235:Duplicated attach collection partition
					&& e.getErrorCode() != -23)
				Assert.fail(e.getMessage());
		}
	}
}