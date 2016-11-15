package com.story.consistency;

import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.Random;

import org.testng.annotations.Test;
import org.testng.annotations.Parameters;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.AfterClass;
import org.testng.Assert;

import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.exception.BaseException;
import com.story.consistency.CommLib;

/**
* TestLink: seqDB-10163
* 			seqDB-10165
* 			seqDB-10166
* @author xiaoni huang init
* @Date   2016.9.25
*/

public class CS10163 {
	private CommLib CommLib = new CommLib();
	private SimpleDateFormat dateFm = new SimpleDateFormat("YYYY-MM-dd HH:mm:ss");
	private static Sequoiadb sdb = null;
	private String csName = "cs10163";
	private Random randomInt = new Random();
	
	@BeforeClass
	@Parameters({"coordAddr"})
	public void setUp(String coordAddr){
		//start time
		System.out.println("Begin to run " + this.getClass().getName() 
					+ ", begin in: " + dateFm.format(new Date().getTime()));
		try{
			sdb = new Sequoiadb(coordAddr, "", "");
			//drop cs
			CommLib.clearCS(sdb, csName);
		}catch(BaseException e){
			Assert.fail("Failed to prepare env at th begining. "
					+ "ErrorMsg:\n" +e.getMessage());
		}
		
	}
	
	@AfterClass
	public void tearDown(){
		try{
			//check results in the end
			CommLib.checkCSOfCatalog(sdb, csName);
			//drop cs
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
	@Parameters({"coordAddr"})
	public void testCS10163(String coordAddr){
		Sequoiadb db  = null;
		//-----create cs-----
		try
		{
			db = new Sequoiadb(coordAddr, "", "");
			String tmpName = csName;
			//random generate domain name
			tmpName = csName + "_" + randomInt.nextInt(10000);
			//create domain
		    db.createCollectionSpace(tmpName);
			//check results of catalog
			CommLib.checkCSOfCatalog(db, csName);
		}catch(BaseException e){
			if(e.getErrorCode() != -33){  //-33:Collection space already exists
				db.disconnect();
				Assert.fail(e.getMessage());
			}
		}
		
		//-----drop cs-----
		try{
			String tmpName = csName;
			//random generate cs name
			tmpName = csName + "_" + randomInt.nextInt(10000);
			if(db.isCollectionSpaceExist(tmpName)){
				db.dropCollectionSpace(tmpName);
			}
			//check results of catalog
			CommLib.checkCSOfCatalog(db, csName);
		}catch(BaseException e){
			if(e.getErrorCode() != -34){ //-34:Collection space does not exist
				db.disconnect();
				Assert.fail(e.getMessage());
			}
		}
	}
		
}