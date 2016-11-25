package com.story.consistencyCluster;

import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.Random;

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
* TestLink: seqDB-10221: concurrency[remove group]
* @author xiaoni huang init
* @Date   2016.10.24
*/

public class Group10222 extends SdbTestBase {
	private SimpleDateFormat dateFm = new SimpleDateFormat("YYYY-MM-dd HH:mm:ss");
	private CommLib CommLib = new CommLib();
	private static Sequoiadb sdb = null;
	private String rgName = "rg10222";
	private Random random = new Random();
	private int number = 30;
	
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
			CommLib.clearGroup(sdb, rgName);
			//ready env
			Group10222.this.createGroup(sdb);
		}catch(BaseException e){
			Assert.fail("Failed to prepare env at th begining. "
					+ "ErrorMsg:\n" +e.getMessage());
		}
		
	}
	
	@AfterClass
	public void tearDown(){
		try{
			//clear env
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
	public void testRemoveGroup(){
		Sequoiadb db  = null;
		try{
			db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
		}catch(BaseException e){
			Assert.fail(e.getMessage());
		}
		
		try
		{
			db.removeReplicaGroup(rgName + random.nextInt(number));
		}catch(BaseException e){
			if(e.getErrorCode() != -154){  //-154:Group does not exist
				Assert.fail(e.getMessage());
			}
		}
	}
	
	public void createGroup(Sequoiadb sdb){
		try
		{
			for(int i = 0; i < number; i++){
				sdb.createReplicaGroup(rgName + i);
			}
		}catch(BaseException e){
			Assert.fail(e.getMessage());
		}
	}
	
}