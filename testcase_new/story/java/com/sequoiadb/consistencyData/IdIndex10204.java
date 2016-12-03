package com.sequoiadb.consistencyData;

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
import com.sequoiadb.consistencyData.CommLib;
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.testcommon.SdbTestBase;

/**
* TestLink: seqDB-10204: concurrency[create IdIndex]
* @author xiaoni huang init
* @Date   2016.10.17
*/

public class IdIndex10204 extends SdbTestBase {
	private SimpleDateFormat dateFm = new SimpleDateFormat("YYYY-MM-dd HH:mm:ss");
	private CommLib CommLib = new CommLib();
	private static Sequoiadb sdb = null;
	private String csName = "cs10204";
	private String clName = "cl10204";
	
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
	public void testIdIndex10204(){
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
			opt.put("SortBufferSize", 128);
			clDB.createIdIndex(opt);
			//check results
			CommLib.checkIndex(sdb, csName, clName);
		}catch(BaseException e){
			db.disconnect();
			Assert.fail(e.getMessage());
		}
		
		//-----drop index-----
		try{
			clDB.dropIdIndex();
			//check results
			CommLib.checkIndex(sdb, csName, clName);
		}catch(BaseException e){
			Assert.fail(e.getMessage());
		}finally{
			db.disconnect();
		}
		
	}
}