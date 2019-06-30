package com.sequoias3.config;

import java.io.File;
import java.util.List;

import org.testng.Assert;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import com.amazonaws.services.s3.AmazonS3;
import com.sequoias3.region.GetRegionResult;
import com.sequoias3.region.Region;
import com.sequoias3.testcommon.CommLib;
import com.sequoias3.testcommon.S3TestBase;
import com.sequoias3.testcommon.TestTools;
import com.sequoias3.testcommon.s3utils.ConfigUtils;
import com.sequoias3.testcommon.s3utils.ObjectUtils;
import com.sequoias3.testcommon.s3utils.RegionUtils;
import com.sequoias3.testcommon.s3utils.UserUtils;
import com.sequoias3.user.UserCommDefind;

/**
 * test content: 开启鉴权，执行区域管理操作 testlink-case: seqDB-18590
 * 
 * @author wangkexin
 * @Date 2019.06.24
 * @version 1.00
 */
public class CreateRegion18590 extends S3TestBase {
	private boolean runSuccess = false;
	private String regionName1 = "region18590v2";
	private String regionName2 = "region18590v4";
	private String bucketName = "bucket18590";
	private String keyName = "key18590";
	private String[] csNames = { "metaCS18590", "dataCS18590" };
	private String[] metaclNames = { "metaCL18590", "metaHistroyCL18590" };
	private String[] dataclNames = { "dataCL18590" };
	private String[] domainNames = { "domain18590A", "domain18590B" };
	private String authorizationV2 = "AWS " + UserUtils.accessKeyId + ":signature";
	private String authorizationV4 = UserCommDefind.authValPre + UserUtils.accessKeyId + "/";
	private int fileSize = 1024 * 1024 * 3;
	private File localPath = null;
	private String filePath = null;
	private AmazonS3 s3Client = null;

	@BeforeClass
	private void setUp() throws Exception {
		localPath = new File(S3TestBase.workDir + File.separator + TestTools.getClassName());
		filePath = localPath + File.separator + "localFile_" + fileSize + ".txt";

		TestTools.LocalFile.removeFile(localPath);
		TestTools.LocalFile.createDir(localPath.toString());
		TestTools.LocalFile.createFile(filePath, fileSize);

		s3Client = CommLib.buildS3Client();
		CommLib.clearBucket(s3Client, bucketName);
		RegionUtils.clearRegion(regionName1);
		RegionUtils.clearRegion(regionName2);
	}

	@Test
	private void testCreateRegionV2() throws Exception {
		runSuccess = false;
		RegionUtils.createCSAndCL(csNames[0], metaclNames);
		RegionUtils.createCSAndCL(csNames[1], dataclNames);
		// create region
		Region region = new Region();
		String metaLocation = csNames[0] + "." + metaclNames[0];
		String metaHisLocation = csNames[0] + "." + metaclNames[1];
		String dataLocation = csNames[1] + "." + dataclNames[0];
		region.withMetaLocation(metaLocation).withDataLocation(dataLocation).withMetaHisLocation(metaHisLocation)
				.withName(regionName1);
		ConfigUtils.putRegion(region, authorizationV2);
		// get region and check region info
		checkRegionV2(metaLocation, metaHisLocation, dataLocation, authorizationV2);
		// create object on region
		createObjectAndCheckResult(regionName1);

		// delete region and clear environment
		CommLib.clearBucket(s3Client, bucketName);
		ConfigUtils.deleteRegion(regionName1, authorizationV2);
		runSuccess = true;
	}

	@Test
	private void testCreateRegionV4() throws Exception {
		runSuccess = false;
		for (String domainName : domainNames) {
			RegionUtils.createDomain(domainName);
		}
		// create region
		Region region = new Region();
		region.withDataCSShardingType("year").withDataCLShardingType("month").withDataDomain(domainNames[0])
				.withMetaDomain(domainNames[1]).withName(regionName2);
		ConfigUtils.putRegion(region, authorizationV4);

		// get region and check region info
		checkGetResultV4(authorizationV4);
		// create object on region
		createObjectAndCheckResult(regionName2);

		CommLib.clearBucket(s3Client, bucketName);
		ConfigUtils.deleteRegion(regionName2, authorizationV4);
		runSuccess = true;
	}

	@AfterClass
	private void tearDown() throws Exception {
		try {
			if (runSuccess) {
				RegionUtils.dropCS(csNames);
				for (String domainName : domainNames) {
					RegionUtils.dropDomain(domainName);
				}
				TestTools.LocalFile.removeFile(localPath);
			}
		} finally {
			if (s3Client != null) {
				s3Client.shutdown();
			}
		}
	}

	private void checkRegionV2(String metaLocation, String metaHisLocation, String dataLocation, String authorization)
			throws Exception {
		boolean headRegion = ConfigUtils.headRegion(regionName1, authorization);
		Assert.assertTrue(headRegion, "region should exist.");

		GetRegionResult result = ConfigUtils.getRegion(regionName1, authorization);
		Region regionInfo = result.getRegion();
		Assert.assertEquals(regionInfo.getMetaLocation(), metaLocation);
		Assert.assertEquals(regionInfo.getMetaHisLocation(), metaHisLocation);
		Assert.assertEquals(regionInfo.getDataLocation(), dataLocation);

		List<String> regions = ConfigUtils.listRegions(authorization);
		Assert.assertTrue(regions.contains(regionName1));
	}

	private void checkGetResultV4(String authorization) throws Exception {
		boolean headRegion = ConfigUtils.headRegion(regionName2, authorization);
		Assert.assertTrue(headRegion, "region should exist.");

		GetRegionResult result = ConfigUtils.getRegion(regionName2, authorization);
		// expResult
		Region expRegion = new Region().withDataCSShardingType("year").withDataCLShardingType("month")
				.withDataDomain(domainNames[0]).withMetaDomain(domainNames[1]).withName(regionName2)
				.withDataLocation("").withMetaHisLocation("").withMetaLocation("");

		Region region = result.getRegion();
		Assert.assertEquals(region.toString(), expRegion.toString(),
				"region = " + region.toString() + ",expRegion = " + expRegion.toString());

		List<String> regions = ConfigUtils.listRegions(authorization);
		Assert.assertTrue(regions.contains(regionName2));
	}

	@SuppressWarnings("deprecation")
	private void createObjectAndCheckResult(String regionName) throws Exception {
		s3Client.createBucket(bucketName, regionName);
		s3Client.putObject(bucketName, keyName, new File(filePath));
		String downfileMd5 = ObjectUtils.getMd5OfObject(s3Client, localPath, bucketName, keyName);
		Assert.assertEquals(downfileMd5, TestTools.getMD5(filePath));
	}
}
