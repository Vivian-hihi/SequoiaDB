package com.sequoias3.object;

import com.amazonaws.services.s3.AmazonS3;
import com.amazonaws.services.s3.model.*;
import com.sequoias3.testcommon.CommLib;
import com.sequoias3.testcommon.S3TestBase;
import com.sequoias3.testcommon.TestTools;
import org.testng.Assert;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import java.io.File;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Date;
import java.util.List;
import java.util.UUID;

/**
 * @Description: seqDB-16407 :: 带delimiter和maxkeys查询对象版本列表
 * @author fanyu
 * @Date:2018年11月23日
 * @version:1.0
 */

public class ListVersionsByDelimiterMaxKeys16407 extends S3TestBase {
    private boolean runSuccess1 = false;
    private boolean runSuccess2 = false;
    private String bucketName = "bucket16407";
    private String[] objectNames = {"air/16407", "dir/16407", "fire16407", "test16407"};
    private AmazonS3 s3Client = null;
    private int versionNum = 4;

    @BeforeClass
    private void setUp() throws IOException {
        s3Client = CommLib.buildS3Client();
        CommLib.clearBucket(s3Client,bucketName);
        s3Client.createBucket(bucketName);
        CommLib.setBucketVersioning(s3Client,bucketName, BucketVersioningConfiguration.ENABLED);
        for (String objectName : objectNames) {
            for (int j = 0; j < versionNum; j++) {
                s3Client.putObject(bucketName, objectName, "" + UUID.randomUUID());
            }
        }
    }

    @Test
    private void testGtMaxKeys() throws Exception {
        String delimiter = "/";
        String keyMarker = "acr";
        String versionIdMarker = "1";

        //maxResults > versionNum*objectNames.length
        VersionListing vsList = listVersionsByDelimiterMaxKeys(bucketName, delimiter, versionNum * objectNames.length, keyMarker, versionIdMarker);
        List<String> expCommonPrefixes = new ArrayList<String>();
        expCommonPrefixes.add("air/");
        expCommonPrefixes.add("dir/");
        List<String> expKeys = new ArrayList<String>();
        expKeys.add("fire16407");
        expKeys.add("test16407");
        if (!vsList.isTruncated()) {
            checkResult(vsList, delimiter, expCommonPrefixes,expKeys);
        } else {
            Assert.fail("vsList.isTruncated() must be false");
        }
        runSuccess1 = true;
    }

    @Test
    private void testLtMaxKeys() throws Exception {
        String delimiter = "/";
        String keyMarker = "abc";
        String versionIdMarker = "1";

        //maxResults < versionNum*objectNames.length
        VersionListing vsList = listVersionsByDelimiterMaxKeys(bucketName, delimiter, 1, keyMarker, versionIdMarker);
        List<String> expCommonPrefixes1 = new ArrayList<String>();
        expCommonPrefixes1.add("air/");
        if (vsList.isTruncated()) {
            checkResult(vsList, delimiter, expCommonPrefixes1,new ArrayList<String>());
        } else {
            Assert.fail("vsList.isTruncated() must be true");
        }

        //maxResults > versionNum*objectNames.length
        String nextKeyMarker = vsList.getNextKeyMarker();
        String nestVersionIdMarker = vsList.getNextVersionIdMarker();
        VersionListing vsList1 = listVersionsByDelimiterMaxKeys(bucketName, delimiter, 10, nextKeyMarker, nestVersionIdMarker);
        List<String> expCommonPrefixes2 = new ArrayList<String>();
        expCommonPrefixes2.add("dir/");
        List<String> expKeys = new ArrayList<String>();
        expKeys.add("fire16407");
        expKeys.add("test16407");
        if (!vsList1.isTruncated()) {
            checkResult(vsList1, delimiter,expCommonPrefixes2,expKeys);
        } else {
            Assert.fail("vsList.isTruncated() must be false");
        }
        runSuccess2 = true;
    }

    @AfterClass
    private void tearDown() {
        try {
            if (runSuccess1 && runSuccess2) {
                CommLib.clearBucket(s3Client, bucketName);
            }
        } finally {
            if (s3Client != null) {
                s3Client.shutdown();
            }
        }
    }

    private void checkResult(VersionListing vsList, String delimiter, List<String> commonPrefixes, List<String> expKeys) throws Exception {
        Assert.assertEquals(vsList.getBucketName(), bucketName);
        Assert.assertEquals(vsList.getDelimiter(), delimiter);
        List<String> actCommonPrefixes = vsList.getCommonPrefixes();
        Assert.assertEquals(actCommonPrefixes, commonPrefixes);
        List<S3VersionSummary> vsSummaryList = vsList.getVersionSummaries();
        Assert.assertEquals(vsSummaryList.size(),expKeys.size() * versionNum);
        String key = "";
        List<String> actKeys = new ArrayList<String>();
        for (S3VersionSummary versionSummary : vsSummaryList) {
            Assert.assertEquals(versionSummary.getBucketName(), bucketName);
            if(!key.equals(versionSummary.getKey())){
                actKeys.add(versionSummary.getKey());
            }
            key = versionSummary.getKey();
        }
        Assert.assertEquals(actKeys.toString(),expKeys.toString(),"actObjectNames = " + actKeys + ",keys = " + expKeys);
    }

    private VersionListing listVersionsByDelimiterMaxKeys(String bucketName, String delimiter, Integer maxResults, String keyMarker, String versionIdMarker) {
        ListVersionsRequest request = new ListVersionsRequest();
        request.setBucketName(bucketName);
        request.setDelimiter(delimiter);
        request.setMaxResults(maxResults);
        request.setKeyMarker(keyMarker);
        request.setVersionIdMarker(versionIdMarker);
        return s3Client.listVersions(request);
    }
}
