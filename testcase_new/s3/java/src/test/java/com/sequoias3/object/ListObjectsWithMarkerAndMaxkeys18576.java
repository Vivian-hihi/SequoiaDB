package com.sequoias3.object;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

import org.testng.Assert;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import com.amazonaws.services.s3.AmazonS3;
import com.amazonaws.services.s3.model.ListObjectsRequest;
import com.amazonaws.services.s3.model.ObjectListing;
import com.amazonaws.services.s3.model.S3ObjectSummary;
import com.sequoias3.testcommon.CommLib;
import com.sequoias3.testcommon.S3TestBase;

/**
 * @Description seqDB-18576: To get a list by listObjectV1.specify matching
 *              marker and maxkeys
 * @author wuyan
 * @Date 2019.06.20
 * @version 1.00
 */
public class ListObjectsWithMarkerAndMaxkeys18576 extends S3TestBase {
	private boolean runSuccess = false;
	private String bucketName = "bucket18576";
	private String key = "/aa//bb/object18576.png";
	private AmazonS3 s3Client = null;
	private int objectNums = 30;

	@BeforeClass
	private void setUp() {
		s3Client = CommLib.buildS3Client();
		CommLib.clearBucket(s3Client, bucketName);
		s3Client.createBucket(bucketName);
	}

	@Test
	public void testListObjects() throws Exception {
		List<String> keyList = putObjects();
		// test a: starting with the first record
		int startPositionA = 0;
		int maxKeysA = 3;
		listObjectsAndCheckResult(keyList, startPositionA, maxKeysA);

		// test b: show only the last object
		int startPositionB = objectNums - 2;
		int maxKeysB = 2;
		// TODO :文本用例期望的结果中，对于场景b，返回的结果数应为1,而不是指定的maxkeys记录数，请修改对应文本用例
		listObjectsAndCheckResult(keyList, startPositionB, maxKeysB);

		// test c:starting with the last one
		int startPositionC = objectNums - 1;
		int maxKeysC = 1;
		listObjectsAndCheckResult(keyList, startPositionC, maxKeysC);

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

	private void listObjectsAndCheckResult(List<String> keyList, int startPosition, int maxKeys) {
		List<String> expKeyList = new ArrayList<>(keyList);
		String marker = expKeyList.get(startPosition);
		ListObjectsRequest request = new ListObjectsRequest().withBucketName(bucketName).withMarker(marker)
				.withMaxKeys(maxKeys);
		ObjectListing result;
		List<String> queryKeyList = new ArrayList<>();
		List<String> commonPrefixes = new ArrayList<>();
		do {
			result = s3Client.listObjects(request);
			List<String> oneGetCommPrefixes = result.getCommonPrefixes();
			commonPrefixes.addAll(oneGetCommPrefixes);
			List<S3ObjectSummary> objects = result.getObjectSummaries();
			List<String> oneQueryKeyList = new ArrayList<>();
			for (S3ObjectSummary os : objects) {
				String key = os.getKey();
				oneQueryKeyList.add(key);
				queryKeyList.add(key);
			}
			String nextMarker = result.getNextMarker();
			request.setMarker(nextMarker);
			int eachListNums = oneGetCommPrefixes.size() + oneQueryKeyList.size();
			// TODO
			// :下面这个判断对“当maxKeys小于匹配数，而listObjects一次就返回的结果数eachListNums小于maxKeys”这种情况无法察觉
			if (eachListNums > maxKeys) {
				Assert.fail("list nums error! commonPrefixes: " + oneGetCommPrefixes.toString() + "  contents:"
						+ oneQueryKeyList.toString());
			}

		} while (result.isTruncated());

		// TODO :注释有误 teh -> the
		// check teh commonprefixes is null.
		List<String> expCommonPrefixes = new ArrayList<>();
		Assert.assertEquals(commonPrefixes, expCommonPrefixes);

		// check the keyName
		if (startPosition == objectNums - 1) {
			// marker starting with the last one, list keys is 0
			Assert.assertEquals(queryKeyList.size(), 0, "query keylist is :" + queryKeyList.toString());
		} else {
			for (int i = 0; i < startPosition + 1; i++) {
				expKeyList.remove(0);
			}
			Assert.assertEquals(queryKeyList, expKeyList);
		}

	}

	private List<String> putObjects() {
		List<String> keyList = new ArrayList<>();
		for (int i = 0; i < objectNums; i++) {
			String keyName = key + "_" + i;
			s3Client.putObject(bucketName, keyName, "test18576" + i);
			keyList.add(keyName);
		}
		Collections.sort(keyList);
		return keyList;
	}
}
