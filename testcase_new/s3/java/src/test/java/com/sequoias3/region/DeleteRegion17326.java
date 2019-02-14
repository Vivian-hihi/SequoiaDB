package com.sequoias3.region;

import org.testng.Assert;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import com.sequoiadb.base.Sequoiadb;
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
	private static Sequoiadb sdb = null;
	private boolean runSuccess = false;

	@BeforeClass
	private void setUp() throws Exception {
		sdb = new Sequoiadb(S3TestBase.coordUrl, "", "");
		//TODO:1、没有创建区域，不需要判读该区域是否存在
		if(RegionUtils.headRegion(NonexistentRegion)){
			RegionUtils.deleteRegion(NonexistentRegion);
		}
	}
	
	@Test
	public void testGetRegionMessage() throws Exception {
		try{
			RegionUtils.deleteRegion(NonexistentRegion);
		}catch(Exception e){
			Assert.fail("delete non-existent region should not throw exceptions!" + " errorMeg: " + e.getMessage());
		}

		runSuccess = true;
	}
	
	@AfterClass
	private void tearDown() throws Exception {
		if (runSuccess) {
			sdb.close();
		}
	}
}
