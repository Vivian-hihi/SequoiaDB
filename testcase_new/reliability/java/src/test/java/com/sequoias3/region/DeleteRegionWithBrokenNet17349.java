package com.sequoias3.region;

import com.sequoiadb.commlib.GroupMgr;
import com.sequoiadb.exception.ReliabilityException;
import com.sequoiadb.fault.BrokenNetwork;
import com.sequoiadb.task.FaultMakeTask;
import com.sequoiadb.task.OperateTask;
import com.sequoiadb.task.TaskMgr;
import com.sequoias3.commlibs3.S3TestBase;
import com.sequoias3.commlibs3.s3utils.bean.Region;
import com.sequoias3.commlibs3.s3utils.RegionUtils;
import org.springframework.web.client.ResourceAccessException;
import org.testng.Assert;
import org.testng.SkipException;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

/**
 * test content:  删除区域过程中SequoiaS3和sdb节点网络异常
 * testlink-case: seqDB-17349
 * @author wangkexin
 * @Date 2019.01.29
 * @version 1.00
 */
//TODO:文本用例标题和文本用例步骤有点不相符，请重新确认一下
public class DeleteRegionWithBrokenNet17349 extends S3TestBase {
	private GroupMgr groupMgr = null;
	private String regionName = "beijing17349";
	private int deleteFailedRegionId = 0;

	@BeforeClass
	private void setUp() throws Exception {
		groupMgr = GroupMgr.getInstance();
		//TODO:可以不用检查
		if (!groupMgr.checkBusiness()) {
			throw new SkipException("checkBusiness failed");
		}
		
		for(int i = 0 ; i < 100 ; i++){
			String currRegionName = regionName + "-" + i;
			if(RegionUtils.headRegion(currRegionName)){
				RegionUtils.deleteRegion(currRegionName);
			}
			Region region = new Region();
		    region.withName(currRegionName);
		    RegionUtils.putRegion(region);
		}
	}

	@Test
	public void testDeleteRegion() throws Exception {
		try {
			//kill coord when delete regions
			FaultMakeTask faultTask = BrokenNetwork.getFaultMakeTask(S3TestBase.s3HostName, 10, 10);
			TaskMgr mgr = new TaskMgr(faultTask);
			
			DeleteRegionTask dTask = new DeleteRegionTask();
			mgr.addTask(dTask);
			mgr.execute();
			Assert.assertEquals(mgr.isAllSuccess(), true, mgr.getErrorMsg());

			// check whether the cluster is normal and lsn consistency ,the
			// longest waiting time is 600S
			Assert.assertEquals(groupMgr.checkBusinessWithLSN(600), true, "checkBusinessWithLSN() occurs timeout");

			//delete again
			/*for(int i = deleteFailedRegionId ; i < 100 ; i++){
				RegionUtils.deleteRegion(regionName + "-" + i);
			}
			
			//check delete result
			for(int i = 0; i < 100 ; i++){
				String currRegionName = regionName + "-" + i;
				Assert.assertFalse(RegionUtils.headRegion(currRegionName), "current region name is : " + currRegionName);
			}*/
			
		} catch (ReliabilityException e) {
			e.printStackTrace();
			Assert.fail(e.getMessage());
		}
	}

	@AfterClass
	private void tearDown() throws Exception {}

	private class DeleteRegionTask extends OperateTask {
		@Override
		public void exec() throws Exception {
			int i = 0;
			try{
				for (; i < 100; i++) {
					RegionUtils.deleteRegion(regionName + "-" + i);
				}
			}catch(ResourceAccessException e){
				System.out.println(e.getCause().getMessage());
				if(!e.getCause().getMessage().equals("Read timed out")&&!e.getCause().getMessage().equals("Connection reset")){
					throw e;
				}
				deleteFailedRegionId = i;
			}
		}
	}
}