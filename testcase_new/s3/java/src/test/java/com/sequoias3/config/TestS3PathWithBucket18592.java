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
 * test content: 设置带s3路径校验，执行桶管理操作 testlink-case: seqDB-18592
 * 
 * @author wangkexin
 * @Date 2019.06.25
 * @version 1.00
 */
public class TestS3PathWithBucket18592 extends S3TestBase {
	private boolean runSuccess = false;
	private String bucketName = "bucket18592";
	private AmazonS3 s3Client = null;

	@BeforeClass
	private void setUp() throws Exception {
		s3Client = CommLib.buildS3Client();
		CommLib.clearBucket(s3Client, bucketName);
	}

	@Test
	private void testS3Path() {
		try {
			S3PathUtils.createBucket(bucketName);
			Assert.fail("create bucket " + bucketName + " should be failed!");
		} catch (AmazonS3Exception e) {
			Assert.assertEquals(e.getErrorCode(), "NoSuchBucket");
		}

		Assert.assertFalse(s3Client.doesBucketExist(bucketName));
		s3Client.createBucket(bucketName);

		try {
			S3PathUtils.headBucket(bucketName);
			Assert.fail("head bucket " + bucketName + " should be failed!");
		} catch (AmazonS3Exception e) {
			Assert.assertEquals(e.getStatusCode(), 404);
		}

		try {
			S3PathUtils.setBucketVersioning(bucketName, BucketVersioningConfiguration.ENABLED);
			Assert.fail("set bucket " + bucketName + " versioning should be failed!");
		} catch (AmazonS3Exception e) {
			Assert.assertEquals(e.getErrorCode(), "NoSuchBucket");
		}

		try {
			S3PathUtils.getBucketVersioning(bucketName);
			Assert.fail("get bucket " + bucketName + " versioning should be failed!");
		} catch (AmazonS3Exception e) {
			Assert.assertEquals(e.getErrorCode(), "NoSuchBucket");
		}

		try {
			S3PathUtils.listBuckets();
			Assert.fail("list buckets should be failed!");
		} catch (AmazonS3Exception e) {
			Assert.assertEquals(e.getErrorCode(), "NoSuchBucket");
		}

		try {
			S3PathUtils.getBucketLocation(bucketName);
			Assert.fail("get bucket location should be failed!");
		} catch (AmazonS3Exception e) {
			Assert.assertEquals(e.getErrorCode(), "NoSuchBucket");
		}

		try {
			S3PathUtils.deleteBucket(bucketName);
			Assert.fail("delete bucket should be failed!");
		} catch (AmazonS3Exception e) {
			Assert.assertEquals(e.getErrorCode(), "NoSuchBucket");
		}
		runSuccess = true;
	}

	@AfterClass
	private void tearDown() throws Exception {
		try {
			if (runSuccess) {
				s3Client.deleteBucket(bucketName);
			}
		} finally {
			if (s3Client != null) {
				s3Client.shutdown();
			}
		}
	}
}
