package com.sequoias3.delimiter;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import com.amazonaws.services.s3.AmazonS3;
import com.amazonaws.services.s3.model.CreateBucketRequest;
import com.amazonaws.services.s3.model.ListObjectsV2Request;
import com.amazonaws.services.s3.model.ListObjectsV2Result;
import com.sequoias3.testcommon.CommLib;
import com.sequoias3.testcommon.S3TestBase;
import com.sequoias3.testcommon.s3utils.DelimiterUtils;
import com.sequoias3.testcommon.s3utils.ObjectUtils;

/**
 * test content: 指定nextContinuationToken匹配记录被删除，查询列表元数据 testlink-case:
 * seqDB-18131
 * 
 * @author wangkexin
 * @Date 2019.04.23
 * @version 1.00
 */

public class ListObjectsWithDelimiter18131 extends S3TestBase {
	private String bucketName = "bucket18131";
	private String[] objectNames = { "dir1/test18131_1", "dir1/dir2/test18131_2", "dir/aa/test18131_3", "test18131_4" };
	private String delimiter = "test";
	private int maxkeys = 2;
	private List<String> expresultList = new ArrayList<String>();
	private AmazonS3 s3Client = null;
	private boolean runSuccess = false;

	@BeforeClass
	private void setUp() throws Exception {
		s3Client = CommLib.buildS3Client();
		s3Client.createBucket(new CreateBucketRequest(bucketName));

		for (int i = 0; i < objectNames.length; i++) {
			s3Client.putObject(bucketName, objectNames[i], "object_file18131");
		}
	}

	@Test
	public void testGetObjectList() throws Exception {
		// 将分隔符设置为test （默认为'/'）
		DelimiterUtils.putBucketDelimiter(bucketName, delimiter);
		DelimiterUtils.checkCurrentDelimiteInfo(bucketName, delimiter);

		expresultList = ObjectUtils.getCommPrefixes(objectNames, "", delimiter);
		Collections.sort(expresultList);
		List<String> commprefixesResult = new ArrayList<>();
		ListObjectsV2Request req = new ListObjectsV2Request().withBucketName(bucketName).withDelimiter(delimiter)
				.withMaxKeys(maxkeys);
		ListObjectsV2Result result;

		result = s3Client.listObjectsV2(req);
		commprefixesResult.addAll(result.getCommonPrefixes());

		// 删除下一个匹配到的记录
		// TODO:1、用例中是要删除nextToken匹配的记录，可以通过获取匹配是哪条记录
		String nextCommprefix = expresultList.get(2);
		String tobeDeleteKey = "";
		for (int i = 0; i < objectNames.length; i++) {
			if (objectNames[i].startsWith(nextCommprefix)) {
				tobeDeleteKey = objectNames[i];
				expresultList.remove(2);
				break;
			}
		}
		s3Client.deleteObject(bucketName, tobeDeleteKey);

		String nextContinuationToken = result.getNextContinuationToken();
		ListObjectsV2Request req2 = new ListObjectsV2Request().withBucketName(bucketName).withDelimiter(delimiter)
				.withMaxKeys(maxkeys).withContinuationToken(nextContinuationToken);
		ListObjectsV2Result result2 = s3Client.listObjectsV2(req2);
		commprefixesResult.addAll(result2.getCommonPrefixes());

		// check result
		// TODO:2、测试结果需要覆盖所有检查项
		ObjectUtils.checkListObjectsV2Commprefixes(commprefixesResult, expresultList);
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
