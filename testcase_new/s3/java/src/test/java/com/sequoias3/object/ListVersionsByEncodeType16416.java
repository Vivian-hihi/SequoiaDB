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
import java.net.URL;
import java.net.URLDecoder;
import java.net.URLEncoder;
import java.util.ArrayList;
import java.util.List;
import java.util.UUID;

/**
 * @Description: seqDB-16416 :: 指定encoding-type查询对象版本列表
 * @author fanyu
 * @Date:2018年11月26日
 * @version:1.0
 */

public class ListVersionsByEncodeType16416 extends S3TestBase {
    private boolean runSuccess1 = false;
    private boolean runSuccess2 = false;
    private String bucketName = "bucket16416";
    private String[] objectNames = {"BEL","16416!(16416 16416.txt).txt!","16416!-/_!","16416!.|*'!"};
    private AmazonS3 s3Client = null;
    private int versionNum = 2;

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
    private void testCommonPrefixes() throws Exception {
        String prefix = "16416!";
        String delimiter = "!";
        String encodingType = "url";
        VersionListing vsList = listVersions(bucketName,prefix, delimiter,encodingType);

        List<String> expCommonPrefixes = new ArrayList<String>();
        for(int i = 1;i < objectNames.length;i++){
            expCommonPrefixes.add(URLEncoder.encode(objectNames[i],"utf-8"));
        }
        checkResult(vsList, encodingType,expCommonPrefixes,new ArrayList<String>(),new String[]{});
        runSuccess1 = true;
    }

    @Test
    private void testVersions() throws Exception {
        String prefix = "16416!";
        String delimiter = "A/";
        String encodingType = "url";
        VersionListing vsList = listVersions(bucketName,prefix, delimiter,encodingType);

        List<String> expKeys = new ArrayList<String>();
        for(int i = 1;i < objectNames.length;i++){
            expKeys.add(URLEncoder.encode(objectNames[i],"utf-8"));
        }
        String[] expVersions = {"1","0","1","0","1","0"};
        if(!vsList.isTruncated()) {
            checkResult(vsList, encodingType, new ArrayList<String>(), expKeys, expVersions);
        }else{
            Assert.fail("vsList.isTruncated must be false");
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

    private void checkResult(VersionListing vsList,String encodingType, List<String> commonPrefixes, List<String> expKeys, String[] expVersions) throws Exception {
        Assert.assertEquals(vsList.getBucketName(), bucketName);
        Assert.assertEquals(vsList.getEncodingType(),encodingType);
        List<String> actCommonPrefixes = vsList.getCommonPrefixes();
        Assert.assertEquals(actCommonPrefixes.size(), commonPrefixes.size(),
                "actCommonPrefixes = " + actCommonPrefixes.toString()+",expCommonPrefixes = " + commonPrefixes.toString());
        Assert.assertEquals(actCommonPrefixes, commonPrefixes,
                "actCommonPrefixes = " + actCommonPrefixes.toString() + ",expCommonPrefixes=" + commonPrefixes.toString());
        List<S3VersionSummary> vsSummaryList = vsList.getVersionSummaries();
        Assert.assertEquals(vsSummaryList.size(), expVersions.length,"vsSummaryList = " + vsSummaryList.toString());
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

    private VersionListing listVersions(String bucketName, String prefix, String delimiter,String encodingType) {
        ListVersionsRequest request = new ListVersionsRequest();
        request.setBucketName(bucketName);
        request.setPrefix(prefix);
        request.setDelimiter(delimiter);
        request.setEncodingType(encodingType);
        return s3Client.listVersions(request);
    }
}
