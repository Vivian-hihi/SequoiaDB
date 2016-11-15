package com.story.consistency;

import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.Random;

import org.testng.annotations.Test;
import org.testng.annotations.Parameters;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.AfterClass;
import org.bson.BSONObject;
import org.bson.BasicBSONObject;
import org.testng.Assert;

import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.exception.BaseException;
import com.story.consistency.CommLib;

/**
* TestLink: seqDB-10199
* @author xiaoni huang init
* @Date   2016.10.17
*/

public class SubCL10200 {
	private CommLib CommLib = new CommLib();
	private SimpleDateFormat dateFm = new SimpleDateFormat("YYYY-MM-dd HH:mm:ss");
	private static Sequoiadb sdb = null;
	private String csName = "cs10200";
	private String clName = "cl10200";
	private String mCSName = csName + "_mainCS";
	private String sCSName = csName + "_subCS";
	private String mCLName = clName + "_mainCL";
	private String sCLName = clName + "_subCL";
	
	@BeforeClass
	@Parameters({"coordAddr"})
	public void setUp(String coordAddr){
		//start time
		System.out.println("Begin to run " + this.getClass().getName() 
					+ ", begin in: " + dateFm.format(new Date().getTime()));
		try{
			sdb = new Sequoiadb(coordAddr, "", "");
			//clear env
			CommLib.clearCL(sdb, csName, clName);
			CommLib.clearCS(sdb, csName);
			//create cs
			sdb.createCollectionSpace(mCSName);
			sdb.createCollectionSpace(sCSName);
			//create subCL
			SubCL10200 SubCL10200 = new SubCL10200();
			SubCL10200.createMainCL(sdb);
			SubCL10200.createSubCL(sdb);
		}catch(BaseException e){
			Assert.fail("Failed to prepare env at th begining. "
					+ "ErrorMsg:\n" +e.getMessage());
		}
	}
	
	@AfterClass
	public void tearDown(){
		try{
			//clear env
			CommLib.clearCL(sdb, csName, clName);
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
	@Parameters({"coordAddr"})
	public void testSubCL10200(String coordAddr){
		Sequoiadb db  = null;
		SubCL10200 SubCL10200 = new SubCL10200();
		
		//-----attachCL-----
		try{
			db = new Sequoiadb(coordAddr, "", "");
			
			SubCL10200.attachCL(db);
			SubCL10200.checkResult(db);
		}catch(BaseException e){
			Assert.fail(e.getMessage());
		}
		
		//-----detachCL-----
		try{
			//random generate domain name
			Random randomInt = new Random();
			String tmpName = mCLName;
			tmpName = mCLName + "_" + randomInt.nextInt(30);
			//detachCL
			db.getCollectionSpace(mCSName).getCollection(tmpName).
					detachCollection(sCSName + "." + sCLName);
			SubCL10200.checkResult(db);
		}catch(BaseException e){
			if(e.getErrorCode() != -6){  //Duplicated attach
				Assert.fail(e.getMessage());
			}
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
			for(int i = 0; i < 30; i++){
				sdb.getCollectionSpace(mCSName).createCollection(mCLName + "_" + i, mOpt);
			}
		}catch(BaseException e){
			Assert.fail(e.getMessage());
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
			//random generate domain name
			Random randomInt = new Random();
			String tmpName = mCLName;
			tmpName = mCLName + "_" + randomInt.nextInt(30);
			//attachCL
			sdb.getCollectionSpace(mCSName).getCollection(tmpName).
					attachCollection(sCSName + "." + sCLName, options);
		}catch(BaseException e){
			int errCode = e.getErrorCode();
			if(errCode != -235){  //-235:Duplicated attach collection partition
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