package com.sequoias3.config;

import org.testng.Assert;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.DataProvider;
import org.testng.annotations.Test;

import com.amazonaws.services.s3.model.AmazonS3Exception;
import com.sequoias3.testcommon.CommLib;
import com.sequoias3.testcommon.S3TestBase;
import com.sequoias3.testcommon.s3utils.ConfigUtils;
import com.sequoias3.testcommon.s3utils.UserUtils;
import com.sequoias3.user.UserCommDefind;

/**
 * test content: 开启鉴权，使用普通用户执行用户管理操作 testlink-case: seqDB-18589
 * 
 * @author wangkexin
 * @Date 2019.06.24
 * @version 1.00
 */
public class CreateUser18589 extends S3TestBase {
	@DataProvider(name = "authorizationProvider")
	public Object[][] generateAuthorization() {
		return new Object[][] {
				// test a : authorization 为version2版本
				new Object[] { UserCommDefind.normal, "AWS " + accessKeys[0] + ":signature" },
				// test b : authorization 为version4版本
				new Object[] { UserCommDefind.admin, UserCommDefind.authValPre + accessKeys[0] + "/" } };
	}

	private boolean runSuccess = false;
	private String userName = "normaluser18589";
	private String roleName = "normal";
	private String[] accessKeys = null;

	@BeforeClass
	private void setUp() throws Exception {
		CommLib.clearUser(userName);
		accessKeys = UserUtils.createUser(userName, roleName);
	}

	@Test(dataProvider = "authorizationProvider")
	private void testCreateUserV2(String role, String authorization) throws Exception {
		String userName = "user18589";
		try {
			ConfigUtils.createUser(userName, role, authorization);
			Assert.fail("create " + role + " user by normal user should be failed!");
		} catch (AmazonS3Exception e) {
			Assert.assertEquals(e.getErrorCode(), "AccessDenied");
		}

		try {
			ConfigUtils.updateUser(userName, authorization);
			Assert.fail("update " + role + " user by normal user should be failed!");
		} catch (AmazonS3Exception e) {
			Assert.assertEquals(e.getErrorCode(), "AccessDenied");
		}

		try {
			ConfigUtils.getUser(userName, authorization);
			Assert.fail("get " + role + " user by normal user should be failed!");
		} catch (AmazonS3Exception e) {
			Assert.assertEquals(e.getErrorCode(), "AccessDenied");
		}

		try {
			ConfigUtils.deleteUser(userName, authorization, false);
			Assert.fail("delete " + role + " user by normal user should be failed!");
		} catch (AmazonS3Exception e) {
			Assert.assertEquals(e.getErrorCode(), "AccessDenied");
		}
		runSuccess = true;
	}

	@AfterClass
	private void tearDown() throws Exception {
		if (runSuccess) {
			UserUtils.deleteUser(userName);
		}
	}
}
