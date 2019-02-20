package com.sequoias3.region;

import org.testng.Assert;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import com.sequoias3.testcommon.S3TestBase;
import com.sequoias3.testcommon.s3utils.RegionUtils;

/**
 * test content: 删除不存在的区域
 * testlink-case: seqDB-17326
 * @author wangkexin
 * @Date 2019.01.24
 * @version 1.00
 */

public class DeleteRegion17326 extends S3TestBase{
	private String NonexistentRegion = "nonexistent17326";

	@BeforeClass
	private void setUp() throws Exception {}
	
	@Test
	public void testGetRegionMessage() throws Exception {
		try{
			RegionUtils.deleteRegion(NonexistentRegion);
		}catch(Exception e){
			Assert.fail("delete non-existent region should not throw exceptions!" + " errorMeg: " + e.getMessage());
		}
	}
	
	@AfterClass
	private void tearDown() throws Exception {}
}
