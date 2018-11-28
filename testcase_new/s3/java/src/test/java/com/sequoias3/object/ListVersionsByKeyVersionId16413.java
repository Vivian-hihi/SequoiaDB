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
 * @Description:  seqDB-16413 ::带keyMarker、versionIdMarker查询对象版本列表，不匹配
 * @author fanyu
 * @Date:2018年11月23日
 * @version:1.0
 */

public class ListVersionsByKeyVersionId16413 extends S3TestBase {
    private boolean runSuccess = false;
    private String bucketName = "bucket16413";
    private String[] objectNames = {"16413/abc", "16413/bcd", "16413/cde", "16413/def","16413/efg"};
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
              s3Client.putObject(bucketName, objectName, "" + UUID.randomUUID());
            }
        }
    }

    @Test
    private void test() throws Exception {
        int index = objectNames.length-1;
        String keyMarker = objectNames[index];
        int versionIdMarker = versionNum-1;
        VersionListing vsList = listVersions(bucketName, keyMarker,""+versionIdMarker);
        String[] expVersions = new String[]{"1","0"};
        List<String> expKeys = new ArrayList<String>();
        expKeys.add(objectNames[index]);
        if (!vsList.isTruncated()) {
            checkResult(vsList, null, new ArrayList<String>(),expKeys,expVersions);
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

    private void checkResult(VersionListing vsList, String delimiter, List<String> commonPrefixes,List<String> expKeys,String[] expVersions) throws Exception {
        Assert.assertEquals(vsList.getBucketName(), bucketName);
        Assert.assertEquals(vsList.getDelimiter(), delimiter);
        List<String> actCommonPrefixes = vsList.getCommonPrefixes();
        Assert.assertEquals(actCommonPrefixes.size(), commonPrefixes.size());
        Assert.assertEquals(actCommonPrefixes,commonPrefixes,
                "actCommonPrefixes = " + actCommonPrefixes.toString() + ",expCommonPrefixes="+commonPrefixes.toString());
        List<S3VersionSummary> vsSummaryList = vsList.getVersionSummaries();
        Assert.assertEquals(vsSummaryList.size(), expVersions.length);
        String key = "";
        List<String> actKeys = new ArrayList<String>();
        for(int i = 0; i < vsSummaryList.size(); i++){
            S3VersionSummary versionSummary = vsSummaryList.get(i);
            Assert.assertEquals(versionSummary.getBucketName(), bucketName);
            Assert.assertEquals(versionSummary.getVersionId(), expVersions[i],
                    "bucketName = " + bucketName+",key = " + versionSummary.getKey() +
                            ",versionId = " +  versionSummary.getVersionId());
            if(!key.equals(versionSummary.getKey())){
                actKeys.add(versionSummary.getKey());
            }
            key = versionSummary.getKey();
        }
        Assert.assertEquals(actKeys.toString(),expKeys.toString(),"actObjectNames = " + actKeys + ",keys = " + expKeys);
    }

    private VersionListing listVersions(String bucketName, String keyMarker,String versionIdMarker) {
        ListVersionsRequest request = new ListVersionsRequest();
        request.setBucketName(bucketName);
        request.setKeyMarker(keyMarker);
        request.setVersionIdMarker(versionIdMarker);
        return s3Client.listVersions(request);
    }
}
