package com.sequoiadb.metadataConsistency.data;

import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.Random;

import org.testng.annotations.Test;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.AfterClass;
import org.testng.Assert;
import org.testng.SkipException;

import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.metadataConsistency.data.CommLib;
import com.sequoiadb.testcommon.SdbTestBase;

/**
* TestLink: 
* 		seqDB-10165: concurrency[createCS, dropCS]
* @author xiaoni huang init
* @Date   2016.9.25
*/

public class CS10165 extends SdbTestBase {
	private SimpleDateFormat dateFm = new SimpleDateFormat("YYYY-MM-dd HH:mm:ss");
	private CommLib CommLib = new CommLib();
	private static Sequoiadb sdb = null;
	private String csName = "cs10165";
	private Random randomInt = new Random();
	private int number = 30;
	
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
			CommLib.clearCS(sdb, csName);
		}catch(BaseException e){
			Assert.fail("Failed to prepare env at th begining. "
					+ "ErrorMsg:\n" +e.getMessage());
		}
	}
	
	@AfterClass
	public void tearDown(){
		try{
			//check results
			CommLib.checkCSOfCatalog(sdb, csName);
			
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
	public void testCreateCS(){
		Sequoiadb db  = null;
		try
		{
			db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
		    db.createCollectionSpace(csName + "_" + randomInt.nextInt(number));
			//check results
			CommLib.checkCSOfCatalog(db, csName);
		}catch(BaseException e){
			if(e.getErrorCode() != -33
					&& e.getErrorCode() != -147){ 
				Assert.fail(e.getMessage());
			}
		}finally{
			db.disconnect();
		}
	}
	
	@Test(invocationCount = 10, threadPoolSize = 10)
	public void testDropCS(){
		Sequoiadb db  = null;
		try
		{
			db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
			db.dropCollectionSpace(csName + "_" + randomInt.nextInt(number));
			//check results
			CommLib.checkCSOfCatalog(db, csName);
		}catch(BaseException e){
			if(e.getErrorCode() != -34
					&& e.getErrorCode() != -147){ 
				Assert.fail(e.getMessage());
			}
		}finally{
			db.disconnect();
		}
	}
		
}