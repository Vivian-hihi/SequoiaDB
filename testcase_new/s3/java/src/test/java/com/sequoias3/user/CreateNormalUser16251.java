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
 * @Description: seqDB-16251 :: 管理员创建普通（normal）用户
 * @author wangkexin
 * @Date:2018年10月30日
 * @version:1.0
 */

public class CreateNormalUser16251 extends S3TestBase {
	private static final Logger logger = Logger.getLogger(CreateNormalUser16251.class);
	private String userName = "CreateAdminUser16251";
	private String bucketName = "CreateAdminUser16251Bucket";
	private String accessKeyID = null;
	private String secretAccessKey = null;
	private boolean runSuccess = false;

	@BeforeClass
	private void setUp() {
		try {
			UserUtils.deleteUser(userName, UserUtils.accessKeyId, true);
		} catch (HttpClientErrorException e) {
			logger.info(e.getResponseBodyAsString());
		}
	}

	@Test
	public void test() throws JSONException {
		// create user
		JSONObject actUser = UserUtils.createUser(userName, UserCommDefind.normal, UserUtils.accessKeyId);
		// check create user result
		checkUser(actUser);
		// create bucket for check
		checkByCraeteBucket();
		runSuccess = true;
	}

	@AfterClass
	private void tearDown() {
		if (runSuccess) {
			try {
				UserUtils.deleteUser(userName, UserUtils.accessKeyId, true);
			} catch (Exception e) {
				logger.info(e.getMessage());
			}
		}
	}

	private void checkUser(JSONObject actUser) {
		// get user
		JSONObject expUser = UserUtils.getUser(userName, UserUtils.accessKeyId);

		JSONObject actJSON = actUser.getJSONObject(UserCommDefind.accessKeys);
		JSONObject expJSON = expUser.getJSONObject(UserCommDefind.accessKeys);

		// check
		accessKeyID = actJSON.getString(UserCommDefind.accessKeyID);
		secretAccessKey = actJSON.getString(UserCommDefind.secretAccessKey);
		Assert.assertEquals(accessKeyID, expJSON.getString(UserCommDefind.accessKeyID),
				"actJSON = " + actJSON.toString() + ",expJSON = " + expJSON.toString());
		Assert.assertEquals(secretAccessKey, expJSON.getString(UserCommDefind.secretAccessKey),
				"actJSON = " + actJSON.toString() + ",expJSON = " + expJSON.toString());
	}

	private void checkByCraeteBucket() {
		// check the user is active
		AmazonS3 s3Client = null;
		try {
			s3Client = CommLib.buildS3Client(accessKeyID, secretAccessKey);
			// create bucket
			s3Client.createBucket(bucketName.toLowerCase());

			// check
			List<Bucket> buckets = s3Client.listBuckets();
			Assert.assertEquals(buckets.size(), 1, " only one bucket");
			Bucket actbucket = buckets.get(0);
			String actOwner = actbucket.getOwner().getDisplayName();
			String actBucketName = actbucket.getName();
			Assert.assertEquals(actBucketName, bucketName.toLowerCase());
			Assert.assertEquals(actOwner, userName.toLowerCase());
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
