package com.sequoiadb.consistencyData;

import java.text.SimpleDateFormat;
import java.util.Date;
import org.testng.annotations.Test;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.AfterClass;
import org.testng.Assert;

import com.sequoiadb.base.CollectionSpace;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.consistencyData.CommLib;
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.testcommon.SdbTestBase;

/**
* TestLink: 
* 		seqDB-10173: concurrency[createCL, dropCS]
* @author xiaoni huang init
* @Date   2016.10.11
*/

public class CL10173 extends SdbTestBase {
	private SimpleDateFormat dateFm = new SimpleDateFormat("YYYY-MM-dd HH:mm:ss");
	private CommLib CommLib = new CommLib();
	private static Sequoiadb sdb = null;
	private String csName = "cs10173";
	private String clName = "cl10173";
	
	@BeforeClass
	public void setUp(){
		System.out.println("Begin to run " + this.getClass().getName() 
					+ ", begin in: " + dateFm.format(new Date().getTime()));
		try{
			sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "");
			CommLib.clearCS(sdb, csName);
		}catch(BaseException e){
			Assert.fail("Failed to prepare env at th begining. "
					+ "ErrorMsg:\n" +e.getMessage());
		}
	}
	
	@AfterClass
	public void tearDown(){
		try{
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
	public void testCreateCL() throws Exception{
		Sequoiadb db  = null;
		CollectionSpace csDB = null;
		try
		{
			db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
			
			if(!db.isCollectionSpaceExist(csName)){
				csDB = db.createCollectionSpace(csName);
			}
			if(csDB != null)csDB.createCollection(clName);
			//check results
			CommLib.checkCLResult(db, csName, clName);
		}catch(BaseException e){
			if(e.getErrorCode() != -33 && e.getErrorCode() != -34
					&& e.getErrorCode() != -22){ 
				Assert.fail(e.getMessage());
			}
		}
		finally{
			db.disconnect();
		}
	}
	
	@Test(invocationCount = 10, threadPoolSize = 10)
	public void testDropCS(){
		Sequoiadb db  = null;
		try{
			db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
			
			db.dropCollectionSpace(csName);
		}catch(BaseException e){
			if(e.getErrorCode() != -34){  
				Assert.fail(e.getMessage());
			}
		}finally{
			db.disconnect();
		}
	}
		
}