package com.sequoias3.region;

import org.testng.Assert;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import com.amazonaws.services.s3.AmazonS3;
import com.amazonaws.services.s3.model.AmazonS3Exception;
import com.sequoiadb.base.Sequoiadb;
import com.sequoias3.testcommon.CommLib;
import com.sequoias3.testcommon.S3TestBase;
import com.sequoias3.testcommon.s3utils.RegionUtils;

/**
 * @Description seqDB-17293: create Region and  specify cs and cl. the cs and cl not exist
 * @author wuyan
 * @Date 2019.1.18
 * @version 1.00
 */
public class CreateRegion17293 extends S3TestBase{
	private boolean runSuccess = false;		
	private String regionName = "region17293";	
	private AmazonS3 s3Client = null;
	private Sequoiadb sdb = null;
	private String[] csNames = {"metaCS17293","dataCS17293"};
	private String[] metaclNames = {"metaCL17293","metaHistroyCL17293"};
	private String[] dataclNames = {"dataCL17293"};

	@BeforeClass
	private void setUp() {	
		sdb = new Sequoiadb(S3TestBase.coordUrl, "", "");			
		RegionUtils.createCSAndCL(sdb,csNames[0],metaclNames);			
		s3Client = CommLib.buildS3Client();			
	}

	@Test
	public void testRegion()throws Exception  {
		try{
			Region region = new Region();
			String metaLocation = csNames[0] + "." + metaclNames[0];	
			String metaHisLocation = csNames[0] + "." + metaclNames[1];
			String dataLocation = csNames[1] + "." + dataclNames[0];	
			region.withMetaLocation(metaLocation).withMetaHisLocation(metaHisLocation)
			.withDataLocation(dataLocation).withName(regionName);
			RegionUtils.putRegion(region);
			Assert.fail("put region must be fail!");
		}catch (AmazonS3Exception e) {
			//return 400:InvalidLocation	
			Assert.assertEquals(e.getStatusCode(), 400);
			Assert.assertEquals(e.getErrorCode(), "InvalidLocation");
		}	
		
		Assert.assertFalse(RegionUtils.headRegion(regionName), "region should be not exist!");		
		runSuccess = true;
	}	

	@AfterClass
	private void tearDown() {
		try {
			if (runSuccess) {
				sdb.dropCollectionSpace(csNames[0]);				
			}
		}finally {
			s3Client.shutdown();	
			sdb.close();
		}
	}
	
	
	
}
