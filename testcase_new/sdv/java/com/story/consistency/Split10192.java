package com.story.consistency;

import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Date;
import java.util.Random;

import org.testng.annotations.Test;
import org.testng.annotations.Parameters;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.AfterClass;
import org.bson.BSONObject;
import org.bson.BasicBSONObject;
import org.testng.Assert;
import org.testng.SkipException;

import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.exception.BaseException;
import com.story.consistency.CommLib;

/**
* TestLink: seqDB-10192
* @author xiaoni huang init
* @Date   2016.10.24
*/

public class Split10192 {
	private CommLib CommLib = new CommLib();
	private SimpleDateFormat dateFm = new SimpleDateFormat("YYYY-MM-dd HH:mm:ss");
	private static Sequoiadb sdb = null;
	private static ArrayList<String> groupNames = null;
	private String csName = "cs10192";
	private String clName = "cl10192";
	private String mCLName = clName + "_m";
	private String sCLName = clName + "_s";
	
	@BeforeClass
	@Parameters({"coordAddr"})
	public void setUp(String coordAddr){
		//start time
		System.out.println("Begin to run " + this.getClass().getName() 
					+ ", begin in: " + dateFm.format(new Date().getTime()));
		try{
			sdb = new Sequoiadb(coordAddr, "", "");
			//judge the mode and group number
			if(CommLib.isStandAlone(sdb) || CommLib.OneGroupMode(sdb)){
				throw new SkipException("The mode is standlone, or only one group, "
						+ "skip the testCase.");
			}
			//get groupNames
			groupNames = CommLib.getDataGroupNames(sdb);
			//clear env
			CommLib.clearCS(sdb, csName);
			//create cs
			sdb.createCollectionSpace(csName);
			//create cl
			Split10192 localFuncs = new Split10192();
			localFuncs.createMainCL(sdb);
			localFuncs.createSubCL(sdb);
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
	public void testSubCL10192(String coordAddr){
		Sequoiadb db  = null;
		
		//split
		try{
			db = new Sequoiadb(coordAddr, "", "");
			
			DBCollection clDB = db.getCollectionSpace(csName).getCollection(sCLName);
			BSONObject strCond = new BasicBSONObject();
			BSONObject endCond = new BasicBSONObject();
			Random i = new Random();
			int bound = i.nextInt(3996);
			strCond.put("a", bound);
			endCond.put("a", bound + 100);
			System.out.println("split condition: " + strCond + ", " +endCond );
			clDB.split(groupNames.get(0), groupNames.get(1), strCond, endCond);
			/*
			//check result
			Split10192 localFuncs = new Split10192();
			localFuncs.checkResult(db);
			*/
		}catch(BaseException e){
			if(e.getErrorCode() != -175){ //-175:The mutex task already exist
				Assert.fail(e.getMessage());
			}
		}
		
		//-----attachCL-----
		try
		{

			Split10192 localFuncs = new Split10192();
			localFuncs.attachCL(db);
		}catch(BaseException e){
			Assert.fail(e.getMessage());
		}
		
		//detach cl
		try{
			db.getCollectionSpace(csName).getCollection(mCLName).
			detachCollection(csName + "." + sCLName);
		}catch(BaseException e){
			Assert.fail(e.getMessage());
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
			sdb.getCollectionSpace(csName).createCollection(mCLName, opt);
		}catch(BaseException e){
			Assert.fail(e.getMessage());
		}
	}
	
	public void createSubCL(Sequoiadb sdb){
		try{
			BSONObject opt = new BasicBSONObject();
			BSONObject subObj = new BasicBSONObject();
			subObj.put("a", 1);
			opt.put("ShardingKey", subObj);
			opt.put("Group", groupNames.get(0));
			opt.put("ReplSize", 0);
			sdb.getCollectionSpace(csName).createCollection(sCLName, opt);
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
			lowBoundObj.put("a", 0);
			upBoundObj.put("a", 100);
			options.put("LowBound", lowBoundObj);
			options.put("UpBound", upBoundObj);
			sdb.getCollectionSpace(csName).getCollection(mCLName).
				attachCollection(csName + "." + sCLName, options);
		}catch(BaseException e){
			if(e.getErrorCode() != -235){  //-235:Duplicated attach collection partition
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