package com.sequoias3.config;

import org.testng.Assert;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.DataProvider;
import org.testng.annotations.Test;

import com.amazonaws.services.s3.AmazonS3;
import com.amazonaws.services.s3.model.AmazonS3Exception;
import com.sequoias3.region.Region;
import com.sequoias3.testcommon.CommLib;
import com.sequoias3.testcommon.S3TestBase;
import com.sequoias3.testcommon.s3utils.RegionUtils;

/**
 * test content: 使用指定模式创建区域，带LobPageSize和RepliSize参数testlink-case: seqDB-18610
 * 
 * @author wangkexin
 * @Date 2019.06.27
 * @version 1.00
 */
public class CreateRegion18610 extends S3TestBase {
	@DataProvider(name = "lobPageSizeAndreplSizeProvider")
	public Object[][] generateAuthorization() {
		return new Object[][] { new Object[] { "4096", "" }, new Object[] { "", "3" }, new Object[] { "8192", "1" } };
	}

	private boolean runSuccess = false;
	private String regionName = "region18610";
	private AmazonS3 s3Client = null;
	private String[] csNames = { "metaCS18610", "dataCS18610" };
	private String[] metaclNames = { "metaCL18610", "metaHistroyCL18610" };
	private String[] dataclNames = { "dataCL18610" };

	@BeforeClass
	private void setUp() throws Exception {
		RegionUtils.createCSAndCL(csNames[0], metaclNames);
		RegionUtils.createCSAndCL(csNames[1], dataclNames);

		s3Client = CommLib.buildS3Client();
		//TODO :1、清理区域建议放在createCS前面
		RegionUtils.clearRegion(regionName);
	}

	@Test(dataProvider = "lobPageSizeAndreplSizeProvider")
	public void testRegion(String dataLobPageSize, String dataReplSize) throws Exception {
		Region region = new Region();
		String metaLocation = csNames[0] + "." + metaclNames[0];
		String metaHisLocation = csNames[0] + "." + metaclNames[1];
		String dataLocation = csNames[1] + "." + dataclNames[0];
		region.withMetaLocation(metaLocation).withDataLocation(dataLocation).withMetaHisLocation(metaHisLocation)
				.withName(regionName).withDataLobPageSize(dataLobPageSize).withDataReplSize(dataReplSize);
		try {
			RegionUtils.putRegion(region);//TODO: 2、如果是必定失败，建议增加成功报错处理
		} catch (AmazonS3Exception e) {
			Assert.assertEquals(e.getErrorCode(), "ConflictRegionType");
		}

		runSuccess = true;
	}

	@AfterClass
	private void tearDown() throws Exception {
		try {
			if (runSuccess) {
				RegionUtils.dropCS(csNames);
			}
		} finally {
			s3Client.shutdown();
		}
	}
}
