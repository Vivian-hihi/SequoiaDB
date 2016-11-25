package com.story.consistencyData;

import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Date;
import java.util.Random;
import org.bson.BSONObject;
import org.bson.BasicBSONObject;
import org.testng.annotations.Test;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.AfterClass;
import org.testng.Assert;
import org.testng.SkipException;

import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.testcommon.SdbTestBase;
import com.story.consistencyData.CommLib;

/**
* TestLink: seqDB-10161
*			seqDB-10162
* @author xiaoni huang init
* @Date   2016.9.20
*/

public class Domain10161 extends SdbTestBase {
	private SimpleDateFormat dateFm = new SimpleDateFormat("YYYY-MM-dd HH:mm:ss");
	private CommLib CommLib = new CommLib();
	private static Sequoiadb sdb = null;
	private static ArrayList<String> dataGroups = null;
	private String domainName = "dm10161";
	private Random random = new Random();
	private int number = 20;
	
	@BeforeClass
	public void setUp(){
		//start time
		System.out.println("Begin to run " + this.getClass().getName() 
					+ ", begin in: " + dateFm.format(new Date().getTime()));
		try{
			sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "");
			//judge the mode and group number
			if(CommLib.isStandAlone(sdb) || CommLib.OneGroupMode(sdb)){
				throw new SkipException("The mode is standlone, or only one group, "
						+ "skip the testCase.");
			}
			//clear env
			CommLib.clearDomain(sdb, domainName);
			//ready env
			dataGroups = CommLib.getDataGroupNames(sdb);
		}catch(BaseException e){
			Assert.fail("Failed to prepare env at th begining. "
					+ "ErrorMsg:\n" +e.getMessage());
		}
		
	}
	
	@AfterClass
	public void tearDown(){
		try{
			//clear env
			CommLib.clearDomain(sdb, domainName);
		}catch(BaseException e){
			Assert.fail("ErrorMsg:\n" +e.getMessage());
		}finally{
			System.out.println("End to run " + this.getClass().getName() 
						+ ", end in: " + dateFm.format(new Date().getTime()));
			sdb.disconnect();
		}
	}
	
	@Test(invocationCount = 10, threadPoolSize = 10)
	public void testCreateDomain(){
		Sequoiadb db  = null;
		try{
			db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
		}catch(BaseException e){
			Assert.fail(e.getMessage());
		}
		
		try
		{
			BSONObject opt = new BasicBSONObject();
			opt.put( "Groups", dataGroups );
			opt.put( "AutoSplit", false );
			for(int i = 0; i < 10; i++){
			    db.createDomain (domainName + "_" + random.nextInt(number), opt);
			}
			//check results of catalog
			CommLib.checkDomainOfCatalog(db, domainName);
		}catch(BaseException e){
			if(e.getErrorCode() != -215){  //-215:Domain already exists
				Assert.fail(e.getMessage());
			}
		}
		finally{
			db.disconnect();
		}
	}

	@Test(invocationCount = 10, threadPoolSize = 10)
	public void testAlterDomain(){
		Sequoiadb db  = null;
		try{
			db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
		}catch(BaseException e){
			Assert.fail(e.getMessage());
		}
		
		try{
			BSONObject opt = new BasicBSONObject();
			int drSize = random.nextInt(dataGroups.size());
			opt.put( "Groups", dataGroups.get(drSize).split(",") );
			opt.put( "AutoSplit", true );
			for(int i = 0; i < 10; i++){
				db.getDomain(domainName + "_" + random.nextInt(number)).alterDomain(opt);
			}
			//check results of catalog
			CommLib.checkDomainOfCatalog(db, domainName);
		}catch(BaseException e){
			if(e.getErrorCode() != -214){  //-214:Domain does not exist
				Assert.fail(e.getMessage());
			}
		}
		finally{
			db.disconnect();
		}
	}

	@Test(invocationCount = 10, threadPoolSize = 10)
	public void testDropDomain(){
		Sequoiadb db  = null;
		try{
			db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
		}catch(BaseException e){
			Assert.fail(e.getMessage());
		}
		
		try{
			for(int i = 0; i < 5; i++){
				db.dropDomain(domainName + "_" + random.nextInt(number));
			}
			//check results of catalog
			CommLib.checkDomainOfCatalog(db, domainName);
		}catch(BaseException e){
			if(e.getErrorCode() != -214){  //-214:Domain does not exist
				db.disconnect();
				Assert.fail(e.getMessage());
			}
		}
		finally{
			db.disconnect();
		}
	}
		
}
