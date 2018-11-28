package com.sequoias3.object;

import com.amazonaws.services.s3.AmazonS3;
import com.amazonaws.services.s3.model.*;
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
 * @Description: seqDB-16412 :: 带keyMarker、versionIdMarker查询对象版本列表，不匹配keyMarker
 * @author fanyu
 * @Date:2018年11月23日
 * @version:1.0
 */

public class ListVersionsByKeyVersionId16412 extends S3TestBase {
    private boolean runSuccess = false;
    private String bucketName = "bucket16412";
    private String[] objectNames = {"aaa/16412", "bbb/16412", "ccc/16412"};
    private List<PutObjectResult> objectList = new ArrayList<PutObjectResult>();
    private AmazonS3 s3Client = null;
    private int versionNum = 3;

    @BeforeClass
    private void setUp() throws IOException {
        s3Client = CommLib.buildS3Client();
        CommLib.clearBucket(s3Client, bucketName);
        s3Client.createBucket(bucketName);
        CommLib.setBucketVersioning(s3Client, bucketName, BucketVersioningConfiguration.ENABLED);
        for (String objectName : objectNames) {
            for (int j = 0; j < versionNum; j++) {
                PutObjectResult object = s3Client.putObject(bucketName, objectName, "" + UUID.randomUUID());
                objectList.add(object);
            }
        }
    }

    @Test
    private void test() throws Exception {
        String keyMarker = objectNames[objectNames.length-1];
        int versionIdMarker = 0;
        VersionListing vsList = listVersions(bucketName, keyMarker,""+versionIdMarker);
        if (!vsList.isTruncated()) {
            checkResult(vsList, null, new ArrayList<String>(),new String[]{});
        } else {
            Assert.fail("vsList.isTruncated() must be false");
        }
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

    private void checkResult(VersionListing vsList, String delimiter, List<String> commonPrefixes, String[] expVersions) throws Exception {
        Assert.assertEquals(vsList.getBucketName(), bucketName);
        Assert.assertEquals(vsList.getDelimiter(), delimiter);
        List<String> actCommonPrefixes = vsList.getCommonPrefixes();
        Assert.assertEquals(actCommonPrefixes.size(), commonPrefixes.size());
        Assert.assertEquals(actCommonPrefixes,commonPrefixes,
                "actCommonPrefixes = " + actCommonPrefixes.toString() + ",expCommonPrefixes="+commonPrefixes.toString());
        List<S3VersionSummary> vsSummaryList = vsList.getVersionSummaries();
        Assert.assertEquals(vsSummaryList.size(), expVersions.length);
    }

    private VersionListing listVersions(String bucketName, String keyMarker,String versionIdMarker) {
        ListVersionsRequest request = new ListVersionsRequest();
        request.setBucketName(bucketName);
        request.setKeyMarker(keyMarker);
        request.setVersionIdMarker(versionIdMarker);
        return s3Client.listVersions(request);
    }
}
