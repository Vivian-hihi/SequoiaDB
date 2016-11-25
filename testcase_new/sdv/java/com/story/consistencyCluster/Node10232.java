package com.story.consistencyCluster;

import java.text.SimpleDateFormat;
import java.util.Date;

import org.testng.annotations.Test;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.AfterClass;
import org.testng.Assert;
import org.testng.SkipException;

import com.sequoiadb.base.Node;
import com.sequoiadb.base.ReplicaGroup;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.testcommon.SdbTestBase;
import com.story.consistencyData.CommLib;

/**
* TestLink: seqDB-10232:concurrency[removeNode, dropRG]
* @author xiaoni huang init
* @Date   2016.10.24
*/

public class Node10232 extends SdbTestBase {
	private CommLib CommLib = new CommLib();
	private SimpleDateFormat dateFm = new SimpleDateFormat("YYYY-MM-dd HH:mm:ss");
	private static Sequoiadb sdb = null;
	private String rgName = "group10232";
	
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
			//create group
			sdb.createReplicaGroup(rgName);
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
			Assert.fail(e.getMessage());
		}finally{
			System.out.println("End to run " + this.getClass().getName() 
						+ ", end in: " + dateFm.format(new Date().getTime()));
			sdb.disconnect();
		}
	}
	
	@Test(invocationCount = 10, threadPoolSize = 10)
	public void testNode10232(){
		Sequoiadb db = null;
		//-----remove node-----
		try{
			db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
			
			ReplicaGroup rgDB = db.getReplicaGroup(rgName);
			Node node = rgDB.getSlave();
			String hostName = node.getHostName();
			int svcName = node.getPort();
			rgDB.removeNode(hostName, svcName, null);
		}catch(BaseException e){
			if(e.getErrorCode() != -155){  //-155:Node does not exist
				Assert.fail(e.getMessage());
			}
		}
		
		//-----remove group-----
		try
		{
			db.removeReplicaGroup(rgName);
		}catch(BaseException e){
			if(e.getErrorCode() != -154){ //-154:Group does not exist
				Assert.fail(e.getMessage());
			}
		}
		
		//-----create group-----
		try
		{
			db.createReplicaGroup(rgName);
		}catch(BaseException e){
			if(e.getErrorCode() != -153){ //-153:Group already exist
				Assert.fail(e.getMessage());
			}
		}

		//-----create node-----
		try
		{
			CommLib.createNode(db, 
					   rgName, 
					   SdbTestBase.reservedPortBegin, 
					   SdbTestBase.reservedPortEnd, 
					   SdbTestBase.reservedDir);
	
		}catch(BaseException e){
			Assert.fail(e.getMessage());
		}finally{
			db.disconnect();
		}
	}
	
}