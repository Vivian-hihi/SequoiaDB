package com.sequoias3.region;

import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

import org.testng.Assert;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import com.sequoiadb.base.Sequoiadb;
import com.sequoias3.testcommon.S3TestBase;
import com.sequoias3.testcommon.s3utils.RegionUtils;

/**
 * test content: 更新/删除区域，获取区域列表
 * testlink-case: seqDB-17318
 * @author wangkexin
 * @Date 2019.01.24
 * @version 1.00
 */

public class GetRegionList17318 extends S3TestBase{
	private String regionName = "Beijing17318";
	private String dataDomain = "dataDomain17318";
	private String metaDomain = "metaDomain17318";
	private String metaCSName = "metaCS17318";
	private String dataCSName = "dataCS17318";
	private String[] metaClNames = {"metaCL17318","metaHistoryCL17318"};
	private String[] dataClName = {"dataCL17318"};
	private List<String> regionNames = new ArrayList<>();
	private int regionNum = 100;
	private static Sequoiadb sdb = null;
	private boolean runSuccess = false;

	@BeforeClass
	private void setUp() throws Exception {
		sdb = new Sequoiadb(S3TestBase.coordUrl, "", "");
		
		RegionUtils.dropDomain(metaDomain);
		RegionUtils.dropDomain(dataDomain);
		
		RegionUtils.createCSAndCL(metaCSName, metaClNames);
		RegionUtils.createCSAndCL(dataCSName, dataClName);
		
		RegionUtils.createDomain(dataDomain);
		RegionUtils.createDomain(metaDomain);
		
		for(int i = 0; i < regionNum; i++ ){
			String currRegionName = regionName+"-"+i;
			regionNames.add(currRegionName.toLowerCase());
			if(RegionUtils.headRegion(currRegionName)){
				RegionUtils.deleteRegion(currRegionName);
			}
		}
	}
	
	@Test
	public void testCreateRegion() throws Exception {
		//create regions
		for(int i = 0; i < regionNum/2; i++){
	        Region region = new Region();
	        region.withName(regionNames.get(i))
	        .withMetaLocation(metaCSName + "." + metaClNames[0])
	        .withMetaHisLocation(metaCSName + "." + metaClNames[1])
	        .withDataLocation(dataCSName + "." + dataClName[0]);
	        RegionUtils.putRegion(region);
		}
		
		for(int i = regionNum/2; i < regionNum; i++){
	        Region region = new Region();
	        region.withName(regionNames.get(i))
	        .withDataCSShardingType("quarter")
	    	.withDataCLShardingType("month")
	    	.withDataDomain(dataDomain)
	    	.withMetaDomain(metaDomain);
	        RegionUtils.putRegion(region);
		}
    	
		//update region message (the specified mode can not be modified)
		for(int i = regionNum/2; i < regionNum; i++){
	        Region region = new Region();
	        region.withName(regionNames.get(i))
	        .withDataCSShardingType("year")
	    	.withDataCLShardingType("quarter")
	    	.withDataDomain(dataDomain)
	    	.withMetaDomain(metaDomain);
	        RegionUtils.putRegion(region);
		}
		
		checkGetRegion();
		
		//delete region
		List<String> deleteRegions = new ArrayList<>();
		for(int i = 40; i < 60; i++){
			String deleteRegion = regionNames.get(i);
			RegionUtils.deleteRegion(deleteRegion);
			deleteRegions.add(deleteRegion);
		}
		regionNames.removeAll(deleteRegions);
		checkDeleteRegion(deleteRegions);
		
		//delete all region
		for(String regionName : regionNames){
			RegionUtils.deleteRegion(regionName);
		}
		checkDeleteRegion(regionNames);
    	runSuccess = true;
	}
	
	@AfterClass
	private void tearDown() throws Exception {
		if (runSuccess) {
			sdb.dropCollectionSpace(metaCSName);
			sdb.dropCollectionSpace(dataCSName);
			sdb.dropDomain(dataDomain);
			sdb.dropDomain(metaDomain);
			deleteRegions(regionNames);
			sdb.close();
		}
	}
	
	private void deleteRegions(List<String> regions) throws Exception{
		for(int i = 0; i < regions.size(); i++ ){
			if(RegionUtils.headRegion(regions.get(i))){
				RegionUtils.deleteRegion(regions.get(i));
			}
		}
	}
	
	private void checkGetRegion() throws Exception{
		List<String> actRegions = RegionUtils.listRegions();
		Set<String> unRepeatRegionNames = new HashSet<>();
		for(String regionName : actRegions){
			boolean isRepeat = unRepeatRegionNames.add(regionName);
			Assert.assertTrue(isRepeat, "the region name " + regionName + " is repeated!");
		}
		Assert.assertTrue(actRegions.containsAll(regionNames)," expect region num is : " + regionNames.size());
	}
	
	private void checkDeleteRegion(List<String> deleteRegions) throws Exception{
		List<String> actRegions = RegionUtils.listRegions();
		for(String region : deleteRegions){
			Assert.assertFalse(actRegions.contains(region),"the region : " + region + " is still exist!");
		}
	}
}
