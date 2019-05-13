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
 * test content: 带prefix、start-after、delimiter在设置continueation-token前后匹配条件不一致
 * testlink-case: seqDB-18128
 * 
 * @author wangkexin
 * @Date 2019.04.23
 * @version 1.00
 */

public class ListObjectsWithDelimiter18128 extends S3TestBase {
	private String bucketName = "bucket18128";
	private List<String> keyList = new ArrayList<String>();
	private String keyNamePrefix[] = { "dir1?dir", "dir2?dir3/dir" };
	private String prefix[] = { "dir1?", "dir2?" };
	private String delimiter = "?";
	private String startAfter[] = { "dir1?dir10?", "dir2?dir3/dir100?" };
	private int objectNum[] = { 1500, 500 };
	private List<String> expresultList1 = new ArrayList<String>();
	private List<String> expresultList2 = new ArrayList<String>();
	private int objectOnceQueryNum = 1000;
	private AmazonS3 s3Client = null;
	private boolean runSuccess = false;

	@BeforeClass
	private void setUp() throws Exception {
		s3Client = CommLib.buildS3Client();
		CommLib.clearBucket(s3Client, bucketName);
		// create bucket and set bucket version status
		s3Client.createBucket(new CreateBucketRequest(bucketName));
		CommLib.setBucketVersioning(s3Client, bucketName, "Enabled");

		// put multiple objects
		for (int j = 0; j < objectNum[0]; j++) {
			String currentKey = keyNamePrefix[0] + j + "?test18128";
			s3Client.putObject(bucketName, currentKey, "object_file18128");
			keyList.add(currentKey);
		}

		for (int j = 0; j < objectNum[1]; j++) {
			String currentKey = keyNamePrefix[1] + j + "?test18128";
			s3Client.putObject(bucketName, currentKey, "object_file18128");
			keyList.add(currentKey);
		}

		String[] objectNames = new String[keyList.size()];
		expresultList1 = ObjectUtils.getCommPrefixes(keyList.toArray(objectNames), prefix[0], delimiter);
		expresultList2 = ObjectUtils.getCommPrefixes(keyList.toArray(objectNames), prefix[1], delimiter);

		Collections.sort(expresultList1);
		Collections.sort(expresultList2);
	}

	@Test
	public void testGetObjectList() throws Exception {
		// 将分隔符设置为? （默认为'/'）
		DelimiterUtils.putBucketDelimiter(bucketName, delimiter);
		DelimiterUtils.checkCurrentDelimiteInfo(bucketName, delimiter);

		// First query
		ListObjectsV2Request req = new ListObjectsV2Request().withBucketName(bucketName).withPrefix(prefix[0])
				.withDelimiter(delimiter).withStartAfter(startAfter[0]);
		ListObjectsV2Result result = s3Client.listObjectsV2(req);
		List<String> commprefixesResult = result.getCommonPrefixes();
		// 取出expresultList1从startAfter开始的1000条commprefixes记录
		int startIndex = expresultList1.indexOf(startAfter[0]) + 1;
		expresultList1 = expresultList1.subList(startIndex, startIndex + objectOnceQueryNum);
		ObjectUtils.checkListObjectsV2Commprefixes(commprefixesResult, expresultList1);

		// Second query
		String nextContinuationToken = result.getNextContinuationToken();
		ListObjectsV2Request req2 = new ListObjectsV2Request().withBucketName(bucketName).withPrefix(prefix[1])
				.withDelimiter(delimiter).withStartAfter(startAfter[1]).withContinuationToken(nextContinuationToken);
		ListObjectsV2Result result2 = s3Client.listObjectsV2(req2);
		// TODO:1、第二次查询需要校验所有项结果，第一次查询没有匹配完成的记录，第二次查询如果不匹配当前delimiter，应该显示在version中
		List<String> commprefixesResult2 = result2.getCommonPrefixes();
		expresultList2 = expresultList2.subList(expresultList2.indexOf(startAfter[1]) + 1, expresultList2.size());
		ObjectUtils.checkListObjectsV2Commprefixes(commprefixesResult2, expresultList2);

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
