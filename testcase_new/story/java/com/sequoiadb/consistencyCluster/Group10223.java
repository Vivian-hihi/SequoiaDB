package com.sequoiadb.consistencyCluster;

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
import com.sequoiadb.consistencyData.CommLib;
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.testcommon.SdbTestBase;

/**
* TestLink: seqDB-10223
* @author xiaoni huang init
* @Date   2016.10.24
*/

public class Group10223 extends SdbTestBase {
	private SimpleDateFormat dateFm = new SimpleDateFormat("YYYY-MM-dd HH:mm:ss");
	private CommLib CommLib = new CommLib();
	private static Sequoiadb sdb = null;
	private static ArrayList<String> dataGroups = null;
	private String rgName = "rg10223";
	private String domainName = "dm10223";
	private Random random = new Random();
	private int number = 10;
	
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
			CommLib.clearGroup(sdb, rgName);
			//ready env
			dataGroups = CommLib.getDataGroupNames(sdb);
			Group10223.this.createDomain(sdb);
			Group10223.this.createGroupAndNode();
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
			CommLib.clearGroup(sdb, rgName);
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
	public void testDropGroup(){
		Sequoiadb db  = null;
		try{
			db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
		}catch(BaseException e){
			Assert.fail(e.getMessage());
		}
		
		try
		{
			db.removeReplicaGroup(rgName + "_" + number);
		}catch(BaseException e){
			if(e.getErrorCode() != -154){  //-154:Group does not exist
				Assert.fail(e.getMessage());
			}
		}finally{
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
			opt.put( "Groups", dataGroups );
			sdb.getDomain(domainName).alterDomain(opt);
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
	
	public void createGroupAndNode(){
		try
		{
			for(int i = 0; i < number; i++){
				String tmpName = rgName + "_" + i;
				sdb.createReplicaGroup(tmpName);
				CommLib.createNode(sdb, tmpName, SdbTestBase.reservedPortBegin, 
						SdbTestBase.reservedPortEnd, SdbTestBase.reservedDir);
				sdb.getReplicaGroup(tmpName).start();
			}
		}catch(BaseException e){
			if(e.getErrorCode() != -153){  //Group already exist
				Assert.fail(e.getMessage());
			}
		}
	}
	
	public void createDomain(Sequoiadb sdb){
		try
		{
			ArrayList<String> dataGroupNames = CommLib.getDataGroupNames(sdb);
			BSONObject opt = new BasicBSONObject();
			opt.put( "Groups", dataGroupNames );
			sdb.createDomain ( domainName, opt);
		}catch(BaseException e){
			Assert.fail(e.getMessage());
		}
	}
	
}