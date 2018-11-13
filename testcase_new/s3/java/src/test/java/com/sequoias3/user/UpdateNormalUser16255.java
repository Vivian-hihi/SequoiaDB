package com.sequoias3.user;

import java.util.List;

import org.json.JSONException;
import org.json.JSONObject;
import org.springframework.http.HttpStatus;
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
 * @Description: seqDB-16255 :: 管理员更新普通用户
 * @author fanyu
 * @Date:2018年10月29日
 * @version:1.0
 */

public class UpdateNormalUser16255 extends S3TestBase {
	private String name = "UpdateNormalUser16255";
	private boolean runSuccess = false;

	@BeforeClass
	private void setUp() {
		try {
			UserUtils.deleteUser(name, UserUtils.accessKeyId, true);
		} catch (HttpClientErrorException e) {
			if (e.getStatusCode() != (HttpStatus.NOT_FOUND)) {
				e.printStackTrace();
				Assert.fail(e.getMessage());
			}
		}
	}

	@Test
	private void test() throws JSONException {
		// create user
		JSONObject craeteUser = UserUtils.createUser(name, UserCommDefind.normal, UserUtils.accessKeyId);

		// update user
		JSONObject updateUser = UserUtils.updateUser(name, UserUtils.accessKeyId);

		// check
		checkResult(craeteUser, updateUser);
		runSuccess = true;
	}

	@AfterClass
	private void tearDown() {
		if (runSuccess) {
			UserUtils.deleteUser(name, UserUtils.accessKeyId, true);
		}
	}

	private void checkResult(JSONObject craeteUser, JSONObject updateUser) {
		JSONObject createJSON = craeteUser.getJSONObject(UserCommDefind.accessKeys);
		String accessKeyID1 = createJSON.getString(UserCommDefind.accessKeyID);
		String secretAccessKey1 = createJSON.getString(UserCommDefind.secretAccessKey);

		JSONObject updateJSON = updateUser.getJSONObject(UserCommDefind.accessKeys);
		String accessKeyID = updateJSON.getString(UserCommDefind.accessKeyID);
		String secretAccessKey = updateJSON.getString(UserCommDefind.secretAccessKey);

		// check accessKeyID and secretAccessKey was already updated
		Assert.assertNotEquals(accessKeyID1, accessKeyID);
		Assert.assertNotEquals(secretAccessKey1, secretAccessKey);

		// check updated accessKeyID and secretAccessKey is active
		AmazonS3 s3Client = null;
		try {
			s3Client = CommLib.buildS3Client(accessKeyID, secretAccessKey);
			// create bucket
			s3Client.createBucket(name.toLowerCase());

			// check
			List<Bucket> buckets = s3Client.listBuckets();
			Assert.assertEquals(buckets.size(), 1, " only one bucket");
			Bucket expbucket = buckets.get(0);
			String actOwner = expbucket.getOwner().getDisplayName();
			String actBucketName = expbucket.getName();
			Assert.assertEquals(actBucketName, name.toLowerCase());
			Assert.assertEquals(actOwner, name.toLowerCase());
		} finally {
			if (s3Client != null) {
				s3Client.shutdown();
			}
		}
	}
}
