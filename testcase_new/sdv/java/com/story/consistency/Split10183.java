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

import com.sequoiadb.base.CollectionSpace;
import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.exception.BaseException;
import com.story.consistency.CommLib;

/**
* TestLink: seqDB-10183
* @author xiaoni huang init
* @Date   2016.10.24
*/

public class Split10183 {
	private CommLib CommLib = new CommLib();
	private SimpleDateFormat dateFm = new SimpleDateFormat("YYYY-MM-dd HH:mm:ss");
	private static Sequoiadb sdb = null;
	private static ArrayList<String> groupNames = null;
	private String csName = "cs10183";
	private String clName = "cl10183";
	
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
			//create cl
			sdb.createCollectionSpace(csName);
			Split10183 localFuncs = new Split10183();
			localFuncs.createCL(sdb, groupNames.get(0));
			
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
	
	@Test(invocationCount = 20, threadPoolSize = 20)
	@Parameters({"coordAddr"})
	public void testSplit10183(String coordAddr){
		Sequoiadb db  = null;
		
		//split
		try{
			db = new Sequoiadb(coordAddr, "", "");
			DBCollection clDB = db.getCollectionSpace(csName).getCollection(clName);
			BSONObject strCond = new BasicBSONObject();
			BSONObject endCond = new BasicBSONObject();
			Random i = new Random();
			int bound = i.nextInt(10000);
			strCond.put("a", bound);
			endCond.put("a", bound + 100);
			//System.out.println("split condition: " + strCond + ", " +endCond );
			clDB.split(groupNames.get(0), groupNames.get(1), strCond, endCond);
			//check result
			/*
			Split10183 localFuncs = new Split10183();
			localFuncs.checkResult(db);
			*/
		}catch(BaseException e){
			if(e.getErrorCode() != -175){ //-175:The mutex task already exist
				Assert.fail(e.getMessage());
			}
		}
		
		//-----drop cl-----
		try{
			db.getCollectionSpace(csName).dropCollection(clName);
		}catch(BaseException e){
			if(e.getErrorCode() != -23){  //-23:Collection does not exist
				Assert.fail(e.getMessage());
			}
		}
		
		//-----create cl-----
		try{
			db.getCollectionSpace(csName).createCollection(clName);
		}catch(BaseException e){
			if(e.getErrorCode() != -22){  //-22:Collection already exists
				Assert.fail(e.getMessage());
			}
		}finally{
			db.disconnect();
		}
		
	}
	
	private void createCL(Sequoiadb sdb, String rgName){
		try{
			CollectionSpace csDB = sdb.getCollectionSpace(csName);
			BSONObject opt = new BasicBSONObject();
			BSONObject subObj = new BasicBSONObject();
			subObj.put("a", 1);
			opt.put("ShardingType", "range");
			opt.put("ShardingKey", subObj);
			opt.put("Group", rgName);
			opt.put("ReplSize", 0);
			csDB.createCollection(clName, opt);
		}catch(BaseException e){
			Assert.fail(e.getMessage());
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