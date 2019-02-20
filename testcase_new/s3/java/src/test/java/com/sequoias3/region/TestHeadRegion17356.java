package com.sequoias3.region;

import org.testng.Assert;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import com.amazonaws.services.s3.model.AmazonS3Exception;
import com.sequoiadb.base.Sequoiadb;
import com.sequoias3.testcommon.S3TestBase;
import com.sequoias3.testcommon.s3utils.RegionUtils;

/**
 * test content:  HeadRegion接口参数校验 
 * testlink-case: seqDB-17356
 * @author wangkexin
 * @Date 2019.01.24
 * @version 1.00
 */

public class TestHeadRegion17356 extends S3TestBase{
	private String regionName = "Beijing17356";
	private String metaCSName = "metaCS17356";
	private String dataCSName = "dataCS17356";
	private String[] metaClNames = {"metaCL17356","metaHistoryCL17356"};
	private String[] dataClName = {"dataCL17356"};
	private boolean runSuccess = false;

	@BeforeClass
	private void setUp() throws Exception {
		RegionUtils.createCSAndCL(metaCSName, metaClNames);
		RegionUtils.createCSAndCL(dataCSName, dataClName);
		
		if(RegionUtils.headRegion(regionName)){
			RegionUtils.deleteRegion(regionName);
		}
		Region region = new Region();
        region.withName(regionName)
        .withMetaLocation(metaCSName + "." + metaClNames[0])
	    .withMetaHisLocation(metaCSName + "." + metaClNames[1])
	    .withDataLocation(dataCSName + "." + dataClName[0]);
        RegionUtils.putRegion(region);
	}
	
	@Test
	public void testCreateRegion() throws Exception {
        //合法值
        Assert.assertTrue(RegionUtils.headRegion(regionName));
    	
    	//非法值
        try{
        	RegionUtils.headRegion("");
        	Assert.fail("head region with '' region name should fail");
        }catch(AmazonS3Exception e){
        	Assert.assertEquals(e.getStatusCode(), 405);
        }
        
        try{
        	RegionUtils.headRegion(new String());
        	Assert.fail("head region with null region name should fail");
        }catch(AmazonS3Exception e){
        	Assert.assertEquals(e.getStatusCode(), 405);
        }
        
        runSuccess = true;
	}
	
	@AfterClass
	private void tearDown() throws Exception {
		if (runSuccess) {
			try(Sequoiadb sdb = new Sequoiadb(S3TestBase.coordUrl, "", "")){
				RegionUtils.deleteRegion(regionName);
				sdb.dropCollectionSpace(dataCSName);
				sdb.dropCollectionSpace(metaCSName);
			}
		}
	}
}
