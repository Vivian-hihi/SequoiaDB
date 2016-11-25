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
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.consistencyData.CommLib;
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.testcommon.SdbTestBase;

/**
* TestLink: 
* 		seqDB-10172: create autoSplit collection
* 		seqDB-10177: concurrency[alterCL]
* 		seqDB-10178: concurrency[alterCL, dropCL]
* @author xiaoni huang init
* @Date   2016.10.11
*/

public class CL10178 extends SdbTestBase{
	private SimpleDateFormat dateFm = new SimpleDateFormat("YYYY-MM-dd HH:mm:ss");
	private CommLib CommLib = new CommLib();
	private static Sequoiadb sdb = null;
	private static ArrayList<String> dataGroups = null;
	private String domainName = "dm10178";
	private String csName = "cs10178";
	private String clName = "cl10178";
	private Random random = new Random();
	private int number = 20;
	
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
			CommLib.clearDomain(sdb, domainName);
			//ready env
			dataGroups = CommLib.getDataGroupNames(sdb);
			createDomain();
			createCS();
			createCL();
		}catch(BaseException e){
			Assert.fail("Failed to prepare env at th begining. "
					+ "ErrorMsg:\n" +e.getMessage());
		}
	}
	
	@AfterClass
	public void tearDown(){
		try{
			CommLib.clearCS(sdb, csName);
			CommLib.clearDomain(sdb, domainName);
		}catch(BaseException e){
			Assert.fail("ErrorMsg:\n" +e.getMessage());
		}finally{
			System.out.println("End to run " + this.getClass().getName() 
						+ ", end in: " + dateFm.format(new Date().getTime()));
			sdb.disconnect();
		}
	}
	
	@Test(invocationCount = 10, threadPoolSize = 10)
	public void testAlterCL(){
		Sequoiadb db  = null;
		try{
			db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
			
			BSONObject opt = new BasicBSONObject();
			int rgSize = random.nextInt(dataGroups.size());
			opt.put("Group", dataGroups.get(rgSize) );
			String tmpCLName = clName + "_" + random.nextInt(number);
			CollectionSpace csDB = db.getCollectionSpace(csName);
			if(csDB.isCollectionExist(tmpCLName)){
				csDB.getCollection(tmpCLName).
						alterCollection(opt);
			}
			//check results 
		    CommLib.checkCLResult(db, csName, clName);
		}catch(BaseException e){
			if(e.getErrorCode() != -23){ 
				Assert.fail(e.getMessage());
			}
		}finally{
			db.disconnect();
		}
	}
	
	@Test(invocationCount = 10, threadPoolSize = 10)
	public void testDropCL(){
		Sequoiadb db  = null;
		try
		{
			db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
			
			CollectionSpace csDB = db.getCollectionSpace(csName);
			csDB.dropCollection(clName + "_" + random.nextInt(number));
		}catch(BaseException e){
			if(e.getErrorCode() != -23){  
				Assert.fail(e.getMessage());
			}
			//check results 
		    CommLib.checkCLResult(db, csName, clName);
		}finally{
			db.disconnect();
		}
	}
	
	public void createDomain(){
		try{
			BSONObject opt = new BasicBSONObject();
			opt.put( "Groups", dataGroups );
			opt.put( "AutoSplit", true );
			sdb.createDomain(domainName, opt);
		}catch(BaseException e){
			Assert.fail(e.getMessage());
		}
	}
	
	public void createCS(){
		try{
			BSONObject opt = new BasicBSONObject();
			opt.put( "Domain", domainName );
			sdb.createCollectionSpace(csName, opt);
		}catch(BaseException e){
			Assert.fail(e.getMessage());
		}
	}
	
	public void createCL(){
		try
		{
			CollectionSpace csDB = sdb.getCollectionSpace(csName);
			BSONObject opt = new BasicBSONObject();
			BSONObject subObj = new BasicBSONObject();
			subObj.put("a", 1);
			opt.put("ShardingType", "hash");
			opt.put("ShardingKey", subObj);
			opt.put("ReplSize", 0);
			opt.put("AutoSplit", true);
			for(int i = 0; i < number; i++){
			    csDB.createCollection(clName + "_" + i, opt);
			}
		}catch(BaseException e){
			Assert.fail(e.getMessage());
		}
		
	}
		
}