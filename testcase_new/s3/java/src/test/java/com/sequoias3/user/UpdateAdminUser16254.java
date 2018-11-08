package com.sequoias3.user;

import java.util.List;

import org.apache.log4j.Logger;
import org.json.JSONException;
import org.json.JSONObject;
import org.springframework.web.client.HttpClientErrorException;
import org.testng.Assert;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import com.amazonaws.services.s3.AmazonS3;
import com.amazonaws.services.s3.model.Bucket;
import com.sequoias3.testcommon.CommLib;
import com.sequoias3.testcommon.S3TestBase;

/**
 * @Description: seqDB-16254 :: 管理员更新管理员用户
 * @author wangkexin
 * @Date:2018年10月31日
 * @version:1.0
 */

public class UpdateAdminUser16254 extends S3TestBase {
	private static final Logger logger = Logger.getLogger(UpdateAdminUser16254.class);
	private String Username = "UpdateAdminUser16254";
	private String bucketName = "UpdateAdminUser16254Bucket";
	private boolean runSuccess = false;

	@BeforeClass
	private void setUp() {
		try {
			UserUtils.deleteUser(Username, UserUtils.accessKeyId, true);
		} catch (HttpClientErrorException e) {
			logger.info(e.getResponseBodyAsString());
		}
	}

	@Test
	public void test() throws JSONException {
		// create user
		JSONObject craeteUser = UserUtils.createUser(Username, UserCommDefind.admin, UserUtils.accessKeyId);

		// update user
		JSONObject updateUser = UserUtils.updateUser(Username, UserUtils.accessKeyId);
		System.out.println("actUser = " + updateUser.toString());

		// create bucket for check
		checkResult(craeteUser, updateUser);
		runSuccess = true;
	}

	@AfterClass
	private void tearDown() {
		if (runSuccess) {
			try {
				UserUtils.deleteUser(Username, UserUtils.accessKeyId, true);
			} catch (Exception e) {
				logger.info(e.getMessage());
			}
		}
	}

	private void checkResult(JSONObject craeteUser, JSONObject updateUser) {
		JSONObject createJSON = craeteUser.getJSONObject(UserCommDefind.accessKeys);
		String accessKeyIdOld = createJSON.getString(UserCommDefind.accessKeyID);
		String secretAccessKeyOld = createJSON.getString(UserCommDefind.secretAccessKey);

		JSONObject updateJSON = updateUser.getJSONObject(UserCommDefind.accessKeys);
		String accessKeyID = updateJSON.getString(UserCommDefind.accessKeyID);
		String secretAccessKey = updateJSON.getString(UserCommDefind.secretAccessKey);

		// check accessKeyID and secretAccessKey was already updated
		Assert.assertNotEquals(accessKeyIdOld, accessKeyID);
		Assert.assertNotEquals(secretAccessKeyOld, secretAccessKey);

		// check updated accessKeyID and secretAccessKey is active
		AmazonS3 s3Client = null;
		try {
			s3Client = CommLib.buildS3Client(accessKeyID, secretAccessKey);
			// create bucket
			s3Client.createBucket(bucketName.toLowerCase());

			// check
			List<Bucket> buckets = s3Client.listBuckets();
			Assert.assertEquals(buckets.size(), 1, " only one bucket");
			Bucket expbucket = buckets.get(0);
			String actOwner = expbucket.getOwner().getDisplayName();
			String actBucketName = expbucket.getName();
			Assert.assertEquals(actBucketName, bucketName.toLowerCase());
			Assert.assertEquals(actOwner, Username.toLowerCase());
		} catch (Exception e) {
			e.printStackTrace();
			Assert.fail(e.getMessage());
		} finally {
			if (s3Client != null) {
				s3Client.shutdown();
			}
		}
	}
}
