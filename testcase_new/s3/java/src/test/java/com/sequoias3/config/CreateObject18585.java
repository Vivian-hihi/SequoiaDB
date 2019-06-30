package com.sequoias3.config;

import java.util.List;

import org.testng.Assert;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.DataProvider;
import org.testng.annotations.Test;

import com.amazonaws.services.s3.AmazonS3;
import com.amazonaws.services.s3.model.HeadBucketRequest;
import com.sequoias3.testcommon.CommLib;
import com.sequoias3.testcommon.S3TestBase;
import com.sequoias3.testcommon.TestTools;
import com.sequoias3.testcommon.s3utils.ConfigUtils;
import com.sequoias3.testcommon.s3utils.UserUtils;
import com.sequoias3.user.UserCommDefind;

/**
 * test content: 关闭鉴权，设置鉴权头部错误，上传对象 testlink-case: seqDB-18585
 * 
 * @author wangkexin
 * @Date 2019.06.20
 * @version 1.00
 */
public class CreateObject18585 extends S3TestBase {
	@DataProvider(name = "authorizationProvider")
	public Object[][] generateAuthorization() {
		return new Object[][] {
				// test a : authorization 为空
				new Object[] { "bucket18585", "" },
				// test b : authorization 为version2版本
				new Object[] { "bucket18585v2", "AWS " + accessKeys[0] + ":signature" },
				// test c : authorization 为version4版本
				new Object[] { "bucket18585v4", UserCommDefind.authValPre + accessKeys[0] + "/" } };
	}

	private boolean runSuccess = false;
	private String userName = "user18585";
	private String roleName = "normal";
	private String keyName = "key18585";
	private AmazonS3 s3Client = null;
	private AmazonS3 s3ClientNorMal = null;
	private String[] accessKeys = null;

	@BeforeClass
	private void setUp() throws Exception {
		CommLib.clearUser(userName);
		accessKeys = UserUtils.createUser(userName, roleName);
		s3ClientNorMal = CommLib.buildS3Client(accessKeys[0], accessKeys[1]);
		s3Client = CommLib.buildS3Client();
	}

	@Test(enabled = false) // (dataProvider = "authorizationProvider")
	private void testCreateObject(String bucketName, String authorization) throws Exception {
		String tmpContent = "content18585" + authorization;

		// 鉴权关闭，直接使用默认用户，携带鉴权消息被忽略
		// create bucket
		ConfigUtils.createBucket(bucketName, authorization);

		// check head bucket
		ConfigUtils.headBucket(bucketName, authorization);
		s3Client.headBucket(new HeadBucketRequest(bucketName));

		List<String> buckets = ConfigUtils.listBuckets(authorization);
		Assert.assertTrue(buckets.contains(bucketName));
		ConfigUtils.checkCreateBucketResult(s3ClientNorMal, bucketName, S3TestBase.s3UserName);

		ConfigUtils.putObject(bucketName, keyName, tmpContent, authorization);
		Assert.assertTrue(s3ClientNorMal.doesObjectExist(bucketName, keyName));
		Assert.assertTrue(s3Client.doesObjectExist(bucketName, keyName));

		// check head object
		ConfigUtils.headObject(bucketName, keyName, authorization);

		String actEtg = ConfigUtils.getObject(bucketName, keyName, authorization);
		Assert.assertEquals(actEtg, TestTools.getMD5(tmpContent.getBytes()));

		ConfigUtils.deleteObjet(bucketName, keyName, authorization);
		ConfigUtils.deleteBucket(bucketName, authorization);
		Assert.assertFalse(s3Client.doesBucketExist(bucketName));

		runSuccess = true;
	}

	@AfterClass
	private void tearDown() throws Exception {
		try {
			if (runSuccess) {
				UserUtils.deleteUser(userName);
			}
		} finally {
			if (s3Client != null) {
				s3Client.shutdown();
			}
			if (s3ClientNorMal != null) {
				s3ClientNorMal.shutdown();
			}
		}
	}
}
