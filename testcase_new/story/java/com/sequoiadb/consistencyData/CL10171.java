package com.sequoiadb.consistencyData;

import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.Random;

import org.testng.annotations.Test;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.AfterClass;
import org.bson.BSONObject;
import org.bson.BasicBSONObject;
import org.testng.Assert;

import com.sequoiadb.base.CollectionSpace;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.consistencyData.CommLib;
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.testcommon.SdbTestBase;

/**
* TestLink: 
* 		seqDB-10171: concurrency[createCL, alterCL, dropCL]
* @author xiaoni huang init
* @Date   2016.10.11
*/

public class CL10171 extends SdbTestBase {
	private SimpleDateFormat dateFm = new SimpleDateFormat("YYYY-MM-dd HH:mm:ss");
	private CommLib CommLib = new CommLib();
	private static Sequoiadb sdb = null;
	private String csName = "cs10171";
	private String clName = "cl10171";
	private Random random = new Random();
	private int number = 20;
	
	@BeforeClass
	public void setUp(){
		System.out.println("Begin to run " + this.getClass().getName() 
					+ ", begin in: " + dateFm.format(new Date().getTime()));
		try{
			sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "");
			
			CommLib.clearCS(sdb, csName);
			sdb.createCollectionSpace(csName);
			
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
	public void testCreateCL(){
		Sequoiadb db = null;
		try
		{
			db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
			CollectionSpace csDB = db.getCollectionSpace(csName);
			
			csDB.createCollection(clName + "_" + random.nextInt(number));
			//check results
			CommLib.checkCLResult(db, csName, clName);
		}catch(BaseException e){
			if(e.getErrorCode() != -22){   
				Assert.fail(e.getMessage());
			}
		}finally{
			db.disconnect();
		}
	}	
	
	@Test(invocationCount = 10, threadPoolSize = 10)
	public void testAlterCL(){
		Sequoiadb db  = null;
		try{
			db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
			CollectionSpace csDB = db.getCollectionSpace(csName);
			
			BSONObject opt = new BasicBSONObject();
			opt.put("ReplSize", 1 );
			if(csDB.isCollectionExist(clName)){
				csDB.getCollection(clName + "_" + random.nextInt(number)).alterCollection(opt);
			}
			//check results 
		    CommLib.checkCLResult(db, csName, clName);
		}catch(BaseException e){
			if(e.getErrorCode() != -23){  
				Assert.fail(e.getMessage());
			}
		}finally{
			db.disconnect();
		}
	}
	
	@Test(invocationCount = 10, threadPoolSize = 10)
	public void testDropCL(){
		Sequoiadb db = null;
		try
		{
			db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
			CollectionSpace csDB = db.getCollectionSpace(csName);
			
			csDB.dropCollection(clName +"_"+ random.nextInt(number));
			//check results
			CommLib.checkCLResult(db, csName, clName);
		}catch(BaseException e){
			if(e.getErrorCode() != -23){  
				Assert.fail(e.getMessage());
			}
		}finally{
			db.disconnect();
		}
	}
		
}