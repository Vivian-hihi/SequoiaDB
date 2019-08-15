package com.sequoias3.partupload;

import java.util.HashMap;
import java.util.Map;

import org.testng.Assert;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.DataProvider;
import org.testng.annotations.Test;

import com.amazonaws.services.s3.AmazonS3;
import com.amazonaws.services.s3.model.AmazonS3Exception;
import com.amazonaws.services.s3.model.CreateBucketRequest;
import com.amazonaws.services.s3.model.InitiateMultipartUploadRequest;
import com.amazonaws.services.s3.model.InitiateMultipartUploadResult;
import com.amazonaws.services.s3.model.ObjectMetadata;
import com.sequoias3.testcommon.CommLib;
import com.sequoias3.testcommon.S3TestBase;
import com.sequoias3.testcommon.s3utils.ObjectUtils;

/**
 * test content:withMetadata接口参数校验 testlink-case: seqDB-18807
 * 
 * @author wangkexin
 * @Date 2019.8.6
 * @version 1.00
 */
public class WithObjectMetaData18807 extends S3TestBase {
	@DataProvider(name = "legalMetadataProvider")
	public Object[][] generateMetadata() {
		Map<String, String> expMeta = new HashMap<>();
		expMeta.put("test1", "1234");
		expMeta.put("test2", "");
		expMeta.put("test3", null);

		Map<String, String> expMeta2 = new HashMap<>();
		expMeta2.put("test", ObjectUtils.getRandomString(2044));

		return new Object[][] {
				// test a : 合法元数据信息，空串，null
				new Object[] { expMeta },
				// test b : 长度等于2kB （key+value总大小）
				new Object[] { expMeta2 } };
	}

	private String bucketName = "bucket18807";
	private String keyName = "key18807";
	private AmazonS3 s3Client = null;
	private boolean runSuccess = false;

	@BeforeClass
	private void setUp() throws Exception {
		s3Client = CommLib.buildS3Client();
		CommLib.clearBucket(s3Client, bucketName);
		s3Client.createBucket(new CreateBucketRequest(bucketName));
	}

	@Test(dataProvider = "legalMetadataProvider")
	public void testLegalMetaData(Map<String, String> meta) throws Exception {
		runSuccess = false;
		InitiateMultipartUploadRequest initRequest = new InitiateMultipartUploadRequest(bucketName, keyName);
		ObjectMetadata metadata = new ObjectMetadata();
		metadata.setUserMetadata(meta);
		initRequest.withObjectMetadata(metadata);
		InitiateMultipartUploadResult result = s3Client.initiateMultipartUpload(initRequest);
		String uploadId = result.getUploadId();
		Assert.assertNotEquals(uploadId, null);
		runSuccess = true;
	}

	@Test
	public void testIllegalMetaData() throws Exception {
		runSuccess = false;
		InitiateMultipartUploadRequest initRequest = new InitiateMultipartUploadRequest(bucketName, keyName);
		ObjectMetadata metadata = new ObjectMetadata();
		Map<String, String> meta = new HashMap<>();
		meta.put("test", ObjectUtils.getRandomString(2045));
		metadata.setUserMetadata(meta);
		initRequest.withObjectMetadata(metadata);
		try {
			s3Client.initiateMultipartUpload(initRequest);
			Assert.fail("when size more than 2KB , it should fail");
		} catch (AmazonS3Exception e) {
			Assert.assertEquals(e.getErrorMessage(), "Your metadata headers exceed the maximum allowed metadata size.");
		}
		runSuccess = true;
	}

	@AfterClass
	private void tearDown() {
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
