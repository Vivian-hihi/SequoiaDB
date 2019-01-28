package com.sequoias3.region;

import java.util.List;

import org.testng.Assert;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import com.amazonaws.services.s3.AmazonS3;
import com.amazonaws.services.s3.model.Bucket;
import com.sequoiadb.base.Sequoiadb;
import com.sequoias3.testcommon.CommLib;
import com.sequoias3.testcommon.S3TestBase;
import com.sequoias3.testcommon.s3utils.RegionUtils;

/**
 * test content: 获取空区域信息
 * testlink-case: seqDB-17321
 * @author wangkexin
 * @Date 2019.01.24
 * @version 1.00
 */

public class GetRegionMessage17321 extends S3TestBase{
	private AmazonS3 s3Client = null;
	private String regionName = "beijing17321";
	private static Sequoiadb sdb = null;
	private boolean runSuccess = false;

	@BeforeClass
	private void setUp() throws Exception {
		s3Client = CommLib.buildS3Client();
		sdb = new Sequoiadb(S3TestBase.coordUrl, "", "");
		
		if(RegionUtils.headRegion(regionName)){
			RegionUtils.deleteRegion(regionName);
		}
	}
	
	@Test
	public void testGetRegionMessage() throws Exception {
		// create region
		Region region = new Region();
		region.withName(regionName);
		RegionUtils.putRegion(region);

		GetRegionResult result = RegionUtils.getRegion(regionName);
		List<Bucket> bucketList = result.getBuckets();
		Assert.assertEquals(bucketList.size(),0);

		runSuccess = true;
	}
	
	@AfterClass
	private void tearDown() throws Exception {
		try {
			if (runSuccess) {
				RegionUtils.deleteRegion(regionName);
				sdb.close();
			}
		}finally {
			if( s3Client != null ){
				s3Client.shutdown();
			}
		}
	}
}
