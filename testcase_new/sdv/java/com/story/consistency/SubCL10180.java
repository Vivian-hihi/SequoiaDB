package com.story.consistency;

import java.text.SimpleDateFormat;
import java.util.Date;

import org.testng.annotations.Test;
import org.testng.annotations.Parameters;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.AfterClass;
import org.bson.BSONObject;
import org.bson.BasicBSONObject;
import org.testng.Assert;

import com.sequoiadb.base.CollectionSpace;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.exception.BaseException;
import com.story.consistency.CommLib;

/**
* TestLink: seqDB-10186
* @author xiaoni huang init
* @Date   2016.10.11
*/

public class SubCL10180 {
	private CommLib CommLib = new CommLib();
	private SimpleDateFormat dateFm = new SimpleDateFormat("YYYY-MM-dd HH:mm:ss");
	private static Sequoiadb sdb = null;
	private String csName = "cs10180";
	private String clName = "cl10180";
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
			CommLib.clearCS(sdb, csName);
			//create cs/cl
			SubCL10180 SubCL10180 = new SubCL10180();
			SubCL10180.createCL(sdb);
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
	@Parameters({"coordAddr"})
	public void testSubCL10186(String coordAddr){
		Sequoiadb db  = null;
		//-----create mainCL-----
		try{
			db = new Sequoiadb(coordAddr, "", "");
			
			BSONObject mOpt = new BasicBSONObject();
			BSONObject mSubObj = new BasicBSONObject();
			mSubObj.put("a", 1);
			mOpt.put("ShardingKey", mSubObj); 
			mOpt.put("ReplSize", 0);
			mOpt.put("IsMainCL", true);
			db.getCollectionSpace(mCSName).createCollection(mCLName, mOpt);
		}catch(BaseException e){
			if(e.getErrorCode() != -22){ //-22:Collection already exists
				db.disconnect();
				Assert.fail(e.getMessage());
			}
		}
		
		//-----attachCL-----
		try
		{
			db = new Sequoiadb(coordAddr, "", "");
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
			CommLib.checkCLOfCatalog(db, csName, clName);
			CommLib.checkCLOfDataRG(db, csName, clName);
			boolean rc = CommLib.compareDataAndCata(db, csName, clName);
			Assert.assertTrue(rc);
		}catch(BaseException e){
			if(e.getErrorCode() != -235){  //-235:Duplicated attach collection partition
				db.disconnect();
				Assert.fail(e.getMessage());
			}
		}
		
		//-----alter cl-----
		try{
			//random generate cs name
		    CollectionSpace csDB = db.getCollectionSpace(sCSName);
			if(csDB.isCollectionExist(sCLName)){
				BSONObject options = new BasicBSONObject();
				options.put("ReplSize", 7);
			    csDB.getCollection(sCLName).alterCollection(options);
			}
			//check results of catalog
			CommLib.checkCLOfCatalog(db, csName, clName);
			CommLib.checkCLOfDataRG(db, csName, clName);
			boolean rc = CommLib.compareDataAndCata(db, csName, clName);
			Assert.assertTrue(rc);
		}catch(BaseException e){
			if(e.getErrorCode() != -23){  
				//-23:Collection does not exist
				db.disconnect();
				Assert.fail(e.getMessage());
			}
		}
		
		//-----drop mainCL-----
		try{
			db.getCollectionSpace(mCSName).dropCollection(mCLName);
			
			//check results of catalog
			CommLib.checkCLOfCatalog(db, csName, clName);
			CommLib.checkCLOfDataRG(db, csName, clName);
			boolean rc = CommLib.compareDataAndCata(db, csName, clName);
			Assert.assertTrue(rc);
		}catch(BaseException e){
			if(e.getErrorCode() != -23){ //-23:Collection does not exist
				db.disconnect();
				Assert.fail(e.getMessage());
			}
		}finally{
			db.disconnect();
		}
	}
	
	/**
	 * create cs/cl
	 * @param sdb
	 */
	public void createCL(Sequoiadb sdb){
		try{
			//create cs
			sdb.createCollectionSpace(mCSName);
			sdb.createCollectionSpace(sCSName);
			
			//create subCL
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
	
}