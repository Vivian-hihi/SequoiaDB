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

import com.sequoiadb.base.CollectionSpace;
import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.metadataConsistency.data.CommLib;
import com.sequoiadb.testcommon.SdbTestBase;

/**
* TestLink: seqDB-10212: concurrency[createIndex, dropCL]
* @author xiaoni huang init
* @Date   2016.10.17
*/

public class Index10212 extends SdbTestBase {
	private CommLib CommLib = new CommLib();
	private SimpleDateFormat dateFm = new SimpleDateFormat("YYYY-MM-dd HH:mm:ss");
	private static Sequoiadb sdb = null;
	private String csName = "cs10212";
	private String clName = "cl10212";
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
	public void testIndex10212(){
		Sequoiadb db  = null;
		CollectionSpace csDB = null;
		try{
			db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
			csDB = db.getCollectionSpace(csName);
		}catch(BaseException e){
			Assert.fail(e.getMessage());
		}
		
		//-----create index-----
		try{
			String name = idxName;
			Random i = new Random();
			BSONObject opt = new BasicBSONObject();
			opt.put("a" + i.nextInt(10000), 1);
			if(csDB != null){
				DBCollection clDB = csDB.getCollection(clName);
				if(clDB != null){
					clDB.createIndex(name + i.nextInt(100), opt, false, false);
				}
			}
		}catch(BaseException e){
			if( e.getErrorCode() != -247  //-247:Redefine index
					&& e.getErrorCode() != -46  //-46:Duplicate index name
					&& e.getErrorCode() != -23
					&& e.getErrorCode() != -34  
					//because is only one CL in CS, delete the CS data file when deleting the last CL, so exception -34 when creatIndex 
					&& e.getErrorCode() != -108){ //-108:Catalog version is expired on coordinator node
				db.disconnect();
				Assert.fail(e.getMessage());
			}
		}
		
		//-----drop cl-----
		try{
			db.getCollectionSpace(csName).dropCollection(clName);
		}catch(BaseException e){
			if(e.getErrorCode() != -23
					&& e.getErrorCode() != -147){ //-147:Unable to lock
				db.disconnect();
				Assert.fail(e.getMessage());
			}
		}
		
		//-----create cl-----
		try{
			db.getCollectionSpace(csName).createCollection(clName);
		}catch(BaseException e){
			if(e.getErrorCode() != -22){  
				Assert.fail(e.getMessage());
			}
		}finally{
			db.disconnect();
		}
		
	}
	
}