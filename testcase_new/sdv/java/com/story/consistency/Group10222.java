package com.story.consistency;

import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.Random;

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
* TestLink: seqDB-10221
* @author xiaoni huang init
* @Date   2016.10.24
*/

public class Group10222 {
	private CommLib CommLib = new CommLib();
	private SimpleDateFormat dateFm = new SimpleDateFormat("YYYY-MM-dd HH:mm:ss");
	private static Sequoiadb sdb = null;
	private String rgName = "group10222";
	
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
			//clear group
			CommLib.clearGroup(sdb, rgName);
			//create group
			Group10222.this.createGroup(sdb);
		}catch(BaseException e){
			Assert.fail("Failed to prepare env at th begining. "
					+ "ErrorMsg:\n" +e.getMessage());
		}
		
	}
	
	@AfterClass
	public void tearDown(){
		try{
			//clear group
			CommLib.clearGroup(sdb, rgName);
		}catch(BaseException e){
			if(e.getErrorCode() != -154){ //-154:Group does not exist
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
	public void testGroup10222(String coordAddr){
		Sequoiadb db = null;
		
		//-----remove RG-----
		try
		{
			db = new Sequoiadb(coordAddr, "", "");
			//create dataRG 
			String groupName = rgName;
			Random random = new Random();
			db.removeReplicaGroup(groupName + random.nextInt(20));
		}catch(BaseException e){
			if(e.getErrorCode() != -154){  //-154:Group does not exist
				Assert.fail(e.getMessage());
			}
		}
	}
	
	public void createGroup(Sequoiadb sdb){
		try
		{
			for(int i = 0; i < 20; i++){
				sdb.createReplicaGroup(rgName + i);
			}
		}catch(BaseException e){
			Assert.fail(e.getMessage());
		}
	}
	
}