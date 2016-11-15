package com.story.consistency;

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
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.testcommon.SdbTestBase;
import com.story.consistency.CommLib;

/**
* TestLink: 
* 		seqDB-10171: concurrency[createCL, alterCL, dropCL]
* @author xiaoni huang init
* @Date   2016.10.11
*/

public class CL10171 extends SdbTestBase {
	private CommLib CommLib = new CommLib();
	private static Sequoiadb sdb = null;
	private String csName = "cs10171";
	private String clName = "cl10171";
	private Random random = new Random();
	private SimpleDateFormat dateFm = new SimpleDateFormat("YYYY-MM-dd HH:mm:ss");
	
	@BeforeClass
	public void setUp(){
		System.out.println("Begin to run " + this.getClass().getName() 
					+ ", begin in: " + dateFm.format(new Date().getTime()));
		try{
			sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "");
			
			//clear environment
			CommLib.clearCS(sdb, csName);
			//ready environment
			sdb.createCollectionSpace(csName);
			
		}catch(BaseException e){
			Assert.fail("Failed to prepare env at th begining. "
					+ "ErrorMsg:\n" +e.getMessage());
		}
	}
	
	@AfterClass
	public void tearDown(){
		try{
			//clear environment
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
			csDB = db.getCollectionSpace(csName);
		}catch(BaseException e){
			Assert.fail(e.getMessage());
		}
		
		//-----create collection-----
		try
		{
			csDB.createCollection(clName + "_" + random.nextInt(20));
			//check results
			CommLib.checkCLResult(db, csName, clName);
			
		}catch(BaseException e){
			if(e.getErrorCode() != -22){   //-22:Collection already exists
				Assert.fail(e.getMessage());
			}
		}finally{
			db.disconnect();
		}
		
	}	
	
	@Test(invocationCount = 20, threadPoolSize = 20)
	public void testAlterCL(){
		Sequoiadb db  = null;
		CollectionSpace csDB = null;
		try{
			db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
			csDB = db.getCollectionSpace(csName);
		}catch(BaseException e){
			Assert.fail(e.getMessage());
		}
		
		//-----alter collection-----
		try{
			BSONObject opt = new BasicBSONObject();
			opt.put("ReplSize", 1 );
			if(csDB.isCollectionExist(clName)){
				csDB.getCollection(clName).alterCollection(opt);
			}
			//check results 
		    CommLib.checkCLResult(db, csName, clName);
		}catch(BaseException e){
			if(e.getErrorCode() != -23){  //-23:Collection does not exist
				db.disconnect();
				Assert.fail(e.getMessage());
			}
		}finally{
			db.disconnect();
		}
	}
	
	@Test(invocationCount = 20, threadPoolSize = 20)
	public void testDropCL(){
		Sequoiadb db  = null;
		CollectionSpace csDB = null;
		try{
			db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
			csDB = db.getCollectionSpace(csName);
		}catch(BaseException e){
			Assert.fail(e.getMessage());
		}
		
		//-----drop collection-----
		try
		{
			csDB.dropCollection(clName + "_" + random.nextInt(20));
			//check results
			CommLib.checkCLResult(db, csName, clName);
			
		}catch(BaseException e){
			if(e.getErrorCode() != -23){   //-23:Collection does not exist
				Assert.fail(e.getMessage());
			}
		}finally{
			db.disconnect();
		}
	}
		
}