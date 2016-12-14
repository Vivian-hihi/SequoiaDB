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

import com.sequoiadb.base.CollectionSpace;
import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.metadataConsistency.data.CommLib;
import com.sequoiadb.testcommon.SdbTestBase;

/**
* TestLink: 
* 		seqDB-10168: concurrency[drop cs of subCL, drop mainCL]
* @author xiaoni huang init
* @Date   2016.10.11
*/

public class CS10168 extends SdbTestBase {
	private SimpleDateFormat dateFm = new SimpleDateFormat("YYYY-MM-dd HH:mm:ss");
	private CommLib CommLib = new CommLib();
	private static Sequoiadb sdb = null;
	private String csName = "cs10168";
	private String clName = "cl10168";
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
			//judge the mode and group number
			if(CommLib.isStandAlone(sdb) || CommLib.OneGroupMode(sdb)){
				throw new SkipException("The mode is standlone, or only one group, "
						+ "skip the testCase.");
			}
			//clear env
			CommLib.clearCS(sdb, csName);
			//ready env
			sdb.createCollectionSpace(mCSName);
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
	public void testDropSubCS(){
		Sequoiadb db  = null;
		try{
			db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
		}catch(BaseException e){
			Assert.fail(e.getMessage());
		}
		this.createMainCL(db);
		this.createSubCL(db);
		this.attachCL(db);
		
		//-----drop subCS-----
		try{
			db.dropCollectionSpace(sCSName);
			//check results of catalog
			CommLib.checkCLResult(db, csName, clName);
		}catch(BaseException e){
			if(e.getErrorCode() != -34){  //-34:Collection space does not exist
				Assert.fail(e.getMessage());
			}
		}finally{
			db.disconnect();
		}
		
	}
	
	@Test(invocationCount = 10, threadPoolSize = 10)
	public void testDropMainCL(){
		Sequoiadb db  = null;
		try{
			db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
		}catch(BaseException e){
			Assert.fail(e.getMessage());
		}
		
		//-----drop subCL-----
		try{
			db.getCollectionSpace(mCSName).dropCollection(mCLName);
			//check results of catalog
			CommLib.checkCLResult(db, csName, clName);
		}catch(BaseException e){
			if(e.getErrorCode() != -23){ //-23:Collection does not exist
				Assert.fail(e.getMessage());
			}
		}finally{
			db.disconnect();
		}
	}
		
	public void createMainCL(Sequoiadb sdb){
		try{
			BSONObject opt = new BasicBSONObject();
			BSONObject subObj = new BasicBSONObject();
			subObj.put("a", 1);
			opt.put("ShardingKey", subObj); 
			opt.put("ReplSize", 0);
			opt.put("IsMainCL", true);
			sdb.getCollectionSpace(mCSName).createCollection(mCLName, opt);
		}catch(BaseException e){
			if(e.getErrorCode() != -22){ //-22:Collection already exists
				Assert.fail(e.getMessage());
			}
		}
	}
	
	public void createSubCL(Sequoiadb sdb){
		try{
			sdb.createCollectionSpace(sCSName);
			
			BSONObject opt = new BasicBSONObject();
			BSONObject subObj = new BasicBSONObject();
			subObj.put("a", 1);
			opt.put("ShardingKey", subObj);
			opt.put("ReplSize", 0);
			sdb.getCollectionSpace(sCSName).createCollection(sCLName, opt);
		}catch(BaseException e){
			if(e.getErrorCode() != -33
					&& e.getErrorCode() != -34
					&& e.getErrorCode() != -22){ 
				Assert.fail(e.getMessage());
			}
		}
	}
	
	public void attachCL(Sequoiadb sdb){
		try{
			BSONObject opt = new BasicBSONObject();
			BSONObject lowBound = new BasicBSONObject();
			BSONObject upBound  = new BasicBSONObject();
			lowBound.put("a", 1);
			upBound.put("a", 100);
			opt.put("LowBound", lowBound);
			opt.put("UpBound", upBound);
			CollectionSpace csDB = sdb.getCollectionSpace(mCSName);
			if(csDB.isCollectionExist(clName)){
				DBCollection clDB = csDB.getCollection(mCLName);
				clDB.attachCollection(sCSName + "." + sCLName, opt);
			}
		}catch(BaseException e){
			if(e.getErrorCode() != -34 
					&& e.getErrorCode() != -235){ //Duplicated attach collection partition
				Assert.fail(e.getMessage());
			}
		}
	}

}