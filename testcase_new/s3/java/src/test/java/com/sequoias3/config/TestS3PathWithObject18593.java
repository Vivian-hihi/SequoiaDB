package com.sequoias3.config;

import org.testng.Assert;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import com.amazonaws.services.s3.AmazonS3;
import com.amazonaws.services.s3.model.AmazonS3Exception;
import com.amazonaws.services.s3.model.BucketVersioningConfiguration;
import com.sequoias3.testcommon.CommLib;
import com.sequoias3.testcommon.S3TestBase;
import com.sequoias3.testcommon.s3utils.S3PathUtils;

/**
 * test content: 设置带s3路径校验，执行对象管理操作 testlink-case: seqDB-18593
 * 
 * @author wangkexin
 * @Date 2019.06.25
 * @version 1.00
 */
public class TestS3PathWithObject18593 extends S3TestBase {
	private boolean runSuccess = false;
	private String bucketName = "bucket18593";
	private String keyName = "key18593";
	private String content = "content18593";
	private AmazonS3 s3Client = null;

	@BeforeClass
	private void setUp() throws Exception {
		s3Client = CommLib.buildS3Client();
		CommLib.clearBucket(s3Client, bucketName);
		s3Client.createBucket(bucketName);
		CommLib.setBucketVersioning(s3Client, bucketName, BucketVersioningConfiguration.ENABLED);
	}

	@Test
	private void testS3Path() {
		try {
			S3PathUtils.putObject(bucketName, keyName, content);
			Assert.fail("put object " + keyName + " should be failed!");
		} catch (AmazonS3Exception e) {
			Assert.assertEquals(e.getErrorCode(), "NoSuchBucket");
		}

		Assert.assertFalse(s3Client.doesObjectExist(bucketName, keyName));
		s3Client.putObject(bucketName, keyName, content);

		try {
			S3PathUtils.getObject(bucketName, keyName);
			Assert.fail("get object " + keyName + " should be failed!");
		} catch (AmazonS3Exception e) {
			Assert.assertEquals(e.getStatusCode(), 404);
		}

		try {
			S3PathUtils.headObject(bucketName, keyName);
			Assert.fail("head object " + keyName + " should be failed!");
		} catch (AmazonS3Exception e) {
			Assert.assertEquals(e.getStatusCode(), 404);
		}

		try {
			S3PathUtils.listOvjectV2(bucketName);
			Assert.fail("list objects should be failed!");
		} catch (AmazonS3Exception e) {
			Assert.assertEquals(e.getStatusCode(), 404);
		}

		try {
			S3PathUtils.listVersions(bucketName);
			Assert.fail("list object versions should be failed!");
		} catch (AmazonS3Exception e) {
			Assert.assertEquals(e.getStatusCode(), 404);
		}

		try {
			S3PathUtils.deleteObjet(bucketName, keyName);
			Assert.fail("delete object should be failed!");
		} catch (AmazonS3Exception e) {
			Assert.assertEquals(e.getStatusCode(), 404);
		}

		try {
			S3PathUtils.deleteVersion(bucketName, keyName, 0);
			Assert.fail("delete object version should be failed!");
		} catch (AmazonS3Exception e) {
			Assert.assertEquals(e.getStatusCode(), 404);
		}
		runSuccess = true;
	}

	@AfterClass
	private void tearDown() throws Exception {
		try {
			if (runSuccess) {
				CommLib.clearBucket(s3Client, bucketName);
			}
		} finally {
			if (s3Client != null) {
				s3Client.shutdown();
			}
		}
	}
}
