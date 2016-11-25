package com.story.consistencyData;

import java.text.SimpleDateFormat;
import java.util.Date;

import org.testng.annotations.Test;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.AfterClass;
import org.bson.BSONObject;
import org.bson.BasicBSONObject;
import org.testng.Assert;

import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.DBCursor;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.testcommon.SdbTestBase;
import com.story.consistencyData.CommLib;

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
	public void testIdIndex10211(){
		Sequoiadb db  = null;
		DBCollection clDB = null;
		try{
			db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
			clDB = db.getCollectionSpace(csName).getCollection(clName);
		}catch(BaseException e){
			Assert.fail(e.getMessage());
		}
		
		//-----create index-----
		try{
			BSONObject opt = new BasicBSONObject();
			opt.put("a", 1);
			clDB.createIndex(idxName, opt, false, false);
			//check results
			if(clDB != null)checkResult(db);
		}catch(BaseException e){
			if(e.getErrorCode() != -247){  //-247:Redefine index
				Assert.fail(e.getMessage());
			}
		}
		
		//-----drop index-----
		try{
			clDB.dropIndex(idxName);
			//check results
			checkResult(db);
		}catch(BaseException e){
			if(e.getErrorCode() != -47){  //-47:Index name does not exist
				Assert.fail(e.getMessage());
			}
		}finally{
			db.disconnect();
		}
		
	}
	
	public void checkResult(Sequoiadb sdb){
		try{
			if(CommLib.isStandAlone(sdb)){
				DBCollection clDB = sdb.getCollectionSpace(csName).getCollection(clName);
				DBCursor idxInfo = clDB.getIndex("$id");
				if(idxInfo.hasNext()){
					clDB.dropIdIndex();
				}else{
					BSONObject opt = new BasicBSONObject();
					opt.put("SortBufferSize", 128);
					clDB.createIdIndex(opt);
				}
			}else{
				CommLib.checkIndex(sdb, csName, clName);
			}
		}catch(BaseException e){
			Assert.fail(e.getMessage());
		}
	}
}