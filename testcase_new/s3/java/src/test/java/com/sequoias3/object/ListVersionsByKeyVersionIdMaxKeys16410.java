package com.sequoias3.object;

import com.amazonaws.services.s3.AmazonS3;
import com.amazonaws.services.s3.model.BucketVersioningConfiguration;
import com.amazonaws.services.s3.model.ListVersionsRequest;
import com.amazonaws.services.s3.model.S3VersionSummary;
import com.amazonaws.services.s3.model.VersionListing;
import com.sequoias3.testcommon.CommLib;
import com.sequoias3.testcommon.S3TestBase;
import org.testng.Assert;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import java.io.IOException;
import java.util.ArrayList;
import java.util.List;
import java.util.UUID;

/**
 * @Description:  seqDB-16410 :: 带keyMarker、versionIdMarker和maxkeys查询对象版本列表
 * @author fanyu
 * @Date:2018年11月23日
 * @version:1.0
 */

public class ListVersionsByKeyVersionIdMaxKeys16410 extends S3TestBase {
    private boolean runSuccess1 = false;
    private boolean runSuccess2 = false;
    private boolean runSuccess3 = false;
    private String bucketName = "bucket16409";
    private String[] objectNames = {"123#16409", "234#16409", "345#16409", "456#16409","567#16409"};
    private AmazonS3 s3Client = null;
    private int versionNum = 10;

    @BeforeClass
    private void setUp() throws IOException {
        s3Client = CommLib.buildS3Client();
        CommLib.clearBucket(s3Client, bucketName);
        s3Client.createBucket(bucketName);
        CommLib.setBucketVersioning(s3Client, bucketName, BucketVersioningConfiguration.ENABLED);
        for (String objectName : objectNames) {
            for (int j = 0; j < versionNum; j++) {
                s3Client.putObject(bucketName, objectName, "" + UUID.randomUUID());
            }
        }
    }

    @Test
    private void testHead() throws Exception {
        String keyMarker = "0";
        String versionIdMarker = "0";
        Integer maxResults = versionNum*objectNames.length + 1;
        VersionListing vsList = listVersions(bucketName, keyMarker,versionIdMarker, maxResults);
        List<String> expKeys = new ArrayList<String>();
        for(String objectName : objectNames){
            expKeys.add(objectName);
        }
        if (!vsList.isTruncated()) {
            checkResult(vsList, null, new ArrayList<String>(),expKeys,versionNum*objectNames.length);
        } else {
            Assert.fail("vsList.isTruncated() must be false");
        }
        runSuccess1 = true;
    }

    @Test
    private void testTail1() throws Exception {
        String keyMarker = objectNames[objectNames.length-1];
        String versionIdMarker = "1";
        Integer maxResults = 2;
        VersionListing vsList = listVersions(bucketName, keyMarker,versionIdMarker, maxResults);
        List<String> expKeys = new ArrayList<String>();
        expKeys.add(objectNames[objectNames.length-1]);
        if (!vsList.isTruncated()) {
            checkResult(vsList, null, new ArrayList<String>(), expKeys,1);
        } else {
            Assert.fail("vsList.isTruncated() must be false");
        }
        runSuccess2 = true;
    }

    @Test
    private void testTail2() throws Exception {
        String keyMarker = "567#16409";
        String versionIdMarker = "1";
        Integer maxResults = 1;
        VersionListing vsList = listVersions(bucketName, keyMarker,versionIdMarker, maxResults);
        List<String> expKeys = new ArrayList<String>();
        expKeys.add(objectNames[objectNames.length-1]);
        if (!vsList.isTruncated()) {
            checkResult(vsList, null, new ArrayList<String>(), expKeys,1);
        } else {
            Assert.fail("vsList.isTruncated() must be false");
        }
        runSuccess3 = true;
    }

    @AfterClass
    private void tearDown() {
        try {
            if (runSuccess1 && runSuccess2 && runSuccess3) {
                CommLib.clearBucket(s3Client, bucketName);
            }
        } finally {
            if (s3Client != null) {
                s3Client.shutdown();
            }
        }
    }

    private void checkResult(VersionListing vsList, String delimiter, List<String> commonPrefixes, List<String> expKeys,int expVersionsNum) throws Exception {
        Assert.assertEquals(vsList.getBucketName(), bucketName);
        Assert.assertEquals(vsList.getDelimiter(), delimiter);
        List<String> actCommonPrefixes = vsList.getCommonPrefixes();
        Assert.assertEquals(actCommonPrefixes.size(), commonPrefixes.size());
        Assert.assertEquals(actCommonPrefixes,commonPrefixes,
                "actCommonPrefixes = " + actCommonPrefixes.toString() + ",expCommonPrefixes="+commonPrefixes.toString());
        List<S3VersionSummary> vsSummaryList = vsList.getVersionSummaries();
        Assert.assertEquals(vsSummaryList.size(), expVersionsNum);
        String key = "";
        List<String> actKeys = new ArrayList<String>();
        for(int i = 0; i < vsSummaryList.size(); i++){
            S3VersionSummary versionSummary = vsSummaryList.get(i);
            Assert.assertEquals(versionSummary.getBucketName(), bucketName);
            if(!key.equals(versionSummary.getKey())){
                actKeys.add(versionSummary.getKey());
            }
            key = versionSummary.getKey();
        }
        Assert.assertEquals(actKeys.toString(),expKeys.toString(),"actObjectNames = " + actKeys + ",keys = " + expKeys);
    }

    private VersionListing listVersions(String bucketName, String keyMarker,String versionIdMarker, Integer maxResults) {
        ListVersionsRequest request = new ListVersionsRequest();
        request.setBucketName(bucketName);
        request.setKeyMarker(keyMarker);
        request.setVersionIdMarker(versionIdMarker);
        request.setMaxResults(maxResults);
        return s3Client.listVersions(request);
    }
}
