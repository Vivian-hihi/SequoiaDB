package com.sequoias3.region;

import com.amazonaws.services.s3.model.AmazonS3Exception;
import com.sequoiadb.commlib.GroupMgr;
import com.sequoiadb.commlib.GroupWrapper;
import com.sequoiadb.commlib.NodeWrapper;
import com.sequoiadb.fault.KillNode;
import com.sequoiadb.task.FaultMakeTask;
import com.sequoiadb.task.OperateTask;
import com.sequoiadb.task.TaskMgr;
import com.sequoias3.commlibs3.S3TestBase;
import com.sequoias3.commlibs3.s3utils.bean.Region;
import com.sequoias3.commlibs3.s3utils.RegionUtils;

import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.CopyOnWriteArrayList;

import org.testng.Assert;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

/**
 * test content: 获取区域过程中db端节点异常 
 * testlink-case: seqDB-17344
 * @author wangkexin
 * @Date 2019.01.29
 * @version 1.00
 */
public class DeleteRegionWithKillCoord17344 extends S3TestBase {
	private GroupMgr groupMgr = null;
	private String regionName = "beijing17344";
	private List<String> regionNames = new ArrayList<String>();
	private List<String> deletedRegionNameList = new CopyOnWriteArrayList<String>();
	private int threadNum = 10;
	private GroupWrapper coordGroup = null;

	@BeforeClass
	private void setUp() throws Exception {
		groupMgr = GroupMgr.getInstance();
		coordGroup = groupMgr.getGroupByName("SYSCoord");
		
		for(int i = 0 ; i < threadNum ; i++){
			String currRegionName = regionName + "-" + i;
			RegionUtils.clearRegion(currRegionName);
			
			Region region = new Region();
		    region.withName(currRegionName);
		    RegionUtils.putRegion(region);
		    regionNames.add(currRegionName);
		}
	}

	@Test
	public void testDeleteRegion() throws Exception {
		TaskMgr mgr = new TaskMgr();
        for(NodeWrapper node : coordGroup.getNodes()) {
            FaultMakeTask faultTask = KillNode.getFaultMakeTask(node, 0);
            mgr.addTask(faultTask);
        }
		
        for(int i = 0 ; i < regionNames.size(); i++){
        	DeleteRegionTask dTask = new DeleteRegionTask(regionNames.get(i));
    		mgr.addTask(dTask);
        }
		
		mgr.execute();
		Assert.assertEquals(mgr.isAllSuccess(), true, mgr.getErrorMsg());

		//delete again
		deleteAgainAndCheck();
	}

	@AfterClass
	private void tearDown() throws Exception {}

	private class DeleteRegionTask extends OperateTask {
		private String regionName = "";
		public DeleteRegionTask(String regionName) {
			this.regionName = regionName;
		}
		@Override
		public void exec() throws Exception {
			try{
				RegionUtils.deleteRegion(regionName);
				deletedRegionNameList.add(regionName);
			}catch(AmazonS3Exception e){
				System.out.println(e.getErrorCode() + ", " + e.getStatusCode());
				if(e.getStatusCode() != 500 && !e.getErrorCode().equals("GetDBConnectFail")){
					throw e;
				}
			}
		}
	}
	
	private void deleteAgainAndCheck() throws Exception{
		List<String> tobeDeleteRegions = new ArrayList<String>();
		tobeDeleteRegions.addAll(regionNames);
		tobeDeleteRegions.removeAll(deletedRegionNameList);
		for (String regionName : tobeDeleteRegions) {
			RegionUtils.deleteRegion(regionName);
		}
		
		for(int i = 0; i < regionNames.size() ; i++){
			Assert.assertFalse(RegionUtils.headRegion(regionNames.get(i)), "current region name is : " + regionNames);
		}
	}
}