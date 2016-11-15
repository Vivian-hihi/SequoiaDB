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

import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.exception.BaseException;
import com.story.consistency.CommLib;

/**
* TestLink: seqDB-10167
* @author xiaoni huang init
* @Date   2016.10.11
*/

public class CS10167 {
	private CommLib CommLib = new CommLib();
	private SimpleDateFormat dateFm = new SimpleDateFormat("YYYY-MM-dd HH:mm:ss");
	private static Sequoiadb sdb = null;
	private String csName = "cs10167";
	private String clName = "cl10167";
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
			//create subCS
			sdb.createCollectionSpace(sCSName);
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
	public void testCS10167(String coordAddr){
		Sequoiadb db  = null;
		//-----create subCL-----
		try{
			db = new Sequoiadb(coordAddr, "", "");

			//create cs
			db.createCollectionSpace(mCSName);
			//create mainCL
			BSONObject mOpt = new BasicBSONObject();
			BSONObject mSubObj = new BasicBSONObject();
			mSubObj.put("a", 1);
			mOpt.put("ShardingKey", mSubObj); 
			mOpt.put("ReplSize", 0);
			mOpt.put("IsMainCL", true);
			db.getCollectionSpace(mCSName).createCollection(mCLName, mOpt);
			
			//create subCL
			BSONObject sOpt = new BasicBSONObject();
			BSONObject sSubObj = new BasicBSONObject();
			sSubObj.put("a", 1);
			sOpt.put("ShardingKey", sSubObj);
			sOpt.put("ReplSize", 0);
			db.getCollectionSpace(sCSName).createCollection(sCLName, sOpt);
			
		}catch(BaseException e){
			if(e.getErrorCode() != -22 && e.getErrorCode() != -33){   
				//-22:Collection already exists
				//-33:Collection space already exist
				db.disconnect();
				Assert.fail(e.getMessage());
			}
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
			CommLib.checkCLOfCatalog(db, csName, clName);
			CommLib.checkCLOfDataRG(db, csName, clName);
			boolean rc = CommLib.compareDataAndCata(db, csName, clName);
			Assert.assertTrue(rc);
		}catch(BaseException e){
			int errCode = e.getErrorCode();
			if(errCode != -235 && errCode != -23 && errCode != -34){  
				//-235:Duplicated attach collection partition
				//-23:Collection does not exist
				//-34:Collection space does not exist
				db.disconnect();
				Assert.fail(e.getMessage());
			}
		}
		
		//-----drop cs[mainCS]-----
		try{
			db.dropCollectionSpace(mCSName);
			
			//check results of catalog
			CommLib.checkCLOfCatalog(db, csName, clName);
			CommLib.checkCLOfDataRG(db, csName, clName);
			boolean rc = CommLib.compareDataAndCata(db, csName, clName);
			Assert.assertTrue(rc);
		}catch(BaseException e){
			if(e.getErrorCode() != -34){   //-34:Collection space does not exist
				db.disconnect();
				Assert.fail(e.getMessage());
			}
		}
		
		//-----drop subCL-----
		try{
			db.getCollectionSpace(sCSName).dropCollection(sCLName);
			
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
	
}