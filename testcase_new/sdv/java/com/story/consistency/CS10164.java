package com.story.consistency;

import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Date;

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
* TestLink: seqDB-10164
* @author xiaoni huang init
* @Date   2016.9.26
*/

public class CS10164 {
	private CommLib CommLib = new CommLib();
	private SimpleDateFormat dateFm = new SimpleDateFormat("YYYY-MM-dd HH:mm:ss");
	private static Sequoiadb sdb = null;
	private String domainName = "domain10164";
	private String csName = "cs10164";
	private BSONObject domainOption = new BasicBSONObject();
	private BSONObject csOption = new BasicBSONObject();
	
	@BeforeClass
	@Parameters({"coordAddr"})
	public void setUp(String coordAddr){
		//start time
		System.out.println("Begin to run " + this.getClass().getName() 
					+ ", begin in: " + dateFm.format(new Date().getTime()));
		try{
			sdb = new Sequoiadb(coordAddr, "", "");
			//judge the mode and group number
			if(CommLib.isStandAlone(sdb)){
				throw new SkipException("The mode is standlone, " + "skip the testCase.");
			}
			//drop cs
			CommLib.clearCS(sdb, csName);
			//create domain
			CS10164 dm = new CS10164();
			dm.domain10163(sdb);
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
	public void testCS10164(String coordAddr){
		Sequoiadb db  = null;
		//-----create cs-----
		try
		{
			db = new Sequoiadb(coordAddr, "", "");
			//create domain
			csOption.put( "Domain", domainName );
		    db.createCollectionSpace(csName, csOption);
			//check results of catalog
			CommLib.checkCSOfCatalog(db, csName);
		}catch(BaseException e){
			if(e.getErrorCode() != -33){  //-33:Collection space already exists
				db.disconnect();
				Assert.fail(e.getMessage());
			}
		}
		
		//-----alter domain-----
		try{
			domainOption.put( "AutoSplit", false );
			db.getDomain(domainName).alterDomain(domainOption);
			//check results of catalog
			CommLib.checkDomainOfCatalog(db, domainName);
			CommLib.checkCSOfCatalog(db, csName);
		}catch(BaseException e){
			Assert.fail(e.getMessage());
		}
	}
	
	/**
	 * create domain
	 * @param sdb
	 */
	public void domain10163(Sequoiadb sdb){
		ArrayList<String> dataGroups = new ArrayList<String>();
		BSONObject domainOption = new BasicBSONObject();
		try{
			if(!sdb.isDomainExist(domainName)){
				dataGroups = CommLib.getDataGroupNames(sdb);
				domainOption.put( "Groups", dataGroups );
				domainOption.put( "AutoSplit", true );
			    sdb.createDomain ( domainName, domainOption);
			}
		}catch(BaseException e){
			Assert.fail(e.getMessage());
		}
	}
		
}