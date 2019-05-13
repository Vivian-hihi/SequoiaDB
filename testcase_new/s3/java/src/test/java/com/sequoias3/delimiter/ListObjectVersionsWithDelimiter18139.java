package com.sequoias3.delimiter;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

import org.testng.Assert;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import com.amazonaws.services.s3.AmazonS3;
import com.amazonaws.services.s3.model.CreateBucketRequest;
import com.amazonaws.services.s3.model.ListVersionsRequest;
import com.amazonaws.services.s3.model.S3VersionSummary;
import com.amazonaws.services.s3.model.VersionListing;
import com.sequoias3.testcommon.CommLib;
import com.sequoias3.testcommon.S3TestBase;
import com.sequoias3.testcommon.s3utils.DelimiterUtils;
import com.sequoias3.testcommon.s3utils.ObjectUtils;

/**
 * test content: 带分隔符查询桶中对象版本列表，匹配部分记录 testlink-case: seqDB-18139
 * 
 * @author wangkexin
 * @Date 2019.04.24
 * @version 1.00
 */

public class ListObjectVersionsWithDelimiter18139 extends S3TestBase {
	private String bucketName = "bucket18139";
	private String keyName = "dir";
	private String delimiter = "?";
	private String repeatedKeyName = "dir1?test18139";
	private List<String> keyList = new ArrayList<String>();
	private List<String> versions = new ArrayList<String>();
	private int objectWithDelimiterNum = 150;
	private int objectWithoutDelimiterNum = 50;
	private AmazonS3 s3Client = null;
	private boolean runSuccess = false;

	@BeforeClass
	private void setUp() throws Exception {
		s3Client = CommLib.buildS3Client();
		// create bucket and set bucket version status
		s3Client.createBucket(new CreateBucketRequest(bucketName));
		CommLib.setBucketVersioning(s3Client, bucketName, "Enabled");

		// put objects with delimiter
		for (int i = 0; i < objectWithDelimiterNum; i++) {
			String currentKeyName = keyName + i + delimiter + "test18139";
			s3Client.putObject(bucketName, currentKeyName, "object_file18139");
			keyList.add(currentKeyName);
		}

		// put objects without delimiter
		for (int i = 0; i < objectWithoutDelimiterNum; i++) {
			String currentKeyName = keyName + i + "test18139";
			s3Client.putObject(bucketName, currentKeyName, "object_file18139");
			versions.add(currentKeyName);
		}
		// TODO:1、构造数据的时候需要包含分隔符的对象和不包含分隔符的对象都有覆盖到ab场景
		// put object key = "dir1?test18139" twice again
		s3Client.putObject(bucketName, repeatedKeyName, "object_file18139");
		s3Client.putObject(bucketName, repeatedKeyName, "object_file18139");
	}

	@Test
	public void testGetObjectList() throws Exception {
		// 将分隔符设置为?（默认为'/'）
		DelimiterUtils.putBucketDelimiter(bucketName, delimiter);
		DelimiterUtils.checkCurrentDelimiteInfo(bucketName, delimiter);

		// db端查看访问计划显示索引为目录表索引
		VersionListing versionList = s3Client
				.listVersions(new ListVersionsRequest().withBucketName(bucketName).withDelimiter(delimiter));
		List<String> commonPrefixes = versionList.getCommonPrefixes();

		String[] objectNames = new String[keyList.size()];
		List<String> expresultList = ObjectUtils.getCommPrefixes(keyList.toArray(objectNames), "", delimiter);
		ObjectUtils.checkListObjectsV2Commprefixes(commonPrefixes, expresultList);

		List<String> actVersionsKeyName = new ArrayList<String>();
		List<S3VersionSummary> versionLists = versionList.getVersionSummaries();
		for (S3VersionSummary s3VersionSummary : versionLists) {
			actVersionsKeyName.add(s3VersionSummary.getKey());
			Assert.assertEquals(s3VersionSummary.getVersionId(), "0");
		}

		Collections.sort(versions);
		Assert.assertEquals(actVersionsKeyName, versions, "the returned result by versions is wrong, act: "
				+ actVersionsKeyName.toString() + ", exp: " + versions.toString());

		runSuccess = true;
	}

	@AfterClass
	private void tearDown() {
		try {
			if (runSuccess) {
				CommLib.deleteAllObjectVersions(s3Client, bucketName);
				s3Client.deleteBucket(bucketName);
			}
		} finally {
			if (s3Client != null) {
				s3Client.shutdown();
			}
		}
	}
}
