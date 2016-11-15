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
* TestLink: seqDB-10223
* @author xiaoni huang init
* @Date   2016.10.24
*/

public class Group10223 {
	private CommLib CommLib = new CommLib();
	private SimpleDateFormat dateFm = new SimpleDateFormat("YYYY-MM-dd HH:mm:ss");
	private static Sequoiadb sdb = null;
	private String rgName = "group10223";
	private String domainName = "domain10223";
	
	@BeforeClass
	@Parameters({"coordAddr","SPAREPORTSTART","SPAREPORTSTOP","SPAREPATH"})
	public void setUp(String coordAddr,
			   		  int SPAREPORTSTART, 
			   		  int SPAREPORTSTOP, 
			   		  String SPAREPATH){
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
			//clear env
			CommLib.clearDomain(sdb, domainName);
			CommLib.clearGroup(sdb, rgName);
			//create domain
			Group10223.this.createDomain(sdb);
			//ready group/node
			Group10223.this.createGroupAndNode(sdb, SPAREPORTSTART, SPAREPORTSTOP, SPAREPATH);
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
	@Parameters({"coordAddr"})
	public void testGroup10223(String coordAddr){
		
		Sequoiadb db = null;
		
		//-----drop dataRG-----
		try
		{
			db = new Sequoiadb(coordAddr, "", "");

			String groupName = rgName;
			Random random = new Random();
			db.removeReplicaGroup(groupName + "_" + random.nextInt(10));
		}catch(BaseException e){
			if(e.getErrorCode() != -154){  //-154:Group does not exist
				Assert.fail(e.getMessage());
			}
		}
		
		//-----alter domain-----
		try{
			ArrayList<String> dataGroupNames = CommLib.getDataGroupNames(sdb);
			BSONObject opt = new BasicBSONObject();
			opt.put( "Groups", dataGroupNames );
			sdb.getDomain(domainName).alterDomain(opt);
		}catch(BaseException e){
			if(e.getErrorCode() != -154){  //-154:Group does not exist
				Assert.fail(e.getMessage());
			}
		}finally{
			db.disconnect();
		}
	}
	
	public void createGroupAndNode(Sequoiadb sdb, 
								   int startPort, 
								   int endPort, 
								   String nodePath){
		try
		{
			for(int i = 0; i < 10; i++){
				String groupName = rgName + "_" +  i;
				sdb.createReplicaGroup(groupName);
				CommLib.createNode(sdb, groupName, startPort, endPort, nodePath);
				sdb.getReplicaGroup(groupName).start();
			}
		}catch(BaseException e){
			Assert.fail(e.getMessage());
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