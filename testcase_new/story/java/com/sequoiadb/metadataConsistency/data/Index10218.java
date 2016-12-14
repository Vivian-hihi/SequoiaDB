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
* TestLink: seqDB-10218: concurrency[dropIndex, dropCS]
* @author xiaoni huang init
* @Date   2016.10.24
*/

public class Index10218 extends SdbTestBase {
	private CommLib CommLib = new CommLib();
	private SimpleDateFormat dateFm = new SimpleDateFormat("YYYY-MM-dd HH:mm:ss");
	private static Sequoiadb sdb = null;
	private String csName = "cs10218";
	private String clName = "cl10218";
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
			//create index
			this.createIndex(sdb);
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
	public void testIndex10218(){
		Sequoiadb db  = null;
		DBCollection clDB = null;
		try{
			db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
			clDB = db.getCollectionSpace(csName).getCollection(clName);
		}catch(BaseException e){
			if(e.getErrorCode() != -34){
				Assert.fail(e.getMessage());
			}
		}
		
		//-----drop index-----
		try{
			String name = idxName;
			Random i = new Random();
			clDB.dropIndex(name + i.nextInt(42));
			//check results
			CommLib.checkIndex(db, csName, clName);
		}catch(NullPointerException e){
			
		}catch(BaseException e){
			if(e.getErrorCode() != -47  //-47:Index name does not exist
					&& e.getErrorCode() != -248  //-248:Dropping the collection space is in progress
					&& e.getErrorCode() != -23
					&& e.getErrorCode() != -34){  
				Assert.fail(e.getMessage());
			}
		}
		
		//-----drop cs-----
		try{
			db.dropCollectionSpace(csName);
			//check results
			CommLib.checkCSOfCatalog(db, csName);
		}catch(BaseException e){
			if(e.getErrorCode() != -34){
				Assert.fail(e.getMessage());
			}
		}
		
		//-----create cs-----
		try{
			db.createCollectionSpace(csName).createCollection(clName);
		}catch(BaseException e){
			if(e.getErrorCode() != -33 
					&& e.getErrorCode() != -34
					&& e.getErrorCode() != -22){
				Assert.fail(e.getMessage());
			}
		}finally{
			db.disconnect();
		}
		
	}
	
	public void createIndex(Sequoiadb sdb){
		try{
			for(int i = 0; i < 42; i++){
				BSONObject opt = new BasicBSONObject();
				opt.put("a" + i, 1);
				String name = idxName;
				sdb.getCollectionSpace( csName ).getCollection( clName ).
						createIndex( name + i, opt, false, false );
				
			}
		}catch(BaseException e){
			Assert.fail(e.getMessage());
		}
	}
	
}