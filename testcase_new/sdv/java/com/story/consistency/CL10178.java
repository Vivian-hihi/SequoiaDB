package com.story.consistency;

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
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.testcommon.SdbTestBase;
import com.story.consistency.CommLib;

/**
* TestLink: 
* 		seqDB-10172: create autoSplit collection
* 		seqDB-10177: concurrency[alterCL]
* 		seqDB-10178: concurrency[alterCL, dropCL]
* @author xiaoni huang init
* @Date   2016.10.11
*/

public class CL10178 extends SdbTestBase{
	private CommLib CommLib = new CommLib();
	private static Sequoiadb sdb = null;
	private static ArrayList<String> dataGroups = null;
	private String domainName = "dm10178";
	private String csName = "cs10178";
	private String clName = "cl10178";
	private Random random = new Random();
	private int number = 20;
	private SimpleDateFormat dateFm = new SimpleDateFormat("YYYY-MM-dd HH:mm:ss");
	
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
			CL10178.this.createDomain();
			CL10178.this.createCS();
			CL10178.this.createCL();
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
			CommLib.clearDomain(sdb, domainName);
			
		}catch(BaseException e){
			Assert.fail("ErrorMsg:\n" +e.getMessage());
		}finally{
			System.out.println("End to run " + this.getClass().getName() 
						+ ", end in: " + dateFm.format(new Date().getTime()));
			sdb.disconnect();
		}
	}
	
	@Test(invocationCount = 20, threadPoolSize = 20)
	public void testAlterCL(){
		Sequoiadb db  = null;
		CollectionSpace csDB = null;
		try{
			db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
			csDB = db.getCollectionSpace(csName);
		}catch(BaseException e){
			Assert.fail(e.getMessage());
		}
		
		//-----alter collection-----
		try{
			BSONObject opt = new BasicBSONObject();
			int i = random.nextInt(dataGroups.size());
			opt.put("Group", dataGroups.get(i).toCharArray() );
			if(csDB.isCollectionExist(clName)){
				csDB.getCollection(clName + "_" + random.nextInt(number)).
						alterCollection(opt);
			}
			//check results 
		    CommLib.checkCLResult(db, csName, clName);
		}catch(BaseException e){
			if(e.getErrorCode() != -23){  //-23:Collection does not exist
				db.disconnect();
				Assert.fail(e.getMessage());
			}
		}finally{
			db.disconnect();
		}
	}
	
	@Test(invocationCount = 20, threadPoolSize = 20)
	public void testDropCL(){
		Sequoiadb db  = null;
		CollectionSpace csDB = null;
		try{
			db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
			csDB = db.getCollectionSpace(csName);
		}catch(BaseException e){
			Assert.fail(e.getMessage());
		}
		
		//-----drop collection-----
		try{
			csDB.dropCollection(clName + "_" + random.nextInt(number));
			
		}catch(BaseException e){
			if(e.getErrorCode() != -23){  //-23:Collection does not exist
				db.disconnect();
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