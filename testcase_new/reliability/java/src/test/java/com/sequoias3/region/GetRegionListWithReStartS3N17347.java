package com.sequoias3.region;

import com.amazonaws.services.s3.AmazonS3;
import com.amazonaws.services.s3.model.CreateBucketRequest;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.commlib.SdbTestBase;
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.task.FaultMakeTask;
import com.sequoiadb.task.OperateTask;
import com.sequoiadb.task.TaskMgr;
import com.sequoias3.commlibs3.CommLibS3;
import com.sequoias3.commlibs3.S3TestBase;
import com.sequoias3.commlibs3.s3utils.RegionUtils;
import com.sequoias3.commlibs3.s3utils.S3NodeRestart;
import com.sequoias3.commlibs3.s3utils.bean.Region;
import com.sequoias3.commlibs3.s3utils.bean.S3NodeWrapper;
import org.testng.Assert;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

/**
 * test content: 获取区域列表过程中sequoiaS3端节点异常 testlink-case: seqDB-17347
 * 
 * @author wangkexin
 * @Date 2019.05.10
 * @version 1.00
 */

public class GetRegionListWithReStartS3N17347 extends S3TestBase {
	private String regionName = "Beijing17347";
	private String bucketName = "bucket17347";
	private String metaCSName = "metaCS17347";
	private String dataCSName = "dataCS17347";
	private String[] metaClNames = {"metaCL17347", "metaHistoryCL17347"};
	private String[] dataClName = {"dataCL17347"};
	private List<String> regionNames = new ArrayList<>();
	private int regionNum = 10000;
	private int bucketNum = 80;
	private static Sequoiadb sdb = null;
	private AmazonS3 s3Client = null;
	private boolean runSuccess = false;

	@BeforeClass
	private void setUp() throws Exception {
		s3Client = CommLibS3.buildS3Client();
		sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "");

		RegionUtils.createCSAndCL(metaCSName, metaClNames);
		RegionUtils.createCSAndCL(dataCSName, dataClName);

		clearBuckets();

		for (int i = 0; i < regionNum; i++) {
			String currRegionName = regionName + "-" + i;
			regionNames.add(currRegionName.toLowerCase());
			if (RegionUtils.headRegion(currRegionName)) {
				RegionUtils.deleteRegion(currRegionName);
			}
		}

		// create regions
		for (int i = 0; i < regionNum; i++) {
			Region region = new Region();
			region.withName(regionNames.get(i)).withMetaLocation(metaCSName + "." + metaClNames[0])
					.withMetaHisLocation(metaCSName + "." + metaClNames[1])
					.withDataLocation(dataCSName + "." + dataClName[0]);
			RegionUtils.putRegion(region);
			if (i < bucketNum) {
				s3Client.createBucket(new CreateBucketRequest(bucketName + i, regionNames.get(i).toLowerCase()));
			}
		}
	}

	@Test
	public void testCreateRegion() throws Exception {
		FaultMakeTask faultMakeTask = S3NodeRestart.getFaultMakeTask(new S3NodeWrapper(), 1, 10);
		TaskMgr mgr = new TaskMgr(faultMakeTask);
		mgr.addTask(new GetRegionList());
		mgr.execute();
		mgr.isAllSuccess();
		List<Exception> eList = mgr.getExceptions();
		for (Exception e : eList) {
			if (!e.getMessage().contains("Connection refused")) {
				throw e;
			}
		}
		// list again
		List<String> actRegions = RegionUtils.listRegions();
		checkListResult(actRegions);
		runSuccess = true;
	}

	@AfterClass
	private void tearDown() throws Exception {
		try {
			if (runSuccess) {
				clearBuckets();
				sdb.dropCollectionSpace(metaCSName);
				sdb.dropCollectionSpace(dataCSName);
				deleteRegions(regionNames);
				sdb.close();
			}
		} catch (BaseException e) {
			Assert.fail("clean up failed:" + e.getMessage());
		} finally {
			if (s3Client != null) {
				s3Client.shutdown();
			}
		}
	}


	private class GetRegionList extends OperateTask {
		@Override
		public void exec() throws Exception {
			List<String> actRegions = RegionUtils.listRegions();
			checkListResult(actRegions);
		}
	}

	private void checkListResult(List<String> actRegions) {
		Set<String> unRepeatRegionNames = new HashSet<>();
		for (String regionName : actRegions) {
			boolean isRepeat = unRepeatRegionNames.add(regionName);
			Assert.assertTrue(isRepeat, "the region name " + regionName + " is repeated!");
		}
		Assert.assertTrue(actRegions.containsAll(regionNames));
	}

	private void clearBuckets() {
		for (int i = 0; i < bucketNum; i++) {
			CommLibS3.clearBucket(s3Client, bucketName + i);
		}
	}

	private void deleteRegions(List<String> regions) throws Exception {
		for (int i = 0; i < regions.size(); i++) {
			if (RegionUtils.headRegion(regions.get(i))) {
				RegionUtils.deleteRegion(regions.get(i));
			}
		}
	}
}
