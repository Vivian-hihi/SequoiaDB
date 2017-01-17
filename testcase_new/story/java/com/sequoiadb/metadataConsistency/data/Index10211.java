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

import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.metadataConsistency.data.CommLib;
import com.sequoiadb.testcommon.SdbTestBase;

/**
* TestLink: seqDB-10211: concurrency[createIndex]
* @author xiaoni huang init
* @Date   2016.10.17
*/

public class Index10211 extends SdbTestBase {
	private CommLib CommLib = new CommLib();
	private SimpleDateFormat dateFm = new SimpleDateFormat("YYYY-MM-dd HH:mm:ss");
	private static Sequoiadb sdb = null;
	private String csName = "cs10211";
	private String clName = "cl10211";
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
			//create cs/cl
			sdb.createCollectionSpace(csName);
			sdb.getCollectionSpace(csName).createCollection(clName);
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
	public void testIndex10211(){
		Sequoiadb db  = null;
		DBCollection clDB = null;
		try{
			db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
			clDB = db.getCollectionSpace(csName).getCollection(clName);
		}catch(BaseException e){
			db.disconnect();
			Assert.fail(e.getMessage());
		}
		
		//-----create index-----
		try{
			BSONObject opt = new BasicBSONObject();
			opt.put("a", 1);
			clDB.createIndex(idxName, opt, false, false);
		}catch(BaseException e){
			if(e.getErrorCode() != -247 //-247:Redefine index
					&& e.getErrorCode() != -43){  //-43:Failed to initialize index
				db.disconnect();
				Assert.fail(e.getMessage());
			}
		}
		
		//-----drop index-----
		try{
			clDB.dropIndex(idxName);
		}catch(BaseException e){
			if(e.getErrorCode() != -47){  //-47:Index name does not exist
				Assert.fail(e.getMessage());
			}
		}finally{
			db.disconnect();
		}
		
	}
	
}