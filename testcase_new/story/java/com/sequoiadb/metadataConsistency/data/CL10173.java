package com.sequoiadb.metadataconsistency.data;

import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.Random;

import org.testng.annotations.Test;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.AfterClass;
import org.testng.Assert;
import org.testng.SkipException;

import com.sequoiadb.base.CollectionSpace;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.metadataconsistency.data.CommLib;
import com.sequoiadb.testcommon.SdbTestBase;
import com.sequoiadb.testcommon.SdbThreadBase;

/**
* TestLink: 
* 		seqDB-10173: concurrency[createCL, dropCS]
* @author xiaoni huang init
* @Date   2016.10.11
*/

public class CL10173 extends SdbTestBase {
	private SimpleDateFormat dateFm = new SimpleDateFormat("YYYY-MM-dd HH:mm:ss");
	private static Sequoiadb sdb = null;
	private String csName = "cs10173";
	private String clName = "cl10173";
	private Random random = new Random();
	private int msec = 100;
	
	@BeforeClass
	public void setUp(){
		System.out.println("Begin to run " + getClass().getName() 
					+ ", begin in: " + dateFm.format(new Date().getTime()));
		try{
			sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "");
			//judge the mode
			if(CommLib.isStandAlone(sdb)){
				throw new SkipException("The mode is standlone, " + "skip the testCase.");
			}
			CommLib.clearCS(sdb, csName);
			sdb.createCollectionSpace(csName);
		}catch(BaseException e){
			sdb.disconnect();
			Assert.fail(e.getMessage());
		}
	}
	
	@AfterClass
	public void tearDown(){
		try{
			CommLib.clearCS(sdb, csName);
		}catch(BaseException e){
			Assert.fail(e.getMessage());
		}finally{
			System.out.println("End to run " + getClass().getName() 
						+ ", end in: " + dateFm.format(new Date().getTime()));
			sdb.disconnect();
		}
	}
	
	@Test
	public void test(){
		CreateCL createCL = new CreateCL();
		createCL.start();

		DropCS dropCS = new DropCS();
		CommLib.sleep(random.nextInt(msec));
		dropCS.start();
		
		if( !( createCL.isSuccess() && dropCS.isSuccess() ) ){
			Assert.fail(createCL.getErrorMsg() + dropCS.getErrorMsg());
		}
		
		//check results
		CommLib.checkCLResult(csName, clName);
	}

	private class CreateCL extends SdbThreadBase{
		@Override
		public void exec() throws BaseException{
			Sequoiadb db  = null;
			try
			{
				db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
				CollectionSpace csDB = db.getCollectionSpace(csName);
				
				csDB.createCollection(clName);
				if(csDB.isCollectionExist(clName)){
					CommLib.insertData(db, csName, clName);
				}
			}catch(BaseException e){
				int eCode = e.getErrorCode();
				if( eCode != -23 && eCode != -34 
						&& eCode != -248 && eCode != -147 ){ 
					throw e;
				}
			}
			finally{
				db.disconnect();
			}
		}
	}

	private class DropCS extends SdbThreadBase{
		@Override
		public void exec() throws BaseException{
			Sequoiadb db  = null;
			try{
				db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
				db.dropCollectionSpace(csName);
			}catch(BaseException e){
				int eCode = e.getErrorCode();
				if( eCode != -34 && eCode != -147){  
					throw e;
				}
			}finally{
				db.disconnect();
			}
		}
	}
}