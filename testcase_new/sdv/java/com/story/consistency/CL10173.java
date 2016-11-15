package com.story.consistency;

import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.Random;

import org.testng.annotations.Test;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.AfterClass;
import org.testng.Assert;

import com.sequoiadb.base.CollectionSpace;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.testcommon.SdbTestBase;
import com.story.consistency.CommLib;

/**
* TestLink: 
* 		seqDB-10173: concurrency[createCL, dropCS]
* @author xiaoni huang init
* @Date   2016.10.11
*/

public class CL10173 extends SdbTestBase {
	private CommLib CommLib = new CommLib();
	private static Sequoiadb sdb = null;
	private String csName = "cs10173";
	private String clName = "cl10173";
	private Random random = new Random();
	private SimpleDateFormat dateFm = new SimpleDateFormat("YYYY-MM-dd HH:mm:ss");
	
	@BeforeClass
	public void setUp(){
		System.out.println("Begin to run " + this.getClass().getName() 
					+ ", begin in: " + dateFm.format(new Date().getTime()));
		try{
			sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "");
			//clear env
			CommLib.clearCS(sdb, csName);
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
	
	@Test(invocationCount = 20, threadPoolSize = 20)
	public void testCreateCL(){
		Sequoiadb db  = null;
		CollectionSpace csDB = null;
		try{
			db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
			if(!db.isCollectionSpaceExist(csName)){
				db.createCollectionSpace(csName);
			}
			csDB = db.getCollectionSpace(csName);
		}catch(BaseException e){
			if(e.getErrorCode() != -33){  //-33:Collection space already exists
				Assert.fail(e.getMessage());
			}
		}
		
		//-----create collection-----
		try
		{
		    csDB.createCollection(clName + "_" + random.nextInt(100));
			//check results of catalog
			CommLib.checkCLResult(db, csName, clName);
			
		}catch(BaseException e){
			if(e.getErrorCode() != -22 //-22:Collection already exists
					&& e.getErrorCode() != -34){  //-34:Collection space does not exist
				Assert.fail(e.getMessage());
			}
		}finally{
			db.disconnect();
		}
		
	}
	
	@Test(invocationCount = 20, threadPoolSize = 20)
	public void testDropCS(){
		Sequoiadb db  = null;
		try{
			db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
		}catch(BaseException e){
			Assert.fail(e.getMessage());
		}

		//-----drop collection space-----
		try{
			db.dropCollectionSpace(csName);
		}catch(BaseException e){
			if(e.getErrorCode() != -34){  //-34:Collection space does not exist
				Assert.fail(e.getMessage());
			}
		}finally{
			db.disconnect();
		}
	}
		
}