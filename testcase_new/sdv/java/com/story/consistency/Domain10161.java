package com.story.consistency;

import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Date;
import java.util.Random;
import java.util.concurrent.atomic.AtomicInteger;

import org.bson.BSONObject;
import org.bson.BasicBSONObject;
import org.testng.annotations.Test;
import org.testng.annotations.Parameters;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.AfterClass;
import org.testng.Assert;
import org.testng.SkipException;

import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.exception.BaseException;
import com.story.consistency.CommLib;

/**
* TestLink: seqDB-10161
*			seqDB-10162
* @author xiaoni huang init
* @Date   2016.9.20
*/

public class Domain10161 {
	private CommLib CommLib = new CommLib();
	private SimpleDateFormat dateFm = new SimpleDateFormat("YYYY-MM-dd HH:mm:ss");
	private AtomicInteger totalCreateNumber = new AtomicInteger(0);
	private AtomicInteger totalDropNumber = new AtomicInteger(0);
	private static Sequoiadb sdb = null;
	private BSONObject domainOption = new BasicBSONObject();
	private String domainName = "domain10161";
	
	private Random randomInt = new Random();
	
	@BeforeClass
	@Parameters({"coordAddr"})
	public void setUp(String coordAddr){
		//start time
		System.out.println("Begin to run " + this.getClass().getName() 
					+ ", begin in: " + dateFm.format(new Date().getTime()));
		try{
			sdb = new Sequoiadb(coordAddr, "", "");
			//judge the mode and group number
			if(CommLib.isStandAlone(sdb) || CommLib.OneGroupMode(sdb)){
				throw new SkipException("The mode is standlone, or only one group, "
						+ "skip the testCase.");
			}
			//drop domain
			CommLib.clearDomain(sdb, domainName);
		}catch(BaseException e){
			Assert.fail("Failed to prepare env at th begining. "
					+ "ErrorMsg:\n" +e.getMessage());
		}
		
	}
	
	@AfterClass
	public void tearDown(){
		try{
			//check results in the end
			CommLib.checkDomainOfCatalog(sdb, domainName);
			//drop domain
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
	@Parameters({"coordAddr"})
	public void testDomain10161(String coordAddr){
		Sequoiadb db  = null;
		ArrayList<String> dataGroups = new ArrayList<String>();
		String tmpName = domainName;
		//-----create domain-----
		try
		{
			db = new Sequoiadb(coordAddr, "", "");
			dataGroups = CommLib.getDataGroupNames(db);
			
			//random generate domain name
			tmpName = domainName + "_" + randomInt.nextInt(10000);
			domainOption.put( "Groups", dataGroups );
			domainOption.put( "AutoSplit", true );
			//probability execution create domain
			if (totalCreateNumber.get()== 0 ||
				totalCreateNumber.get()*1.0/(totalCreateNumber.get() + totalDropNumber.get()) <= 0.6){
				totalCreateNumber.incrementAndGet();
			    db.createDomain ( tmpName, domainOption);
			}
			//check results of catalog
			CommLib.checkDomainOfCatalog(db, domainName);
		}catch(BaseException e){
			if(e.getErrorCode() != -215){  //-215:Domain already exists
				db.disconnect();
				Assert.fail(e.getMessage());
			}
		}
		
		//-----alter domain-----
		try{
			dataGroups.remove(0);
			domainOption.put( "Groups", dataGroups );
			domainOption.put( "AutoSplit", false );
			db.getDomain(tmpName).alterDomain(domainOption);
			//check results of catalog
			CommLib.checkDomainOfCatalog(db, domainName);
		}catch(BaseException e){
			if(e.getErrorCode() != -214){  //-214:Domain does not exist
				db.disconnect();
				Assert.fail(e.getMessage());
			}
		}
			
		//----drop domain-----
		tmpName = domainName;
		try
		{
			//random generate domain name
			tmpName = tmpName + "_" + randomInt.nextInt(1000);
			//probability execution drop domain
			if (totalCreateNumber.get() == 0 ||
			    totalCreateNumber.get()*1.0/(totalCreateNumber.get() + totalDropNumber.get()) <= 0.4){
				totalDropNumber.incrementAndGet();
			    db.dropDomain(tmpName);
			}
			//check results of catalog
			CommLib.checkDomainOfCatalog(db, domainName);
		}catch(BaseException e){
			if(e.getErrorCode() != -214){
				Assert.fail(e.getMessage());
			}
		}
		finally{
			db.disconnect();
		}
	}
		
}
