package com.sequoias3.region;

import org.testng.Assert;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import com.amazonaws.services.s3.AmazonS3;
import com.amazonaws.services.s3.model.AmazonS3Exception;
import com.sequoias3.testcommon.CommLib;
import com.sequoias3.testcommon.S3TestBase;
import com.sequoias3.testcommon.s3utils.RegionUtils;

/**
 * @Description seqDB-17311: create Region and specify all config.
 * @author wuyan
 * @Date 2019.1.22
 * @version 1.00
 */
public class CreateRegion17311 extends S3TestBase {
	private boolean runSuccess = false;
	private String regionName = "region17311";
	private AmazonS3 s3Client = null;
	private String[] csNames = { "metaCS17311", "dataCS17311" };
	private String[] metaclNames = { "metaCL17311", "metaHistroyCL17311" };
	private String[] dataclNames = { "dataCL17311" };

	@BeforeClass
	private void setUp() {
		RegionUtils.createCSAndCL(csNames[0], metaclNames);
		RegionUtils.createCSAndCL(csNames[1], dataclNames);
		s3Client = CommLib.buildS3Client();
	}

	@Test
	public void testRegion() throws Exception {
		try {
			Region region = new Region();
			String metaLocation = csNames[0] + "." + metaclNames[0];
			String metaHisLocation = csNames[0] + "." + metaclNames[1];
			String dataLocation = csNames[1] + "." + dataclNames[0];
			region.withMetaLocation(metaLocation).withMetaHisLocation(metaHisLocation).withDataLocation(dataLocation)
					.withDataCLShardingType("month").withDataCSShardingType("year").withName(regionName);
			RegionUtils.putRegion(region);
			Assert.fail("put region must be fail!");
		} catch (AmazonS3Exception e) {
			// return 409:ConflictRegionType
			Assert.assertEquals(e.getStatusCode(), 409);
			Assert.assertEquals(e.getErrorCode(), "ConflictRegionType");
		}

		Assert.assertFalse(RegionUtils.headRegion(regionName), "region should be not exist!");
		runSuccess = true;
	}

	@AfterClass
	private void tearDown() {
		try {
			if (runSuccess) {
				RegionUtils.dropCS(csNames);
			}
		} finally {
			s3Client.shutdown();
		}
	}

}
