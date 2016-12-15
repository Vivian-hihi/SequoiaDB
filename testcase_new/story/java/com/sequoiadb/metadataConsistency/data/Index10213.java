package com.sequoiadb.metadataConsistency.data;

import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.Random;

import org.testng.annotations.Test;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.AfterClass;
import org.bson.BSONObject;
import org.bson.BasicBSONObject;
import org.testng.Assert;
import org.testng.SkipException;

import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.metadataConsistency.data.CommLib;
import com.sequoiadb.testcommon.SdbTestBase;

/**
* TestLink: seqDB-10213: concurrency[createIndex in mainCL]
* @author xiaoni huang init
* @Date   2016.10.20
*/

public class Index10213 extends SdbTestBase {
	private CommLib CommLib = new CommLib();
	private SimpleDateFormat dateFm = new SimpleDateFormat("YYYY-MM-dd HH:mm:ss");
	private static Sequoiadb sdb = null;
	private String csName = "cs10213";
	private String clName = "cs10213";
	private String mCLName = clName + "_m";
	private String sCLName = clName + "_s";
	private String idxName = "idx";
	
	@BeforeClass
	public void setUp(){
		//start time
		System.out.println("Begin to run " + this.getClass().getName() 
					+ ", begin in: " + dateFm.format(new Date().getTime()));
		try{
			sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "");
			//judge the mode
			if(CommLib.isStandAlone(sdb)){
				throw new SkipException("The mode is standlone, " + "skip the testCase.");
			}
			
			//clear env
			CommLib.clearCS(sdb, csName);
			//create cs
			sdb.createCollectionSpace(csName);
			//create subCL
			this.createMainCL(sdb);
			this.createSubCL(sdb);
			this.attachCL(sdb);
			
		}catch(BaseException e){
			Assert.fail("Failed to prepare env at th begining. "
					+ "ErrorMsg:\n" +e.getMessage());
		}
	}
	
	@AfterClass
	public void tearDown(){
		try{
			//check results
			CommLib.checkIndex(sdb, csName, clName);
			
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
	public void testIndex10213(){
		Sequoiadb db  = null;
		DBCollection clDB = null;
		try{
			db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
			clDB = db.getCollectionSpace(csName).getCollection(mCLName);
		}catch(BaseException e){
			Assert.fail(e.getMessage());
		}
		
		//-----create index-----
		try{
			String name = idxName;
			Random i = new Random();
			BSONObject opt = new BasicBSONObject();
			opt.put("a" + i.nextInt(10000), 1);
			clDB.createIndex(name + i.nextInt(10000), opt, false, false);
		}catch(BaseException e){
			if(e.getErrorCode() != -247){  //-247:Redefine index
				Assert.fail(e.getMessage());
			}
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
			opt.put("ReplSize", 0);
			for(int i = 0; i < 10; i++){
				sdb.getCollectionSpace(csName).createCollection(sCLName + i, opt);
			}
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
			for(int i = 0; i < 10; i++){
				int bound = i * 100;
				lowBoundObj.put("a", bound);
				upBoundObj.put("a", bound + 100);
				options.put("LowBound", lowBoundObj);
				options.put("UpBound", upBoundObj);
				sdb.getCollectionSpace(csName).getCollection(mCLName).
					attachCollection(csName + "." + sCLName + i, options);
			}
		}catch(BaseException e){
			Assert.fail(e.getMessage());
		}
	}	
}