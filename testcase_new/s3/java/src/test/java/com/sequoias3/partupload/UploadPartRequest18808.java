package com.sequoias3.partupload;

import java.io.File;
import java.util.ArrayList;
import java.util.List;

import org.testng.Assert;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.DataProvider;
import org.testng.annotations.Test;

import com.amazonaws.services.s3.AmazonS3;
import com.amazonaws.services.s3.model.AmazonS3Exception;
import com.amazonaws.services.s3.model.CreateBucketRequest;
import com.amazonaws.services.s3.model.PartETag;
import com.amazonaws.services.s3.model.UploadPartRequest;
import com.amazonaws.services.s3.model.UploadPartResult;
import com.sequoias3.testcommon.CommLib;
import com.sequoias3.testcommon.S3TestBase;
import com.sequoias3.testcommon.TestTools;
import com.sequoias3.testcommon.s3utils.ObjectUtils;
import com.sequoias3.testcommon.s3utils.PartUploadUtils;

/**
 * test content: UploadPartRequest接口参数校验 testlink-case: seqDB-18808
 * 
 * @author wangkexin
 * @Date 2019.8.6
 * @version 1.00
 */
// TODO:1、参考文本用例需要补充其它参数的验证
public class UploadPartRequest18808 extends S3TestBase {
	@DataProvider(name = "legalKeyNameProvider")
	public Object[][] generateKeyName() {
		return new Object[][] {
				// test a : 范围内取值， partNumber:10
				new Object[] { "/dir1/test18808.txt", 10 },
				// test b : 长度边界值:1字节 ，partNumber:1
				new Object[] { ObjectUtils.getRandomString(1), 1 },
				// test c : 长度边界值:900字节 ，partNumber:10000
				new Object[] { ObjectUtils.getRandomString(900), 10000 } };
	}

	private String bucketName = "bucket18808";
	private AmazonS3 s3Client = null;
	private long fileSize = 5 * 1024;
	private File localPath = null;
	private File file = null;
	private String filePath = null;
	private boolean runSuccess = false;

	@BeforeClass
	private void setUp() throws Exception {
		localPath = new File(S3TestBase.workDir + File.separator + TestTools.getClassName());
		filePath = localPath + File.separator + "localFile_" + fileSize + ".txt";

		TestTools.LocalFile.removeFile(localPath);
		TestTools.LocalFile.createDir(localPath.toString());
		TestTools.LocalFile.createFile(filePath, fileSize);
		file = new File(filePath);

		s3Client = CommLib.buildS3Client();
		CommLib.clearBucket(s3Client, bucketName);
		s3Client.createBucket(new CreateBucketRequest(bucketName));
	}

	@Test(dataProvider = "legalKeyNameProvider")
	public void testLegalKeyName(String keyName, int partNumber) throws Exception {
		runSuccess = false;
		String uploadId = PartUploadUtils.initPartUpload(s3Client, bucketName, keyName);
		List<PartETag> partEtags = new ArrayList<>();
		int filePosition = 0;
		long fileSize = file.length();
		UploadPartRequest partRequest = new UploadPartRequest().withFile(file).withFileOffset(filePosition)
				.withPartNumber(partNumber).withPartSize(fileSize).withBucketName(bucketName).withKey(keyName)
				.withUploadId(uploadId);
		UploadPartResult uploadPartResult = s3Client.uploadPart(partRequest);
		partEtags.add(uploadPartResult.getPartETag());

		PartUploadUtils.completeMultipartUpload(s3Client, bucketName, keyName, uploadId, partEtags);

		String expMd5 = TestTools.getMD5(filePath);
		String downloadMd5 = ObjectUtils.getMd5OfObject(s3Client, localPath, bucketName, keyName);
		Assert.assertEquals(downloadMd5, expMd5);
		runSuccess = true;
	}

	@Test
	public void testIllegalKeyName() throws Exception {
		runSuccess = false;
		// test a : 对象名为空串，null，901个字节
		String uploadId = PartUploadUtils.initPartUpload(s3Client, bucketName, "key18808");
		int filePosition = 0;
		long fileSize = file.length();
		UploadPartRequest partRequest = new UploadPartRequest().withFile(file).withFileOffset(filePosition)
				.withPartNumber(1).withPartSize(fileSize).withBucketName(bucketName).withKey("").withUploadId(uploadId);
		try {
			s3Client.uploadPart(partRequest);
			Assert.fail("when key name is '',it should fail");
		} catch (AmazonS3Exception e) {
			Assert.assertEquals(e.getErrorCode(), "MalformedXML");
		}

		partRequest = new UploadPartRequest().withFile(file).withFileOffset(filePosition).withPartNumber(1)
				.withPartSize(fileSize).withBucketName(bucketName).withKey(null).withUploadId(uploadId);
		try {
			s3Client.uploadPart(partRequest);
			Assert.fail("when key name is null,it should fail");
		} catch (IllegalArgumentException e) {
			Assert.assertEquals(e.getMessage(), "The key parameter must be specified when uploading a part");
		}

		partRequest = new UploadPartRequest().withFile(file).withFileOffset(filePosition).withPartNumber(1)
				.withPartSize(fileSize).withBucketName(bucketName).withKey(ObjectUtils.getRandomString(901))
				.withUploadId(uploadId);
		try {
			s3Client.uploadPart(partRequest);
			Assert.fail("when key name is 901 characters,it should fail");
		} catch (AmazonS3Exception e) {
			Assert.assertEquals(e.getErrorMessage(),
					"The specified multipart upload does not exist. The upload ID might be invalid, or the multipart upload might have been aborted or completed.");
		}

		runSuccess = true;
	}

	@Test
	public void testIllegalPartNumber() throws Exception {
		runSuccess = false;
		String keyName = "testkey18808";
		// test a : partNumber超过边界值
		String uploadId = PartUploadUtils.initPartUpload(s3Client, bucketName, keyName);
		int filePosition = 0;
		long fileSize = file.length();
		UploadPartRequest partRequest = new UploadPartRequest().withFile(file).withFileOffset(filePosition)
				.withPartNumber(0).withPartSize(fileSize).withBucketName(bucketName).withKey(keyName)
				.withUploadId(uploadId);
		try {
			s3Client.uploadPart(partRequest);
			Assert.fail("when partNumber is 0,it should fail");
		} catch (AmazonS3Exception e) {
			Assert.assertEquals(e.getErrorCode(), "InvalidPartNumber");
		}

		partRequest = new UploadPartRequest().withFile(file).withFileOffset(filePosition).withPartNumber(10001)
				.withPartSize(fileSize).withBucketName(bucketName).withKey(keyName).withUploadId(uploadId);
		try {
			s3Client.uploadPart(partRequest);
			Assert.fail("when partNumber is 10001,it should fail");
		} catch (AmazonS3Exception e) {
			Assert.assertEquals(e.getErrorCode(), "InvalidPartNumber");
		}
		runSuccess = true;
	}

	@AfterClass
	private void tearDown() {
		try {
			if (runSuccess) {
				CommLib.clearBucket(s3Client, bucketName);
				TestTools.LocalFile.removeFile(localPath);
			}
		} finally {
			if (s3Client != null) {
				s3Client.shutdown();
			}
		}
	}
}