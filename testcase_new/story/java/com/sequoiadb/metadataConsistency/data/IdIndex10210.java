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
* TestLink: seqDB-10210: concurrency[dropIdIndex, dropCS]
* @author xiaoni huang init
* @Date   2016.10.17
*/

public class IdIndex10210 extends SdbTestBase {
	private CommLib CommLib = new CommLib();
	private SimpleDateFormat dateFm = new SimpleDateFormat("YYYY-MM-dd HH:mm:ss");
	private static Sequoiadb sdb = null;
	private String csName = "cs10210";
	private String clName = "cl10210";
	
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
			BSONObject opt = new BasicBSONObject();
			opt.put("AutoIndexId", false);
			sdb.getCollectionSpace(csName).createCollection(clName, opt);
			
			//create index
			DBCollection clDB = sdb.getCollectionSpace(csName).getCollection(clName);
			BSONObject opt2 = new BasicBSONObject();
			opt2.put("SortBufferSize", 128);
			clDB.createIdIndex(opt2);
			
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
	public void testIdIndex10210(){
		Sequoiadb db  = null;
		DBCollection clDB = null;
		try{
			db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
			if(db.isCollectionSpaceExist(csName)){
				clDB = db.getCollectionSpace(csName).getCollection(clName);
			}
		}catch(BaseException e){
			db.disconnect();
			Assert.fail(e.getMessage());
		}

		//-----drop index-----
		try{
			if(clDB != null){
				clDB.dropIdIndex();
			}
			//check results
			if( db.isCollectionSpaceExist(csName) ){
				CommLib.checkIndex(db, csName, clName);
			}
		}catch(BaseException e){
			if(e.getErrorCode() != -248  //-248: Dropping the collection space is in progress
					&& e.getErrorCode() != -23
					&& e.getErrorCode() != -34){ 
				db.disconnect();
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
				db.disconnect();
				Assert.fail(e.getMessage());
			}
		}
		
		//-----create cs-----
		try{
			db.createCollectionSpace(csName);
		}catch(BaseException e){
			if(e.getErrorCode() != -33){
				db.disconnect();
				Assert.fail(e.getMessage());
			}
		}
		
		//-----create cl-----
		try{
			BSONObject opt = new BasicBSONObject();
			opt.put("AutoIndexId", false);
			db.getCollectionSpace(csName).createCollection(clName, opt);
		}catch(BaseException e){
			if(e.getErrorCode() != -34 && e.getErrorCode() != -22){
				db.disconnect();
				Assert.fail(e.getMessage());
			}
		}
		
		//-----create index-----
		try{
			BSONObject opt = new BasicBSONObject();
			opt.put("SortBufferSize", 128);
			if(clDB != null){
				clDB.createIdIndex(opt);
			}
			//check results
			if(db.isCollectionSpaceExist(csName)){
				CommLib.checkIndex(db, csName, clName);
			}
		}catch(BaseException e){
			if(e.getErrorCode() != -248 //-248:Dropping the collection space is in progress
					&& e.getErrorCode() != -23
					&& e.getErrorCode() != -34){
				Assert.fail(e.getMessage());
			}
		}finally{
			db.disconnect();
		}
		
	}
	
}