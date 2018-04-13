package com.sequoiadb.cappedCL.killnode;

import java.text.SimpleDateFormat;
import java.util.Date;

import org.bson.BSONObject;
import org.bson.BasicBSONObject;
import org.bson.util.JSON;
import org.testng.Assert;
import org.testng.SkipException;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import com.sequoiadb.base.CollectionSpace;
import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.cappedCL.Utils;
import com.sequoiadb.commlib.GroupMgr;
import com.sequoiadb.commlib.GroupWrapper;
import com.sequoiadb.commlib.NodeWrapper;
import com.sequoiadb.commlib.SdbTestBase;
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.exception.ReliabilityException;
import com.sequoiadb.fault.KillNode;
import com.sequoiadb.task.FaultMakeTask;
import com.sequoiadb.task.OperateTask;
import com.sequoiadb.task.TaskMgr;

/**
 * @FileName seqDB-11816: insert and pop records in capped CL when primary node is killed
 * @Author liuxiaoxuan
 * @Date 2017-10-16
 */
public class CurdCappedCLAndKillMasterNode11816 extends SdbTestBase{

	private GroupMgr groupMgr = null;
	private Sequoiadb sdb = null;
	private boolean isClearEnv = false;
	private DBCollection cappedCL = null;
	private String cappedcsName_11816 = "story_killNode_cappedcs_11816";
	private String cappedclName_11816 = "killNode_cappedcl_11816";
	private String dataGroupName = null;
	private int insertNums = 10000;
	private final int strLength = 968;
	
	@BeforeClass
	public void setup() {
		System.out.println(this.getClass().getName() + " begin at:"
                + new SimpleDateFormat("YYYY-MM-dd HH:mm:ss.SSS").format(new Date()));
		try {
			groupMgr = new GroupMgr();
			//check environment for 120s
			if(!groupMgr.checkBusiness(120)) {
				throw new SkipException("checkBusiness failed");
			}
			sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "");
			dataGroupName = groupMgr.getAllDataGroupName().get(0);
			System.out.println("group: " + dataGroupName);
			//create cl
			createCappedCL();
			//init insert 
     	    Utils.insertRecords(cappedCL, insertNums, strLength);
		}catch (Exception e) {
			Assert.fail(this.getClass().getName() + "setUp error, detail: " + e.getMessage());
		}
	}
	
	@Test
	public void curdAndKillNodeTest() {
		try {
			GroupWrapper dataGroup = groupMgr.getGroupByName(dataGroupName);
			NodeWrapper priNode = dataGroup.getMaster();
			
			FaultMakeTask faultMakeTask = KillNode.getFaultMakeTask(priNode.hostName(), priNode.svcName(), 5);
			
			TaskMgr taskMgr = new TaskMgr(faultMakeTask ,new InsertTask(),new PopTask());
			taskMgr.execute();
			
			//TaskMgr check if there is any exception
			Assert.assertEquals(taskMgr.isAllSuccess(), true, taskMgr.getErrorMsg());
			
			//check whether the cluster is normal and lsn consistency ,the longest waiting time is 20 minutes
	        Assert.assertEquals(groupMgr.checkBusinessWithLSN(1200), true, "check LSN consistency fail");
	         
	        //check data consistency
	        Assert.assertEquals(dataGroup.checkInspect(60), true, "data is different on " + dataGroup.getGroupName());
	         
	        //Normal operating environment
	        isClearEnv = true; 
	        
		} catch (ReliabilityException e) {
			// TODO Auto-generated catch block
			Assert.fail("test reliability Exception: " + e.getMessage());
		}
		
	}
	
	@AfterClass
	public void tearDown() {
		try {
			if(isClearEnv) {
				sdb.dropCollectionSpace(cappedcsName_11816);
			}
		}catch (BaseException e) {
			Assert.fail("teardown failed: " + e.getMessage());
		}finally {
			if(sdb != null) {
				sdb.close();
				System.out.println(this.getClass().getName() + " end at:"
	                    + new SimpleDateFormat("YYYY-MM-dd HH:mm:ss.SSS").format(new Date()));
			}
		}
	}
	
	private void createCappedCL() {
	    try {
	    	//create capped cs
			BSONObject csOptions = new BasicBSONObject();
			csOptions.put("Capped", true);
			CollectionSpace cappedCS = sdb.createCollectionSpace(cappedcsName_11816,csOptions);
			
			//create capped cl
			BSONObject clOptions = new BasicBSONObject();
			clOptions.put("Capped", true);
			clOptions.put("Size", 8096);
			clOptions.put("AutoIndexId", false);
			clOptions.put("Group", dataGroupName);
			cappedCL = cappedCS.createCollection(cappedclName_11816,clOptions);
		}catch (BaseException e) {
			Assert.fail("create capped cl failed, detail:" + e.getMessage());
		}   
		
	}
	
	private class InsertTask extends OperateTask{

		@Override
		public void exec() throws Exception {
           try {
        	   Sequoiadb db = new Sequoiadb(SdbTestBase.coordUrl,"","");
        	   CollectionSpace cs = db.getCollectionSpace(cappedcsName_11816);
        	   DBCollection cl = cs.getCollection(cappedclName_11816);
        	   
        	   //insert
        	   insertNums = 32768;
        	   Utils.insertRecords(cl, insertNums, strLength);
           }catch (BaseException e) {
        	   System.out.println("insert ending , master node is disconnected");
		   }
		}
		
	}
	
	private class PopTask extends OperateTask{

		@Override
		public void exec() throws Exception {
           try {
        	   Sequoiadb db = new Sequoiadb(SdbTestBase.coordUrl,"","");
        	   db.setSessionAttr((BSONObject)JSON.parse("{PreferedInstance:'M'}"));
        	   CollectionSpace cappedCS = db.getCollectionSpace(cappedcsName_11816);
        	   DBCollection cappedCL = cappedCS.getCollection(cappedclName_11816);
        	   
        	   //pop 
        	   long skip = 1;
        	   long logicalID = Utils.getLogicalID(cappedCL,skip);
        	   int direction = -1;
        	   final int INVALID_LOGICALID = -1;
        	   if(INVALID_LOGICALID != logicalID) {
        		   Utils.pop(cappedCL, logicalID, direction);
        	   }
           }catch (BaseException e) {
        	   System.out.println("pop ending , master node is disconnected");
		   }
		}
		
	}
}
