package com.sequoias3.delimiter;

import java.io.IOException;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

import org.springframework.util.LinkedMultiValueMap;
import org.springframework.util.MultiValueMap;
import org.testng.Assert;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import com.amazonaws.services.s3.AmazonS3;
import com.amazonaws.services.s3.model.BucketVersioningConfiguration;
import com.amazonaws.services.s3.model.ListVersionsRequest;
import com.amazonaws.services.s3.model.VersionListing;
import com.sequoias3.testcommon.CommLib;
import com.sequoias3.testcommon.S3TestBase;
import com.sequoias3.testcommon.s3utils.DelimiterUtils;
import com.sequoias3.testcommon.s3utils.ObjectUtils;

/**
 * @Description: seqDB-18155 :To get a listVersions within a bucket.specify
 *               prefix/keyMarker/delimiter/versionIdMarker.
 * @author wuyan
 * @Date:2019.4.24
 * @version:1.0
 */

public class ListVersions18155 extends S3TestBase {
	private boolean runSuccess = false;
	private String bucketName = "bucket18155";
	private String delimiter = "test";
	private AmazonS3 s3Client = null;
	private int versionNum = 3;
	private String[] keyNames = { "atest_18155", "dir1?test1_18155.png", "dir1??dir2??/dir3/test2_18155",
			"dir1?test3_18155", "dir1?dir2?aa?test4_18155", "/dir1/test_18155", "dir1?test5_18155",
			"dir1?dir2?aa?dd?test6_18155", "dir1_18155", "testdir1.txt_18155" };

	@BeforeClass
	private void setUp() throws IOException {
		s3Client = CommLib.buildS3Client();
		CommLib.clearBucket(s3Client, bucketName);
		s3Client.createBucket(bucketName);
		CommLib.setBucketVersioning(s3Client, bucketName, BucketVersioningConfiguration.ENABLED);

		for (int i = 0; i < keyNames.length; i++) {
			s3Client.putObject(bucketName, keyNames[i], "testContest_" + keyNames[i] + "_version1");
			s3Client.putObject(bucketName, keyNames[i], "testContest_" + keyNames[i] + "_version1");
			s3Client.putObject(bucketName, keyNames[i], "testContest_" + keyNames[i] + "_version3");
		}

		DelimiterUtils.putBucketDelimiter(bucketName, delimiter);
	}

	@Test
	private void testListVersions() throws Exception {

		String prefix = "dir1";
		String keyMarker = keyNames[0];
		String versionIdMarker = "2";

		// list versions by prefix/delimiter/versionIdMarker/keyMarker
		VersionListing vsList = s3Client
				.listVersions(new ListVersionsRequest().withBucketName(bucketName).withDelimiter(delimiter)
						.withPrefix(prefix).withKeyMarker(keyMarker).withVersionIdMarker(versionIdMarker));

		// expected results
		List<String> matchPrefixList = new ArrayList<>();
		matchPrefixList.add("dir1?test");
		matchPrefixList.add("dir1??dir2??/dir3/test");
		matchPrefixList.add("dir1?dir2?aa?test");
		matchPrefixList.add("dir1?dir2?aa?dd?test");

		Collections.sort(matchPrefixList);
		MultiValueMap<String, String> expVersions = new LinkedMultiValueMap<String, String>();

		// expContent:"dir1_18155",the versionId is:2,1,0
		for (int i = versionNum - 1; i >= 0; i--) {
			expVersions.add(keyNames[8], String.valueOf(i));
		}
		// check
		Assert.assertEquals(vsList.isTruncated(), false, "vsList.isTruncated() must be false");
		ObjectUtils.checkListVSResults(vsList, matchPrefixList, expVersions);

		runSuccess = true;
	}

	@AfterClass
	private void tearDown() {
		try {
			if (runSuccess) {
				CommLib.clearBucket(s3Client, bucketName);
			}
		} finally {
			if (s3Client != null) {
				s3Client.shutdown();
			}
		}
	}
}
