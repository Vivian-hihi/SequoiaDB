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
 * @Description: seqDB-16414 :: 指定nextVersionIdMarker匹配记录被删除，查询版本列表信息
 * @author fanyu
 * @Date:2018年11月23日
 * @version:1.0
 */

public class ListVersionsByNextKeyMaxKey16414 extends S3TestBase {
    private boolean runSuccess = false;
    private String bucketName = "bucket16414";
    private String[] objectNames = {"16414/123", "16414/456", "16414/789", "16414/ABC", "16414/DEF"};
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

    @Test(enabled = false)//bug:3986
    private void test() throws Exception {
        int index = 0;
        String keyMarker = objectNames[index];
        int versionIdMarker = versionNum;
        Integer maxResults = 7;

        VersionListing vsList = listVersions(bucketName, keyMarker, "" + versionIdMarker, maxResults);
        String[] expVersions = new String[]{"2", "1", "0", "2", "1", "0", "2"};
        List<String> expKeys = new ArrayList<String>();
        for(int i = 0; i < 3; i++){
            expKeys.add(objectNames[i]);
        }
        if (vsList.isTruncated()) {
            checkResult(vsList, null, new ArrayList<String>(), expKeys, expVersions);
        } else {
            Assert.fail("vsList.isTruncated() must be true");
        }
        System.out.println("==============================================");
        String nextKeyMarker = vsList.getNextKeyMarker();
        String nextVersionIdMarker = String.valueOf(1);

//        System.out.println("nextKeyMarker = " + nextKeyMarker);
//        System.out.println("nextVersionIdMarker = " + nextVersionIdMarker);
        //delete version
        s3Client.deleteVersion(bucketName, nextKeyMarker, nextVersionIdMarker);

        VersionListing vsList1 = listVersions(bucketName, nextKeyMarker, nextVersionIdMarker, maxResults);
        String[] expVersions1 = new String[]{"0", "2", "1", "0", "2", "1", "0"};
        List<String> expKeys1 = new ArrayList<String>();
        for(int i = 3; i < objectNames.length; i++){
            expKeys1.add(objectNames[i]);
        }
        if (vsList1.isTruncated()) {
            checkResult(vsList1, null, new ArrayList<String>(), expKeys1, expVersions1);
        } else {
            Assert.fail("vsList1.isTruncated() must be false");
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

    private void checkResult(VersionListing vsList, String delimiter, List<String> commonPrefixes, List<String> expKeys, String[] expVersions) throws Exception {
        Assert.assertEquals(vsList.getBucketName(), bucketName);
        Assert.assertEquals(vsList.getDelimiter(), delimiter);
        List<String> actCommonPrefixes = vsList.getCommonPrefixes();
        Assert.assertEquals(actCommonPrefixes.size(), commonPrefixes.size());
        Assert.assertEquals(actCommonPrefixes, commonPrefixes,
                "actCommonPrefixes = " + actCommonPrefixes.toString() + ",expCommonPrefixes=" + commonPrefixes.toString());
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

    private VersionListing listVersions(String bucketName, String keyMarker, String versionIdMarker, Integer maxResults) {
        ListVersionsRequest request = new ListVersionsRequest();
        request.setBucketName(bucketName);
        request.setKeyMarker(keyMarker);
        request.setMaxResults(maxResults);
        request.setVersionIdMarker(versionIdMarker);
        return s3Client.listVersions(request);
    }
}
