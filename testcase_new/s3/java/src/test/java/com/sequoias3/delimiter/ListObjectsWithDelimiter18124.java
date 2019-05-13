package com.sequoias3.delimiter;

import java.io.File;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Collections;
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
import com.sequoias3.testcommon.s3utils.DelimiterUtils;
import com.sequoias3.testcommon.s3utils.ObjectUtils;

/**
 * test content: 带分隔符delimiter和start-after查询对象元数据列表 testlink-case: seqDB-18124
 * 
 * @author wangkexin
 * @Date 2019.04.16
 * @version 1.00
 */
public class ListObjectsWithDelimiter18124 extends S3TestBase {
	private boolean runSuccess = false;
	private String bucketName = "bucket18124";
	private String delimiter = "#";
	private String[] objectNames = { "dir1#dir1#test18124_1", "dir2#dir2#test18124_2", "dir3#dir3#test18124_3",
			"dir4#dir4#test18124_4", "test18124_5" };
	private String expContents = "test18124_5";
	// TODO:1、请定义变量为private类型
	List<String> expCommonPrefixes = null;
	private File localPath = null;
	private AmazonS3 s3Client = null;
	private int fileSize = 1024 * 20;
	private String filePath = null;

	@BeforeClass
	private void setUp() throws Exception {
		localPath = new File(S3TestBase.workDir + File.separator + TestTools.getClassName());
		filePath = localPath + File.separator + "localFile_" + fileSize + ".txt";

		TestTools.LocalFile.removeFile(localPath);
		TestTools.LocalFile.createDir(localPath.toString());
		TestTools.LocalFile.createFile(filePath, fileSize);
		s3Client = CommLib.buildS3Client();
		CommLib.clearBucket(s3Client, bucketName);
		s3Client.createBucket(bucketName);

		// 将分隔符设置为# （默认为'/'）
		DelimiterUtils.putBucketDelimiter(bucketName, delimiter);
		DelimiterUtils.checkCurrentDelimiteInfo(bucketName, delimiter);

		for (int i = 0; i < objectNames.length; i++) {
			s3Client.putObject(bucketName, objectNames[i], new File(filePath));
		}
		expCommonPrefixes = ObjectUtils.getCommPrefixes(objectNames, "", delimiter);
		Collections.sort(expCommonPrefixes);
	}

	@Test
	private void testListObjects() throws Exception {
		int objectNums = objectNames.length;
		// test a: 指定位置为中间记录
		int startAfterNoA = objectNums / 2;
		listObjectsAndCheckResult(startAfterNoA);

		// test b: 指定第一条记录
		int startAfterNoB = 0;
		listObjectsAndCheckResult(startAfterNoB);

		// test c:指定最后一条记录
		int startAfterNoC = objectNums - 1;
		listObjectsAndCheckResult(startAfterNoC);

		// test d: 指定匹配最后一条记录
		int startAfterNoD = objectNums - 2;
		listObjectsAndCheckResult(startAfterNoD);

		// test e: 指定匹配不到记录
		String startAfter = "unmatched18124";
		misMatchObject(startAfter);
		runSuccess = true;
	}

	@AfterClass
	private void tearDown() throws Exception {
		try {
			if (runSuccess) {
				TestTools.LocalFile.removeFile(localPath);
				CommLib.clearBucket(s3Client, bucketName);
			}
		} finally {
			if (s3Client != null) {
				s3Client.shutdown();
			}
		}
	}

	private void listObjectsAndCheckResult(int startAfterNo) throws IOException {
		List<String> expkeyList = new ArrayList<>(expCommonPrefixes);
		ListObjectsV2Request request = new ListObjectsV2Request().withBucketName(bucketName).withEncodingType("url")
				.withStartAfter(objectNames[startAfterNo]).withDelimiter(delimiter);
		ListObjectsV2Result result;
		result = s3Client.listObjectsV2(request);
		List<String> actCommonPrefixes = result.getCommonPrefixes();
		List<S3ObjectSummary> objects = result.getObjectSummaries();
		// TODO:1、这段校验结果的代码逻辑要优化下，建议直接按预期的结果去匹配实际结果，另外expkeyList也比较模糊，直接用commonprefixes
		// check commonprefixes
		if (startAfterNo < expkeyList.size()) {
			for (int i = 0; i < startAfterNo + 1; i++) {
				expkeyList.remove(0);
				Assert.assertEquals(objects.get(0).getKey(), expContents);
				Assert.assertEquals(objects.size(), 1);
			}
		} else {
			// 如果指定startAfter为最后一条记录则返回结果为空
			expkeyList.clear();
			Assert.assertEquals(objects.size(), 0);
		}

		Collections.sort(expkeyList);
		Collections.sort(actCommonPrefixes);
		Assert.assertEquals(actCommonPrefixes, expkeyList);
	}

	private void misMatchObject(String startAfter) {
		ListObjectsV2Request request = new ListObjectsV2Request().withBucketName(bucketName).withEncodingType("url")
				.withStartAfter(startAfter).withDelimiter(delimiter);
		ListObjectsV2Result result = s3Client.listObjectsV2(request);
		List<String> actCommonPrefixes = result.getCommonPrefixes();
		List<S3ObjectSummary> objects = result.getObjectSummaries();

		// misMatchObject, the list size is 0
		Assert.assertEquals(actCommonPrefixes.size(), 0);
		Assert.assertEquals(objects.size(), 0);
	}
}
