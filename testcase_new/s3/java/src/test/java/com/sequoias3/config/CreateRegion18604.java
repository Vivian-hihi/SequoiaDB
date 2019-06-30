package com.sequoias3.config;

import java.util.List;

import org.testng.Assert;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import com.amazonaws.services.s3.model.AmazonS3Exception;
import com.sequoias3.region.Region;
import com.sequoias3.testcommon.CommLib;
import com.sequoias3.testcommon.S3TestBase;
import com.sequoias3.testcommon.s3utils.ConfigUtils;
import com.sequoias3.testcommon.s3utils.RegionUtils;
import com.sequoias3.testcommon.s3utils.UserUtils;
import com.sequoias3.user.UserCommDefind;

/**
 * test content: 开启鉴权，使用普通用户执行区域管理操作 testlink-case: seqDB-18604
 * 
 * @author wangkexin
 * @Date 2019.06.24
 * @version 1.00
 */
public class CreateRegion18604 extends S3TestBase {
	private boolean runSuccess = false;
	private String regionName = "region18604";
	private String[] csNames = { "metaCS18604", "dataCS18604" };
	private String[] metaclNames = { "metaCL18604", "metaHistroyCL18604" };
	private String[] dataclNames = { "dataCL18604" };
	private String[] domainNames = { "domain18604A", "domain18604B" };
	private String userName = "normaluser18604";
	private String roleName = "normal";
	private String[] accessKeys = null;

	@BeforeClass
	private void setUp() throws Exception {
		CommLib.clearUser(userName);
		accessKeys = UserUtils.createUser(userName, roleName);
		RegionUtils.clearRegion(regionName);
	}

	@Test
	private void testCreateRegionV2() throws Exception {
		runSuccess = false;
		String authorizationV2 = "AWS " + accessKeys[0] + ":signature";

		RegionUtils.createCSAndCL(csNames[0], metaclNames);
		RegionUtils.createCSAndCL(csNames[1], dataclNames);
		// create region
		Region region = new Region();
		String metaLocation = csNames[0] + "." + metaclNames[0];
		String metaHisLocation = csNames[0] + "." + metaclNames[1];
		String dataLocation = csNames[1] + "." + dataclNames[0];
		region.withMetaLocation(metaLocation).withDataLocation(dataLocation).withMetaHisLocation(metaHisLocation)
				.withName(regionName);

		try {
			ConfigUtils.putRegion(region, authorizationV2);
			Assert.fail("create region " + regionName + " by normal user should be failed!");
		} catch (AmazonS3Exception e) {
			Assert.assertEquals(e.getErrorCode(), "AccessDenied");
		}

		try {
			ConfigUtils.headRegion(regionName, authorizationV2);
			Assert.fail("head region " + regionName + " by normal user should be failed!");
		} catch (AmazonS3Exception e) {
			Assert.assertEquals(e.getStatusCode(), 403);
		}

		try {
			ConfigUtils.getRegion(regionName, authorizationV2);
			Assert.fail("get region " + regionName + " by normal user should be failed!");
		} catch (AmazonS3Exception e) {
			Assert.assertEquals(e.getErrorCode(), "AccessDenied");
		}

		// 所有用户都可查询区域列表
		List<String> regions = ConfigUtils.listRegions(authorizationV2);
		Assert.assertFalse(regions.contains(regionName));

		try {
			ConfigUtils.deleteRegion(regionName, authorizationV2);
			Assert.fail("delete region " + regionName + " by normal user should be failed!");
		} catch (AmazonS3Exception e) {
			Assert.assertEquals(e.getErrorCode(), "AccessDenied");
		}
		runSuccess = true;
	}

	@Test
	private void testCreateRegionV4() throws Exception {
		runSuccess = false;
		String authorizationV4 = UserCommDefind.authValPre + accessKeys[0] + "/";

		for (String domainName : domainNames) {
			RegionUtils.createDomain(domainName);
		}
		// create region
		Region region = new Region();
		region.withDataCSShardingType("year").withDataCLShardingType("month").withDataDomain(domainNames[0])
				.withMetaDomain(domainNames[1]).withName(regionName);

		try {
			ConfigUtils.putRegion(region, authorizationV4);
			Assert.fail("create region " + regionName + " by normal user should be failed!");
		} catch (AmazonS3Exception e) {
			Assert.assertEquals(e.getErrorCode(), "AccessDenied");
		}

		try {
			ConfigUtils.headRegion(regionName, authorizationV4);
			Assert.fail("head region " + regionName + " by normal user should be failed!");
		} catch (AmazonS3Exception e) {
			Assert.assertEquals(e.getStatusCode(), 403);
		}

		try {
			ConfigUtils.getRegion(regionName, authorizationV4);
			Assert.fail("get region " + regionName + " by normal user should be failed!");
		} catch (AmazonS3Exception e) {
			Assert.assertEquals(e.getErrorCode(), "AccessDenied");
		}

		// 所有用户都可查询区域列表
		List<String> regions = ConfigUtils.listRegions(authorizationV4);
		Assert.assertFalse(regions.contains(regionName));

		try {
			ConfigUtils.deleteRegion(regionName, authorizationV4);
			Assert.fail("delete region " + regionName + " by normal user should be failed!");
		} catch (AmazonS3Exception e) {
			Assert.assertEquals(e.getErrorCode(), "AccessDenied");
		}
		runSuccess = true;
	}

	@AfterClass
	private void tearDown() throws Exception {
		if (runSuccess) {
			RegionUtils.dropCS(csNames);
			UserUtils.deleteUser(userName);
		}
	}
}
