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
 * @Description:  seqDB-16408 :: 带delimiter和maxkeys查询对象版本列表，不匹配delimiter
 * @author fanyu
 * @Date:2018年11月23日
 * @version:1.0
 */

public class ListVersionsByDelimiterMaxKeys16408 extends S3TestBase {
    private boolean runSuccess = false;
    private String bucketName = "bucket16408";
    private String[] objectNames = {"aaa/16408", "bbb/16408", "ccc16408", "ddd16408","eee16408"};
    private AmazonS3 s3Client = null;
    private int versionNum = 5;

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
        String delimiter = "#";
        Integer maxResults = versionNum*objectNames.length;
        //maxResults = versionNum*objectNames.length
        VersionListing vsList = listVersionsByDelimiterMaxKeys(bucketName, delimiter, maxResults);
        List<String> expKeys = new ArrayList<String>();
        for(String objectName: objectNames){
            expKeys.add(objectName);
        }
        if (!vsList.isTruncated()) {
            checkResult(vsList, delimiter,new ArrayList<String>(), expKeys);
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

    private VersionListing listVersionsByDelimiterMaxKeys(String bucketName, String delimiter, Integer maxResults) {
        ListVersionsRequest request = new ListVersionsRequest();
        request.setBucketName(bucketName);
        request.setDelimiter(delimiter);
        request.setMaxResults(maxResults);
        return s3Client.listVersions(request);
    }
}
