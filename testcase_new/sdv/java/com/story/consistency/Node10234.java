package com.story.consistency;

import java.text.SimpleDateFormat;
import java.util.Date;

import org.testng.annotations.Test;
import org.testng.annotations.Parameters;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.AfterClass;
import org.testng.Assert;
import org.testng.SkipException;

import com.sequoiadb.base.ReplicaGroup;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.exception.BaseException;
import com.story.consistency.CommLib;

/**
* TestLink: seqDB-10234
* @author xiaoni huang init
* @Date   2016.10.24
*/

public class Node10234 {
	private CommLib CommLib = new CommLib();
	private SimpleDateFormat dateFm = new SimpleDateFormat("YYYY-MM-dd HH:mm:ss");
	private static Sequoiadb sdb = null;
	private String rgName = "group10234";
	
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
			//clear group
			CommLib.clearGroup(sdb, rgName);
			//ready group/node
			Node10234.this.createGroupAndNode(sdb, SPAREPORTSTART, SPAREPORTSTOP, SPAREPATH);
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
	public void testNode10234(String coordAddr){
		Sequoiadb db = null;
		ReplicaGroup rgDB = null;
		String hostName = null;
		int svcName = 0;
		
		//-----detach CL-----
		try
		{
			db = new Sequoiadb(coordAddr, "", "");
			rgDB = db.getReplicaGroup(rgName);
			hostName = rgDB.getSlave().getHostName();
			svcName = rgDB.getSlave().getPort();
			
			rgDB.detachNode(hostName, svcName, null);
		}catch(BaseException e){
			if(e.getErrorCode() != -204 && e.getErrorCode() != -155){  
				//-204:Unable to remove the last node or primary in a group
				//-155:Node does not exist (node has been to detach)
				Assert.fail(e.getMessage());
			}
		}

		//-----attach CL-----
		try
		{
			rgDB.attachNode(hostName, svcName, null);
		}catch(BaseException e){
			if(e.getErrorCode() != -157){  
				//-157:Invalid node configuration  (node has been to attach)
				Assert.fail(e.getMessage());
			}
		}
	}
	
	public void createGroupAndNode(Sequoiadb sdb, 
			   int startPort, 
			   int endPort, 
			   String nodePath){
		try{
			sdb.createReplicaGroup(rgName);
			//create 7 node
			CommLib.createNode(sdb, rgName, startPort, endPort, nodePath);
			CommLib.createNode(sdb, rgName, startPort, endPort, nodePath);
			CommLib.createNode(sdb, rgName, startPort, endPort, nodePath);
			CommLib.createNode(sdb, rgName, startPort, endPort, nodePath);
			CommLib.createNode(sdb, rgName, startPort, endPort, nodePath);
			CommLib.createNode(sdb, rgName, startPort, endPort, nodePath);
			CommLib.createNode(sdb, rgName, startPort, endPort, nodePath);
			//start
			sdb.getReplicaGroup(rgName).start();
		}catch(BaseException e){
			Assert.fail(e.getMessage());
		}
	}
	
}