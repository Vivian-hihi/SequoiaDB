package com.sequoias3.user.concurrent;

import java.util.ArrayList;
import java.util.List;

import org.apache.log4j.Logger;
import org.json.JSONObject;
import org.json.XML;
import org.springframework.web.client.HttpClientErrorException;
import org.testng.Assert;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import com.sequoias3.testcommon.S3TestBase;
import com.sequoias3.testcommon.S3ThreadBase;
import com.sequoias3.user.UserCommDefind;
import com.sequoias3.user.UserUtils;

/**
 * @Description: seqDB-16272 :: 并发删除相同用户
 * @author wangkexin
 * @Date:2018年11月06日
 * @version:1.0
 */

public class DeleteUser16272 extends S3TestBase {
	private static Logger log = Logger.getLogger(DeleteUser16272.class);
	private static int successCount = 0;
	private String username = "DeleteUser16272";
	private int num = 100;
	private boolean runSuccess = false;

	@BeforeClass
	private void setUp() throws Exception {
		try {
			UserUtils.deleteUser(username, UserUtils.accessKeyId, true);
		} catch (HttpClientErrorException e) {
			log.info(e.getResponseBodyAsString());
		}
		// create an normal user
		JSONObject actUser = UserUtils.createUser(username, UserCommDefind.normal, UserUtils.accessKeyId);
	}

	@Test
	public void testDeleteUser() throws Exception {
		List<DeleteUser> threads = new ArrayList<DeleteUser>();
		for (int i = 0; i < num; i++) {
			threads.add(new DeleteUser());
		}
		for (int i = 0; i < num; i++) {
			threads.get(i).start();
		}
		for (int i = 0; i < num; i++) {
			if (threads.get(i).isSuccess()) {
				successCount++;
			}
		}
		if (successCount == 0) {
			Assert.fail("No thread has been deleted successfully");
		}
		// check result
		checkResult();
		runSuccess = true;
	}

	@AfterClass
	private void tearDown() throws Exception {
		if (runSuccess) {
			try {
				UserUtils.deleteUser(username, UserUtils.accessKeyId, true);
			} catch (HttpClientErrorException e) {
				log.info(e.getResponseBodyAsString());
			}
		}
	}

	private class DeleteUser extends S3ThreadBase {
		@Override
		public void exec() {
			try {
				UserUtils.deleteUser(username, UserUtils.accessKeyId);
			} catch (HttpClientErrorException e) {
				// e.printStackTrace();
				Assert.fail(e.getMessage());
			}
		}
	}

	private void checkResult() {
		try {
			UserUtils.getUser(username, UserUtils.accessKeyId);
			Assert.fail("exp fail but act success");
		} catch (HttpClientErrorException e) {
			String errorMsg = e.getResponseBodyAsString();
			org.json.JSONObject json1 = XML.toJSONObject(errorMsg);
			if (!json1.getJSONObject(UserCommDefind.error).getString(UserCommDefind.errorCode).contains("NoSuchUser")) {
				e.printStackTrace();
				Assert.fail(e.getMessage());
			}
		}
	}
}
