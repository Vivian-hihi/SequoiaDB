package com.sequoias3.delimiter;

import org.testng.Assert;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import com.amazonaws.services.s3.AmazonS3;
import com.amazonaws.services.s3.model.AmazonS3Exception;
import com.amazonaws.services.s3.model.CreateBucketRequest;
import com.sequoias3.testcommon.CommLib;
import com.sequoias3.testcommon.S3TestBase;
import com.sequoias3.testcommon.s3utils.DelimiterUtils;

/**
 * test content: 不开启版本控制，不带versionId删除不存在的对象 testlink-case: seqDB-18171
 * 
 * @author wangkexin
 * @Date 2019.04.29
 * @version 1.00
 */
public class DeleteObjectWithDelimiter18171 extends S3TestBase {
	private boolean runSuccess = false;
	private String bucketName = "bucket18171";
	private String key = "dir1/dir2/中文&object18171";
	private String delimiter = "&";
	private AmazonS3 s3Client = null;

	@BeforeClass
	private void setUp() throws Exception {
		s3Client = CommLib.buildS3Client();
		s3Client.createBucket(new CreateBucketRequest(bucketName));
	}

	@Test
	public void testDeleteObject() throws Exception {
		DelimiterUtils.putBucketDelimiter(bucketName, delimiter);
		DelimiterUtils.checkCurrentDelimiteInfo(bucketName, delimiter);

		// 删除不存在的对象
		s3Client.deleteObject(bucketName, key);
		checkDeleteObjectResult(bucketName, key);
		runSuccess = true;
	}

	@AfterClass
	private void tearDown() {
		try {
			if (runSuccess) {
				// TODO:1、这个判断没有必要，可以去掉
				if (s3Client.doesObjectExist(bucketName, key)) {
					s3Client.deleteObject(bucketName, key);
				}
				s3Client.deleteBucket(bucketName);
			}
		} finally {
			s3Client.shutdown();
		}
	}

	// TODO:2、这里建议检查下list，防止生成删除标记对象
	private void checkDeleteObjectResult(String bucketName, String key) {
		boolean isExistObject = s3Client.doesObjectExist(bucketName, key);
		Assert.assertFalse(isExistObject, "the object should not exist!");
		try {
			s3Client.getObject(bucketName, key);
			Assert.fail("get not exist key must be fail !");
		} catch (AmazonS3Exception e) {
			Assert.assertEquals(e.getErrorCode(), "NoSuchKey");
		}
	}
}
