package com.story.consistency;

import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Date;
import java.util.Random;

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
* TestLink: seqDB-10157
*			seqDB-10159
*			seqDB-10160
*			seqDB-10162
* @author xiaoni huang init
* @Date   2016.9.23
*/

public class DomainAndRG10157 {
	private CommLib CommLib = new CommLib();
	private SimpleDateFormat dateFm = new SimpleDateFormat("YYYY-MM-dd HH:mm:ss");
	private static Sequoiadb sdb = null;
	private String rgName = "group10157";
	private String domainName = "domain10157";
	BSONObject domainOption = new BasicBSONObject();

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
			//drop replicaGroup
			sdb.removeReplicaGroup(rgName);
		}catch(BaseException e){
			if(e.getErrorCode() != -154){   //-154:Group does not exist
				Assert.fail(e.getMessage());
			}
		}finally{
			System.out.println("End to run " + this.getClass().getName() 
						+ ", end in: " + dateFm.format(new Date().getTime()));
			sdb.disconnect();
		}
	}
	
	@Test(invocationCount = 10, threadPoolSize = 10)
	@Parameters({"coordAddr","SPAREPORTSTART","SPAREPORTSTOP","SPAREPATH"})
	public void testDomain10157(String coordAddr, 
					int SPAREPORTSTART, int SPAREPORTSTOP, String SPAREPATH){
		
		Sequoiadb db = new Sequoiadb(coordAddr, "", "");
		
		//create dataRG and node
		CommLib.createNode(db, rgName, SPAREPORTSTART, SPAREPORTSTOP, SPAREPATH);
		
		//create domain
		ArrayList<String> dataGroups = new ArrayList<String>();
		try
		{
			dataGroups = CommLib.getDataGroupNames(db);
			domainOption.put( "Groups", dataGroups );
			domainOption.put( "AutoSplit", true );
			for(int i = 0; i < 10; i++){
				String tmpName = domainName;
				//random generate domain name
				tmpName = domainName + "_" + randomInt.nextInt(100);
			    db.createDomain ( tmpName, domainOption);
			}
			//check results of catalog
			CommLib.checkDomainOfCatalog(sdb, domainName);
		}catch(BaseException e){
			int eCode = e.getErrorCode();
			if(eCode != -215 && eCode != -90 && eCode != -278){  
				//-215:Domain already exists;  -90:Replication group is not activated;
				//-278:The special group is not data group
				db.disconnect();
				Assert.fail(e.getMessage());
			}
		}
		
		//-----alter domain-----
		try{
			String tmpName = domainName;
			//random generate domain name
			tmpName = domainName + "_" + randomInt.nextInt(100);
			dataGroups.remove(0);
			domainOption.put( "Groups", dataGroups );
			domainOption.put( "AutoSplit", false );
			if(db.isDomainExist(tmpName)){
				db.getDomain(tmpName).alterDomain(domainOption);
			}
			//check results of catalog
			CommLib.checkDomainOfCatalog(sdb, domainName);
		}catch(BaseException e){
			if(e.getErrorCode() != -214){ //-214:Domain does not exist
				db.disconnect();
				Assert.fail(e.getMessage());
			}
		}
		
		//----drop domain-----
		try
		{
			String tmpName = domainName;
			//random generate domain name
			tmpName = domainName + "_" + randomInt.nextInt(100);
			if(db.isDomainExist(tmpName)){
				db.dropDomain(tmpName);
			}
			//check results of catalog
			CommLib.checkDomainOfCatalog(sdb, domainName);
		}catch(BaseException e){
			if(e.getErrorCode() != -214){
				db.disconnect();
				Assert.fail(e.getMessage());
			}
		}
		
		//-----drop dataRG-----
		try
		{
			db.removeReplicaGroup(rgName);
		}catch(BaseException e){
			if(e.getErrorCode() != -154){  //-154:Group does not exist
				Assert.fail(e.getMessage());
			}
		}
		finally{
			db.disconnect();
		}
	}
	
}