package com.sequoias3.region;

import java.util.Date;

import org.testng.Assert;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import com.amazonaws.services.s3.AmazonS3;
import com.amazonaws.services.s3.model.CreateBucketRequest;
import com.sequoiadb.base.Sequoiadb;
import com.sequoias3.testcommon.CommLib;
import com.sequoias3.testcommon.S3TestBase;
import com.sequoias3.testcommon.s3utils.RegionUtils;

/**
 * test content: 获取区域信息
 * testlink-case: seqDB-17320
 * @author wangkexin
 * @Date 2019.01.24
 * @version 1.00
 */

public class GetRegionMessage17320 extends S3TestBase{
	private String specifiedModeRegion = "Beijing17320";
	private String shardingModeRegion = "Shanghai17320";
	private String dataDomain = "dataDomain17320";
	private String metaDomain = "metaDomain17320";
	private String metaCSName = "metaCS17320";
	private String dataCSName = "dataCS17320";
	private String[] metaClNames = {"metaCL17320","metaHistoryCL17320"};
	private String[] dataClName = {"dataCL17320"};
	private String spmodeBucket = "bucket17320-1";
	private String shmodeBucket = "bucket17320-2";
	String shardingDataCSName = null;
	String shardingMetaCSName = null;
	private static Sequoiadb sdb = null;
	private AmazonS3 s3Client = null;
	private boolean runSuccess = false;

	@BeforeClass
	private void setUp() throws Exception {
		s3Client = CommLib.buildS3Client();
		CommLib.clearBucket(s3Client, spmodeBucket);
		CommLib.clearBucket(s3Client, shmodeBucket);
		
		//delete data/meta cs
		shardingDataCSName = RegionUtils.getDataCSName(shardingModeRegion.toLowerCase(), "quarter", new Date());
		shardingMetaCSName = RegionUtils.getMetaCSName(shardingModeRegion.toLowerCase());
		sdb = new Sequoiadb(S3TestBase.coordUrl, "", "");
		if (sdb.isCollectionSpaceExist(shardingDataCSName)) {
			sdb.dropCollectionSpace(shardingDataCSName);
		}
		if (sdb.isCollectionSpaceExist(shardingMetaCSName)) {
			sdb.dropCollectionSpace(shardingMetaCSName);
		}
		
		RegionUtils.dropDomain(metaDomain);
		RegionUtils.dropDomain(dataDomain);

		if (RegionUtils.headRegion(specifiedModeRegion)) {
			RegionUtils.deleteRegion(specifiedModeRegion);
		}
		if (RegionUtils.headRegion(shardingModeRegion)) {
			RegionUtils.deleteRegion(shardingModeRegion);
		}
		
		RegionUtils.createCSAndCL(metaCSName, metaClNames);
		RegionUtils.createCSAndCL(dataCSName, dataClName);
		
		RegionUtils.createDomain(dataDomain);
		RegionUtils.createDomain(metaDomain);
		
		// create region with specified mode
		Region region1 = new Region();
		region1.withName(specifiedModeRegion)
		.withMetaLocation(metaCSName + "." + metaClNames[0])
		.withMetaHisLocation(metaCSName + "." + metaClNames[1])
		.withDataLocation(dataCSName + "." + dataClName[0]);
		RegionUtils.putRegion(region1);

		// create region with ShardingType mode
		Region region2 = new Region();
		region2.withName(shardingModeRegion)
		.withDataCSShardingType("quarter")
		.withDataCLShardingType("month")
		.withDataDomain(dataDomain)
		.withMetaDomain(metaDomain);
		RegionUtils.putRegion(region2);
		
		//create bucket and put object
		s3Client.createBucket(new CreateBucketRequest(spmodeBucket, specifiedModeRegion.toLowerCase()));
		s3Client.createBucket(new CreateBucketRequest(shmodeBucket, shardingModeRegion.toLowerCase()));
		
		s3Client.putObject(spmodeBucket, "key17320_1", "content17320_1");
		s3Client.putObject(shmodeBucket, "key17320_2", "content17320_2");
	}
	
	@Test
	public void testCreateRegion() throws Exception {
		//TODO:1、没有校验桶列表信息
		//specified mode : get region
		GetRegionResult spResult = RegionUtils.getRegion(specifiedModeRegion);
		Region spRegion = spResult.getRegion();
		Assert.assertEquals(spRegion.getName(), specifiedModeRegion.toLowerCase());
		Assert.assertEquals(spRegion.getMetaLocation(), metaCSName + "." + metaClNames[0]);
		Assert.assertEquals(spRegion.getMetaHisLocation(), metaCSName + "." + metaClNames[1]);
		Assert.assertEquals(spRegion.getDataLocation(), dataCSName + "." + dataClName[0]);
        
		
		//ShardingType mode : get region
		GetRegionResult stResult = RegionUtils.getRegion(shardingModeRegion);
		Region stRegion = stResult.getRegion();
		Assert.assertEquals(stRegion.getName(), shardingModeRegion.toLowerCase());
		Assert.assertEquals(stRegion.getDataCSShardingType(), "quarter");
		Assert.assertEquals(stRegion.getDataCLShardingType(), "month");
		Assert.assertEquals(stRegion.getMetaDomain(), metaDomain);
		Assert.assertEquals(stRegion.getDataDomain(), dataDomain);
		
    	runSuccess = true;
	}
	
	@AfterClass
	private void tearDown() throws Exception {
		try {
			if (runSuccess) {
				CommLib.clearBucket(s3Client, spmodeBucket);
				CommLib.clearBucket(s3Client, shmodeBucket);
				sdb.dropCollectionSpace(metaCSName);
				sdb.dropCollectionSpace(dataCSName);
				sdb.dropCollectionSpace(shardingDataCSName);
				sdb.dropCollectionSpace(shardingMetaCSName);
				sdb.dropDomain(dataDomain);
				sdb.dropDomain(metaDomain);
				RegionUtils.deleteRegion(specifiedModeRegion);
				RegionUtils.deleteRegion(shardingModeRegion);
				sdb.close();
			}
		}finally {
			if( s3Client != null ){
				s3Client.shutdown();
			}
		}
	}
}