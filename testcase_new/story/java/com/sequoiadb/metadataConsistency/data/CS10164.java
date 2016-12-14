package com.sequoiadb.metadataConsistency.data;

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
import com.sequoiadb.metadataConsistency.data.CommLib;
import com.sequoiadb.testcommon.SdbTestBase;

/**
* TestLink: 
* 		seqDB-10164: concurrency[alter subCL, drop mainCL]
* @author xiaoni huang init
* @Date   2016.9.26
*/

public class CS10164 extends SdbTestBase {
	private SimpleDateFormat dateFm = new SimpleDateFormat("YYYY-MM-dd HH:mm:ss");
	private CommLib CommLib = new CommLib();
	private static Sequoiadb sdb = null;
	private static ArrayList<String> dataGroups = null;
	private String domainName = "dm10164";
	private String csName = "cs10164";
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
			if(CommLib.isStandAlone(sdb)){
				throw new SkipException("The mode is standlone, " + "skip the testCase.");
			}
			//clear env
			CommLib.clearCS(sdb, csName);
			CommLib.clearDomain(sdb, domainName);
			//ready env
			dataGroups = CommLib.getDataGroupNames(sdb);
			this.createDomain(sdb);
			
		}catch(BaseException e){
			Assert.fail("Failed to prepare env at th begining. "
					+ "ErrorMsg:\n" +e.getMessage());
		}
		
	}
	
	@AfterClass
	public void tearDown(){
		try{
			//check results of catalog
			CommLib.checkDomainOfCatalog(sdb, domainName);
			CommLib.checkCSOfCatalog(sdb, csName);
			
			//clear env
			CommLib.clearCS(sdb, csName);
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
	public void testCreateCS(){
		Sequoiadb db  = null;
		try
		{
			db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
			
			BSONObject opt = new BasicBSONObject();
			opt.put( "Domain", domainName );
		    db.createCollectionSpace(csName + "_" + random.nextInt(number), opt);
		}catch(BaseException e){
			if(e.getErrorCode() != -33){
				Assert.fail(e.getMessage());
			}
		}finally{
			db.disconnect();
		}
	}
	
	@Test(invocationCount = 10, threadPoolSize = 10)
	public void testAlterDomain(){
		Sequoiadb db  = null;
		try
		{
			db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
			
			BSONObject opt = new BasicBSONObject();
			opt.put( "Groups", dataGroups.get(0).split(",") );
			opt.put( "AutoSplit", false );
			db.getDomain(domainName).alterDomain(opt);
		}catch(BaseException e){
			Assert.fail(e.getMessage());
		}finally{
			db.disconnect();
		}
	}
	
	public void createDomain(Sequoiadb sdb){
		try{
			BSONObject opt = new BasicBSONObject();
			if(!sdb.isDomainExist(domainName)){
				opt.put( "Groups", dataGroups );
				opt.put( "AutoSplit", true );
			    sdb.createDomain ( domainName, opt);
			}
		}catch(BaseException e){
			Assert.fail(e.getMessage());
		}
	}
		
}