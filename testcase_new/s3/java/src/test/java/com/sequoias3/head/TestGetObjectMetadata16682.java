package com.sequoias3.head;

import java.util.Date;

import org.testng.Assert;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import com.amazonaws.services.s3.AmazonS3;
import com.amazonaws.services.s3.model.GetObjectMetadataRequest;
import com.amazonaws.services.s3.model.ObjectMetadata;
import com.amazonaws.services.s3.model.PutObjectResult;
import com.sequoiadb.exception.BaseException;
import com.sequoias3.testcommon.CommLib;
import com.sequoias3.testcommon.S3TestBase;
import com.sequoias3.testcommon.s3utils.HeadUtils;
import com.sequoias3.testcommon.s3utils.UserUtils;

/**
 * test content:  指定versionId查询历史版本对象 
 * testlink-case: seqDB-16682 
 * @author wangkexin
 * @Date 2018.12.07
 * @version 1.00
 */

public class TestGetObjectMetadata16682  extends S3TestBase{
	private boolean runSuccess = false;
	private String bucketName = "bucket16682";
	private String userName = "user16682";
	private String roleName = "normal";
	private String keyName = "key16682";
	private String content = "content16682";
	private AmazonS3 s3Client = null;

	@BeforeClass
	private void setUp() throws Exception {
		CommLib.clearUser(userName);
		String[] accessKeys = UserUtils.createUser(userName, roleName);
		s3Client = CommLib.buildS3Client(accessKeys[0], accessKeys[1]);
	}

	@Test
	private void testGetObjectMetadata() throws Exception {
		s3Client.createBucket(bucketName);
		CommLib.setBucketVersioning(s3Client, bucketName, "Enabled");
		String content1 = content+"v1111";
		String content2 = content+"v2222222";
		PutObjectResult result1 = s3Client.putObject(bucketName, keyName, content1);
		PutObjectResult result2 = s3Client.putObject(bucketName, keyName, content2);
		s3Client.putObject(bucketName, keyName, content);
		String expEtagV1 = result1.getETag();
		String expEtagV2 = result2.getETag();
		String expVersionidV1 = result1.getVersionId();
		String expVersionidV2 = result2.getVersionId();
		
			
		ObjectMetadata metadata = s3Client.getObjectMetadata(new GetObjectMetadataRequest(bucketName, keyName, expVersionidV1));
		String expLastModified1 = HeadUtils.getGMTDate(new Date());
		checkResult(metadata, expEtagV1, (long)content1.length(), expVersionidV1, expLastModified1);
		
		metadata = s3Client.getObjectMetadata(new GetObjectMetadataRequest(bucketName, keyName, expVersionidV2));
		String expLastModified2 = HeadUtils.getGMTDate(new Date());
		checkResult(metadata, expEtagV2, (long)content2.length(), expVersionidV2, expLastModified2);
		
		runSuccess = true;
	}

	@AfterClass
	private void tearDown() throws Exception {
		try {
			if (runSuccess) {
				UserUtils.deleteUser(userName);
			}
		} catch (BaseException e) {
			Assert.fail("clean up failed:" + e.getMessage());
		} finally {
			if (s3Client != null) {
				s3Client.shutdown();
			}
		}
	}
	
	private void checkResult(ObjectMetadata metadata, String etag, long size, String versionid, String expLastModified){
		Assert.assertEquals(metadata.getETag(), etag, "etag is wrong!");
		Assert.assertEquals(metadata.getContentLength(), size, "size is wrong!");
		Assert.assertEquals(metadata.getVersionId(), versionid, "versionid is wrong!");
		Assert.assertEquals(HeadUtils.getGMTDate(metadata.getLastModified()), expLastModified, "lastModified is wrong!");
	}
}
