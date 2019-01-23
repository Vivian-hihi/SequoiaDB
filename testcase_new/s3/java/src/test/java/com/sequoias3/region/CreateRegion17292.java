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
 * @Description seqDB-17292: create Region and specify incomplete configuartion
 *              information, eg:only specify metaLocation and dataLocation.
 * @author wuyan
 * @Date 2019.1.18
 * @version 1.00
 */
public class CreateRegion17292 extends S3TestBase {
	private boolean runSuccess = false;
	private String regionName = "region17292";
	private AmazonS3 s3Client = null;
	private String[] csNames = { "metaCS17292", "dataCS17292" };
	private String[] dataclNames = { "metaCL17291", "metaHistroyCL17291" };
	private String[] metaclNames = { "dataCL17291" };

	@BeforeClass
	private void setUp() {
		RegionUtils.createCSAndCL(csNames[0], dataclNames);
		RegionUtils.createCSAndCL(csNames[1], metaclNames);
		s3Client = CommLib.buildS3Client();
	}

	@Test
	public void testRegion() throws Exception {
		try {
			Region region = new Region();
			String metaLocation = "metaCS17291.metaCL17291";
			String dataLocation = "dataCS17291.dataCL17291";
			region.withMetaLocation(metaLocation).withDataLocation(dataLocation).withName(regionName);
			RegionUtils.putRegion(region);
			Assert.fail("put region must be fail!");
		} catch (AmazonS3Exception e) {
			// return 400:InvalidLocation
			Assert.assertEquals(e.getStatusCode(), 400);
			Assert.assertEquals(e.getErrorCode(), "InvalidLocation");
		}
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
