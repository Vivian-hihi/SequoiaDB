package com.sequoiadb.consistencyData;

import java.text.SimpleDateFormat;
import java.util.ArrayList;
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
import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.consistencyData.CommLib;
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.testcommon.SdbTestBase;

/**
* TestLink: seqDB-10183: concurrency[split, dropCL, createCL]
* @author xiaoni huang init
* @Date   2016.10.24
*/

public class Split10183 extends SdbTestBase {
	private SimpleDateFormat dateFm = new SimpleDateFormat("YYYY-MM-dd HH:mm:ss");
	private CommLib CommLib = new CommLib();
	private static Sequoiadb sdb = null;
	private static ArrayList<String> groupNames = null;
	private String csName = "cs10183";
	private String clName = "cl10183";
	
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
			//get groupNames
			groupNames = CommLib.getDataGroupNames(sdb);
			//clear env
			CommLib.clearCS(sdb, csName);
			//ready env
			sdb.createCollectionSpace(csName);
			createCL(sdb, groupNames.get(0));
		}catch(BaseException e){
			Assert.fail("Failed to prepare env at th begining. "
					+ "ErrorMsg:\n" +e.getMessage());
		}
	}
	
	@AfterClass
	public void tearDown(){
		try{
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
	public void testSplit(){
		Sequoiadb db  = null;
		try{
			db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
			
			if(db.getCollectionSpace(csName).isCollectionExist(clName)){
				DBCollection clDB = db.getCollectionSpace(csName).getCollection(clName);
				BSONObject strCond = new BasicBSONObject();
				BSONObject endCond = new BasicBSONObject();
				Random i = new Random();
				int bound = i.nextInt(3996);
				strCond.put("a", bound);
				endCond.put("a", bound + 100);
				clDB.split(groupNames.get(0), groupNames.get(1), strCond, endCond);
				//check result
				CommLib.checkCLResult(db, csName, clName);
			}
		}catch(BaseException e){
			if(e.getErrorCode() != -175 //-175:The mutex task already exist
					&& e.getErrorCode() != -147  //-147:Unable to lock
					&& e.getErrorCode() != -23 ){  
				Assert.fail(e.getMessage());
			}
		}finally{
			db.disconnect();
		}
		
	}
	
	@Test(invocationCount = 10, threadPoolSize = 10)
	public void testDropCL(){
		Sequoiadb db  = null;
		try{
			db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
		}catch(BaseException e){
			Assert.fail(e.getMessage());
		}
		
		//drop cl
		try{
			db.getCollectionSpace(csName).dropCollection(clName);
		}catch(BaseException e){
			if(e.getErrorCode() != -147  //-147:Unable to lock
					&& e.getErrorCode() != -23){  
				db.disconnect();
				Assert.fail(e.getMessage());
			}
		}
		
		//create cl
		try{
			createCL(db, groupNames.get(0));
		}catch(BaseException e){
			Assert.fail(e.getMessage());
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
			if(e.getErrorCode() != -22){  
				Assert.fail(e.getMessage());
			}
		}
	}
		
}