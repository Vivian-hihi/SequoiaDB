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
 * @author fanyu
 * @Description: seqDB-16415 ::多次查询结果在commprefix中有相同记录
 * @Date:2018年11月24日
 * @version:1.0
 */

public class ListVersionsByPrefixDelimiterMaxkeys16415 extends S3TestBase {
    private boolean runSuccess = false;
    private String bucketName = "bucket16415";
    private String[] objectNames = {"/aa/bb/test1","/aa/bb/test2","/bb/cc/test1",
            "/bb/cc/test2","/cc/dd/test1","/cc/dd/test2"};
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
    //TODO:4、补充场景b测试点
    @Test//SEQUOIADBMAINSTREAM-3987
    private void test() throws Exception {
        String prefix = "/";
        String delimiter = "/";
        Integer maxResults = 1;

        VersionListing vsList = listVersions(bucketName,prefix ,delimiter, null,null,maxResults);
        List<String> expCommonPrefixes = new ArrayList<String>();
        expCommonPrefixes.add("/aa/");
       if (vsList.isTruncated()) {
            checkResult(vsList,expCommonPrefixes,new ArrayList<String>(),new String[]{});
        } else {
           Assert.fail("vsList.isTruncated() must be true");
       }


       //has same commonprefixes
        Integer maxResults2 = 2;
        VersionListing vsList2 = listVersions(bucketName,prefix ,delimiter, null,null,maxResults2);
        List<String> expCommonPrefixes2 = new ArrayList<String>();
        expCommonPrefixes2.add("/aa/");
        expCommonPrefixes2.add("/bb/");
        if (vsList.isTruncated()) {
        	//TODO:1、这里是测试场景a，返回记录数为0条，建议传入记录数会比较直观，或者增加描述信息。
            checkResult(vsList2,expCommonPrefixes2,new ArrayList<String>(),new String[]{});
        } else {
            Assert.fail("vsList1.isTruncated() must be true");
        }

        //test isTruncated
        String nextKeyMarker = vsList.getNextKeyMarker();
        String nextVersionIdMarker = vsList.getNextVersionIdMarker();
        Integer maxResults3 = 2;
        VersionListing vsList3 = listVersions(bucketName,prefix ,delimiter,nextKeyMarker,nextVersionIdMarker,maxResults3);

        List<String> expCommonPrefixes3 = new ArrayList<String>();
        expCommonPrefixes3.add("/bb/");
        expCommonPrefixes3.add("/cc/");
        if (!vsList3.isTruncated()) {
        	//TODO:2、这里返回为空记录的结果校验请优化代码，尽量简洁或者增加描述信息
            checkResult(vsList3,expCommonPrefixes3,new ArrayList<String>(),new String[]{});
         } else {
           Assert.fail("vsList3.isTruncated() must be false");
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
    //TODO:3、这段代码可以提取公共方法，或者针对该用例实现检测结果
    private void checkResult(VersionListing vsList, List<String> commonPrefixes, List<String> expKeys, String[] expVersions) throws Exception {
        Assert.assertEquals(vsList.getBucketName(), bucketName);
        List<String> actCommonPrefixes = vsList.getCommonPrefixes();
        System.out.println("actCommonPrefixes = " + actCommonPrefixes.toString());
        Assert.assertEquals(actCommonPrefixes.size(), commonPrefixes.size());
        Assert.assertEquals(actCommonPrefixes, commonPrefixes,
                "actCommonPrefixes = " + actCommonPrefixes.toString() + ",expCommonPrefixes=" + commonPrefixes.toString());
        List<S3VersionSummary> vsSummaryList = vsList.getVersionSummaries();
        Assert.assertEquals(vsSummaryList.size(), expVersions.length);
        String key = "";
        List<String> actKeys = new ArrayList<String>();
        for (int i = 0; i < vsSummaryList.size(); i++) {
            S3VersionSummary versionSummary = vsSummaryList.get(i);
            Assert.assertEquals(versionSummary.getBucketName(), bucketName);
            Assert.assertEquals(versionSummary.getVersionId(), expVersions[i], versionSummary.getKey());
            if(!key.equals(versionSummary.getKey())){
                actKeys.add(versionSummary.getKey());
            }
            key = versionSummary.getKey();
        }
        Assert.assertEquals(actKeys.toString(),expKeys.toString(),"actObjectNames = " + actKeys + ",keys = " + expKeys);
    }

    private VersionListing listVersions(String bucketName, String prefix, String delimiter, String keyMarker,String versionIdMarker,Integer maxResults) {
        ListVersionsRequest request = new ListVersionsRequest();
        request.setBucketName(bucketName);
        request.setPrefix(prefix);
        request.setDelimiter(delimiter);
        request.setKeyMarker(keyMarker);
        request.setVersionIdMarker(versionIdMarker);
        request.setMaxResults(maxResults);
        VersionListing list = s3Client.listVersions(request);
        return list;
    }
}
