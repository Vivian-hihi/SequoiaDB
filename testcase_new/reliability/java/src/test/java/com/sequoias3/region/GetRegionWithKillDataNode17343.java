package com.sequoias3.region;

import com.amazonaws.AmazonServiceException;
import com.amazonaws.services.s3.AmazonS3;
import com.amazonaws.services.s3.model.CreateBucketRequest;
import com.sequoiadb.commlib.GroupMgr;
import com.sequoiadb.commlib.GroupWrapper;
import com.sequoiadb.commlib.NodeWrapper;
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.exception.ReliabilityException;
import com.sequoiadb.fault.KillNode;
import com.sequoiadb.task.FaultMakeTask;
import com.sequoiadb.task.OperateTask;
import com.sequoiadb.task.TaskMgr;
import com.sequoias3.commlibs3.CommLibS3;
import com.sequoias3.commlibs3.S3TestBase;
import com.sequoias3.commlibs3.s3utils.bean.GetRegionResult;
import com.sequoias3.commlibs3.s3utils.bean.Region;
import com.sequoias3.commlibs3.s3utils.RegionUtils;
import org.testng.Assert;
import org.testng.SkipException;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

/**
 * test content: 获取区域过程中db端节点异常 
 * testlink-case: seqDB-17343
 * @author wangkexin
 * @Date 2019.01.29
 * @version 1.00
 */
public class GetRegionWithKillDataNode17343 extends S3TestBase {
	private GroupMgr groupMgr = null;
	private String bucketName = "bucket17343";
	private String regionName = "Beijing17343";
	private String dataDomain = "dataDomain17343";
	private String metaDomain = "metaDomain17343";
	private String dataGroupName = null;
	private AmazonS3 s3Client = null;
	private boolean runSuccess = false;

	@BeforeClass
	private void setUp() throws Exception {
		s3Client = CommLibS3.buildS3Client();
		groupMgr = GroupMgr.getInstance();
		//TODO:可以不用检测
		if (!groupMgr.checkBusiness()) {
			throw new SkipException("checkBusiness failed");
		}
		dataGroupName = groupMgr.getAllDataGroupName().get(0);
		
		RegionUtils.dropDomain(metaDomain);
		RegionUtils.dropDomain(dataDomain);

		if(RegionUtils.headRegion(regionName)){
			RegionUtils.deleteRegion(regionName);
		}
		
		RegionUtils.createDomain(dataDomain);
		RegionUtils.createDomain(metaDomain);
		
		Region region = new Region();
        region.withName(regionName)
        .withDataDomain(dataDomain)
        .withMetaDomain(metaDomain)
        .withDataCSShardingType("quarter")  //TODO:"quarter"和"month"多处使用，建议使用变量
        .withDataCLShardingType("month");
        RegionUtils.putRegion(region);
		s3Client.createBucket(new CreateBucketRequest(bucketName, regionName.toLowerCase()));
	}

	@Test
	public void testGetRegion() throws Exception {
		try {
			GroupWrapper dataGroup = groupMgr.getGroupByName(dataGroupName);
			NodeWrapper priNode = dataGroup.getMaster();
            //TODO:需要强杀集群中所有的主节点
			FaultMakeTask faultTask = KillNode.getFaultMakeTask(priNode.hostName(), priNode.svcName(), 1);
			TaskMgr mgr = new TaskMgr(faultTask);
			//TODO：文本用例中是并发获取区域，请跟写文本用例的人确认一下
			GetRegionTask gTask = new GetRegionTask();
			mgr.addTask(gTask);
			mgr.execute();
			Assert.assertEquals(mgr.isAllSuccess(), true, mgr.getErrorMsg());

			// check whether the cluster is normal and lsn consistency ,the
			// longest waiting time is 600S
			//TODO:可以不用检测
			Assert.assertEquals(groupMgr.checkBusinessWithLSN(600), true, "checkBusinessWithLSN() occurs timeout");

			GetRegionResult result = RegionUtils.getRegion(regionName);
			checkRegion(result);
		} catch (ReliabilityException e) {
			//TODO：非预期异常抛出去，不要进行捕获
			e.printStackTrace();
			Assert.fail(e.getMessage());
		}
		runSuccess = true;
	}

	@AfterClass
	private void tearDown() throws Exception {
		try {
			if (runSuccess) {
				s3Client.deleteBucket(bucketName);
				RegionUtils.dropDomain(metaDomain);
				RegionUtils.dropDomain(dataDomain);
				RegionUtils.deleteRegion(regionName);
			}
		} catch (BaseException e) {
			//TODO：非预期异常抛出去，不要进行捕获
			Assert.fail("clean up failed:" + e.getMessage());
		} finally {
			if (s3Client != null) {
				s3Client.shutdown();
			}
		}
	}

	private class GetRegionTask extends OperateTask {
		@Override
		public void exec() throws Exception {
			try {
				for (int i = 0; i < 100; i++) {
					GetRegionResult result = RegionUtils.getRegion(regionName);
					checkRegion(result);
				}
			}catch(AmazonServiceException e){
				//TODO：线程内对异常的处理，尽量不要使用Assert.assertEquals，可以将异常抛出去
				Assert.assertEquals(e.getErrorCode(), "GetDBConnectFail");
			} 
		}
	}
	
	private void checkRegion(GetRegionResult result){
		Region region = result.getRegion();
		String actBucket = result.getBuckets().get(0).getName();
		Assert.assertEquals(actBucket, bucketName);
		Assert.assertEquals(region.getName(), regionName.toLowerCase());
		Assert.assertEquals(region.getDataCSShardingType(), "quarter");
		Assert.assertEquals(region.getDataCLShardingType(), "month");
		Assert.assertEquals(region.getDataDomain(), dataDomain);
		Assert.assertEquals(region.getMetaDomain(), metaDomain);
	}
}