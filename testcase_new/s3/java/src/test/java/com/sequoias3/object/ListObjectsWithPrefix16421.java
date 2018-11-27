package com.sequoias3.object;

import java.io.File;
import java.io.IOException;
import java.util.List;

import org.testng.Assert;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import com.amazonaws.services.s3.AmazonS3;
import com.amazonaws.services.s3.model.ListObjectsV2Request;
import com.amazonaws.services.s3.model.ListObjectsV2Result;
import com.amazonaws.services.s3.model.S3ObjectSummary;
import com.sequoias3.testcommon.CommLib;
import com.sequoias3.testcommon.S3TestBase;
import com.sequoias3.testcommon.TestTools;

/**
 * test content: To get a list of objects within a bucket.specify prefix does not match the object. 
 * testlink-case: seqDB-16421 * 
 * @author wuyan
 * @Date 2018.11.19
 * @version 1.00
 */
public class ListObjectsWithPrefix16421 extends S3TestBase {
	private boolean runSuccess = false;
	private String bucketName = "bucket16421";
	private String key = "/aa/bb/object16421";
	private AmazonS3 s3Client = null;
	private int fileSize = 1024 * 100;
	private int matchObjectNums = 0;
	private File localPath = null;
	private String filePath = null;
	private String prefix = "/dir_1/prefix/test16421";;

	@SuppressWarnings("deprecation")
	@BeforeClass
	private void setUp() throws IOException {
		localPath = new File(S3TestBase.workDir + File.separator + TestTools.getClassName());
		filePath = localPath + File.separator + "localFile_" + fileSize + ".txt";

		TestTools.LocalFile.removeFile(localPath);
		TestTools.LocalFile.createDir(localPath.toString());
		TestTools.LocalFile.createFile(filePath, fileSize);
		s3Client = CommLib.buildS3Client();

		if (s3Client.doesBucketExist(bucketName)) {
			CommLib.clearBucket(s3Client, bucketName);
		}
		s3Client.createBucket(bucketName);
	}

	@Test
	public void testCreateObject() throws Exception {
		putObjects();
		listObjectsAndCheckResult();
		runSuccess = true;
	}

	@AfterClass
	private void tearDown() {
		try {
			if (runSuccess) {
				CommLib.clearBucket(s3Client, bucketName);
			}
		} finally {
			s3Client.shutdown();
		}
	}

	private void listObjectsAndCheckResult() throws IOException {
		ListObjectsV2Request request = new ListObjectsV2Request().withBucketName(bucketName).withEncodingType("url");
		request.withPrefix(prefix);
		ListObjectsV2Result result = s3Client.listObjectsV2(request);
		List<S3ObjectSummary> objects = result.getObjectSummaries();
		Assert.assertEquals(objects.size(), matchObjectNums);
	}

	private void putObjects() {
		int objectNums = 10;
		for (int i = 0; i < objectNums; i++) {
			String keyName = key + "_" + i + TestTools.getRandomString(i);
			s3Client.putObject(bucketName, keyName, new File(filePath));
		}
	}
}
