package com.sequoias3.delimiter.concurrent;

import java.io.File;
import java.util.ArrayList;
import java.util.List;

import org.testng.Assert;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import com.amazonaws.services.s3.AmazonS3;
import com.amazonaws.services.s3.model.CreateBucketRequest;
import com.amazonaws.services.s3.model.ListObjectsV2Request;
import com.amazonaws.services.s3.model.ListObjectsV2Result;
import com.amazonaws.services.s3.model.S3ObjectSummary;
import com.sequoiadb.threadexecutor.ThreadExecutor;
import com.sequoiadb.threadexecutor.annotation.ExecuteOrder;
import com.sequoias3.testcommon.CommLib;
import com.sequoias3.testcommon.S3TestBase;
import com.sequoias3.testcommon.TestTools;
import com.sequoias3.testcommon.s3utils.DelimiterUtils;
import com.sequoias3.testcommon.s3utils.ObjectUtils;

/**
 * test content: 并发不同条件查询对象列表 testlink-case: seqDB-18185
 * 
 * @author wangkexin
 * @Date 2019.05.08
 * @version 1.00
 */

public class ListObjectWithDelimiter18185 extends S3TestBase {
	private String bucketName = "bucket18185";
	private String keyName = "dir1/dir2/test18185";
	private String delimiter = "?";
	private String unMatchDelimiter = "#";
	private String prefix = "dir1";
	private int objectNum = 100;
	private AmazonS3 s3Client = null;
	private int fileSize = 1024 * 10;
	private List<String> keyNames = new ArrayList<>();
	private File localPath = null;
	private String filePath = null;
	private boolean runSuccess = false;

	@BeforeClass
	private void setUp() throws Exception {
		localPath = new File(S3TestBase.workDir + File.separator + TestTools.getClassName());
		filePath = localPath + File.separator + "localFile_" + fileSize + ".txt";
		TestTools.LocalFile.removeFile(localPath);
		TestTools.LocalFile.createDir(localPath.toString());
		TestTools.LocalFile.createFile(filePath, fileSize);

		s3Client = CommLib.buildS3Client();
		CommLib.clearBucket(s3Client, bucketName);
		s3Client.createBucket(new CreateBucketRequest(bucketName));
	}

	@Test
	public void testGetObjectList() throws Exception {
		DelimiterUtils.putBucketDelimiter(bucketName, delimiter);
		DelimiterUtils.checkCurrentDelimiteInfo(bucketName, delimiter);

		for (int i = 0; i < objectNum; i++) {
			String currentKey = keyName + "_" + i + delimiter + "/test" + unMatchDelimiter + ".txt";
			s3Client.putObject(bucketName, currentKey, new File(filePath));
			keyNames.add(currentKey);
		}

		ThreadExecutor es = new ThreadExecutor();
		es.addWorker(new TransListObjectWithDelimiter18185());
		es.addWorker(new TransListObjectWithPrefixAndDelimiter18185());
		es.run();
		runSuccess = true;
	}

	@AfterClass
	private void tearDown() {
		try {
			if (runSuccess) {
				for (String key : keyNames) {
					s3Client.deleteObject(bucketName, key);
				}
				s3Client.deleteBucket(bucketName);
				TestTools.LocalFile.removeFile(localPath);
			}
		} finally {
			if (s3Client != null) {
				s3Client.shutdown();
			}
		}
	}

	class TransListObjectWithDelimiter18185 {
		@ExecuteOrder(step = 1, desc = "指定与桶分隔符不匹配的delimiter查询对象列表")
		public void ListObject() {
			String[] objectNames = keyNames.toArray(new String[keyNames.size()]);
			List<String> expCommprefixList = ObjectUtils.getCommPrefixes(objectNames, "", unMatchDelimiter);
			ListObjectsV2Request request = new ListObjectsV2Request().withBucketName(bucketName).withEncodingType("url")
					.withDelimiter(unMatchDelimiter);
			ListObjectsV2Result result = s3Client.listObjectsV2(request);
			List<String> commonPrefixes = result.getCommonPrefixes();
			ObjectUtils.checkListObjectsV2Commprefixes(commonPrefixes, expCommprefixList);

			List<S3ObjectSummary> objects = result.getObjectSummaries();
			Assert.assertEquals(objects.size(), 0);
		}
	}

	class TransListObjectWithPrefixAndDelimiter18185 {
		@ExecuteOrder(step = 1, desc = "指定prefix和delimiter查询对象列表")
		public void ListObject() {
			String[] objectNames = keyNames.toArray(new String[keyNames.size()]);
			List<String> expCommprefixList = ObjectUtils.getCommPrefixes(objectNames, "", delimiter);
			ListObjectsV2Request request = new ListObjectsV2Request().withBucketName(bucketName).withEncodingType("url")
					.withPrefix(prefix).withDelimiter(delimiter);
			ListObjectsV2Result result = s3Client.listObjectsV2(request);
			List<String> commonPrefixes = result.getCommonPrefixes();
			ObjectUtils.checkListObjectsV2Commprefixes(commonPrefixes, expCommprefixList);

			List<S3ObjectSummary> objects = result.getObjectSummaries();
			Assert.assertEquals(objects.size(), 0);
		}
	}
}
