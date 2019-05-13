package com.sequoias3.delimiter;

import java.util.ArrayList;
import java.util.List;

import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import com.amazonaws.services.s3.AmazonS3;
import com.amazonaws.services.s3.model.CreateBucketRequest;
import com.amazonaws.services.s3.model.ListObjectsV2Request;
import com.amazonaws.services.s3.model.ListObjectsV2Result;
import com.sequoiadb.threadexecutor.ThreadExecutor;
import com.sequoiadb.threadexecutor.annotation.ExecuteOrder;
import com.sequoias3.testcommon.CommLib;
import com.sequoias3.testcommon.S3TestBase;
import com.sequoias3.testcommon.s3utils.DelimiterUtils;
import com.sequoias3.testcommon.s3utils.ObjectUtils;

/**
 * test content: 分隔符状态不可用，查询对象列表 testlink-case: seqDB-18134
 * 
 * @author wangkexin
 * @Date 2019.04.23
 * @version 1.00
 */

public class ListObjectsWithDelimiter18134 extends S3TestBase {
	private String bucketName = "bucket18134";
	private String[] objectNames = new String[100];
	private String delimiter = "test";
	private List<String> expresultList = new ArrayList<String>();
	private AmazonS3 s3Client = null;
	private boolean runSuccess = false;

	@BeforeClass
	private void setUp() throws Exception {
		s3Client = CommLib.buildS3Client();
		s3Client.createBucket(new CreateBucketRequest(bucketName));
		for (int i = 0; i < objectNames.length; i++) {
			String currentKey = "dir" + i + "/dir" + i + delimiter + "test18134";
			objectNames[i] = currentKey;
			s3Client.putObject(bucketName, currentKey, "object_file18134");
		}
	}

	@Test
	public void testGetObjectList() throws Exception {
		ThreadExecutor es = new ThreadExecutor();
		es.addWorker(new TransListObjectsV218134());
		es.addWorker(new TransUpdateDelimiter18134());
		es.run();
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

	// TODO:1、类名加上Trans前缀是有特殊意义吗？
	class TransUpdateDelimiter18134 {
		@ExecuteOrder(step = 1, desc = "更新分隔符")
		public void updateDelimiter() {
			DelimiterUtils.putBucketDelimiter(bucketName, delimiter);
		}

		@ExecuteOrder(step = 3, desc = "检查分隔符更新结果")
		public void checkResult() throws Exception {
			DelimiterUtils.checkCurrentDelimiteInfo(bucketName, delimiter);
		}
	}

	class TransListObjectsV218134 {
		ListObjectsV2Request req = new ListObjectsV2Request().withBucketName(bucketName).withDelimiter(delimiter);
		ListObjectsV2Result result = new ListObjectsV2Result();

		@ExecuteOrder(step = 1, desc = "使用新分隔符查询对象列表")
		public void listObjectsV2() {
			result = s3Client.listObjectsV2(req);
		}

		@ExecuteOrder(step = 2, desc = "检查结果")
		public void checkResult() {
			// 手工校验查询方式为元数据扫描方式
			List<String> commprefixesResult = result.getCommonPrefixes();
			// TODO:2、测试结果覆盖下所有检查项，包括content记录，这里建议覆盖对象在context的场景
			// check result
			expresultList = ObjectUtils.getCommPrefixes(objectNames, "", delimiter);
			ObjectUtils.checkListObjectsV2Commprefixes(commprefixesResult, expresultList);
		}
	}
}
