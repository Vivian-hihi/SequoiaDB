package com.sequoias3.delimiter;

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
import com.sequoias3.testcommon.s3utils.DelimiterUtils;

/**
 * @Description seqDB-18119: To get a list of objects within a bucket.specify
 *              matching delimiter and maxkeys. test a: maxkeys is less than the
 *              matching records test b: maxkeys is greater than the matching
 *              records
 * @author wuyan
 * @Date 2019.04.23
 * @version 1.00
 */
public class ListObjects18119 extends S3TestBase {
	private boolean runSuccess = false;
	private String bucketName = "bucket18119";
	private String keyName = "/object18119";
	private AmazonS3 s3Client = null;
	private int keyNums = 50;
	private String delimiter = "a";
	private List<String> keyList = new ArrayList<>();
	private List<String> noMatchDelimiterKeyList = new ArrayList<>();
	private List<String> matchDelimiterKeyList = new ArrayList<>();

	@BeforeClass
	private void setUp() throws IOException {
		s3Client = CommLib.buildS3Client();
		CommLib.clearBucket(s3Client, bucketName);
		s3Client.createBucket(bucketName);
	}

	@Test
	public void testListObjects() throws Exception {
		DelimiterUtils.putBucketDelimiter(bucketName, delimiter);
		putObjects(keyNums);

		// test a: match nums > maxkeys
		int maxKeysA = 3;
		listObjectsAndCheckResult(matchDelimiterKeyList, noMatchDelimiterKeyList, maxKeysA);

		// test b: match nums < maxkeys
		int maxKeysB = 51;
		listObjectsAndCheckResult(matchDelimiterKeyList, noMatchDelimiterKeyList, maxKeysB);

		// test c: match nums = maxkeys
		int maxKeysC = keyNums;
		listObjectsAndCheckResult(matchDelimiterKeyList, noMatchDelimiterKeyList, maxKeysC);
		runSuccess = true;
	}

	@AfterClass
	private void tearDown() {
		try {
			if (runSuccess) {
				for (String keyName : keyList) {
					s3Client.deleteObject(bucketName, keyName);
				}
				s3Client.deleteBucket(bucketName);
			}
		} finally {
			s3Client.shutdown();
		}
	}

	private void listObjectsAndCheckResult(List<String> expCommprefixLists, List<String> expContentLists, int maxKeys)
			throws IOException {
		List<String> matchContentList = new ArrayList<>();
		List<String> matchDelimiterList = new ArrayList<>();
		ListObjectsV2Request request = new ListObjectsV2Request().withBucketName(bucketName).withEncodingType("url")
				.withDelimiter(delimiter).withMaxKeys(maxKeys);
		ListObjectsV2Result result;

		do {
			result = s3Client.listObjectsV2(request);
			List<String> commonPrefixes = result.getCommonPrefixes();
			matchDelimiterList.addAll(commonPrefixes);

			// objects do not match delimiter are displayed in contents,num is
			// less than Maxkeys
			List<S3ObjectSummary> objects = result.getObjectSummaries();
			// TODO ：5.为什么只是判断objects的size，而不去判断commprefixes的size？
			// 建议直接在最后比较结果集比较好，如果这里出错的话问题也不好定位，只能知道返回的contents中的条数不对，但不知道返回的具体记录是什么
			if (objects.size() > maxKeys) {
				Assert.fail("exceed the maximum number,  :" + objects.size());
			}

			List<String> queryKeyList = new ArrayList<>();
			for (S3ObjectSummary os : objects) {
				String key = os.getKey();
				queryKeyList.add(key);
			}
			matchContentList.addAll(queryKeyList);

			String continuationToken = result.getNextContinuationToken();
			request.setContinuationToken(continuationToken);

			int eachListNums = commonPrefixes.size() + queryKeyList.size();
			// TODO
			// :6.这里只是判断每次查询返回的结果是小于等于maxKeys的话，如果每次查询都只返回1条结果，而不是指定的maxKeys这样的情况就检测不到了
			if (eachListNums > maxKeys) {
				Assert.fail("list nums error! commonPrefixes: " + commonPrefixes.toString() + "  contents:"
						+ queryKeyList.toString());
			}
		} while (result.isTruncated());

		// check the commprefixList and contents
		Collections.sort(expCommprefixLists);
		Assert.assertEquals(matchDelimiterList, expCommprefixLists, "matchDelimiterList:"
				+ matchDelimiterList.toString() + "\n commprefixList:" + expCommprefixLists.toString());
		Collections.sort(expContentLists);
		Assert.assertEquals(matchContentList, expContentLists,
				"matchContents:" + matchContentList.toString() + "\n contentList:" + expContentLists.toString());

	}

	private void putObjects(int objectNums) {
		String subKeyName;
		for (int i = 0; i < objectNums; i++) {
			if (i % 2 == 0) {
				subKeyName = keyName + "/test" + "_" + i + "aa.png";
				matchDelimiterKeyList.add(keyName + "/test" + "_" + i + "a");
			} else {
				subKeyName = keyName + "_" + i;
				noMatchDelimiterKeyList.add(subKeyName);
			}
			s3Client.putObject(bucketName, subKeyName, "content_18119_" + i);
			keyList.add(subKeyName);
		}

		/*
		 * put one object match the same dir,eg: "/object18119/test_0aa1.png"
		 * and "/object18119/test_0aa.png",the dir is "/object18119/test_0a"
		 */
		String key = keyName + "/test_0aa1.png";
		s3Client.putObject(bucketName, key, "content_18119_test");
		keyList.add(key);
	}
}
