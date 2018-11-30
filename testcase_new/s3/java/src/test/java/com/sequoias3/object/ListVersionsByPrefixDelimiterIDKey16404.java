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

/**
 * @Description:   seqDB-16404 :: 带prefix、keyMarker、versionIdMarker和delimiter查询对象版本列表，不匹配delimiter
 * @author fanyu
 * @Date:2018年11月20日
 * @version:1.0
 */

public class ListVersionsByPrefixDelimiterIDKey16404 extends S3TestBase {
    private boolean runSuccess = false;
    private String bucketName = "bucket16404";
    //please sort  in an ascending order by objectName
    private String[] objectNames = {"air116404", "dir2/16404A.png", "test16404.doc"};
    private AmazonS3 s3Client = null;
    private int fileSize = 3;
    private int versionNum = 5;
    private File localPath = null;
    private List<String> filePathList = new ArrayList<String>();

    @BeforeClass
    private void setUp() throws IOException {
        localPath = new File(S3TestBase.workDir + File.separator + TestTools.getClassName());
        TestTools.LocalFile.removeFile(localPath);
        TestTools.LocalFile.createDir(localPath.toString());
        for (int i = 0; i < versionNum; i++) {
            String filePath = localPath + File.separator + "localFile_" + (fileSize + i) + ".txt";
            TestTools.LocalFile.createFile(filePath, fileSize + i);
            filePathList.add(filePath);
        }
        s3Client = CommLib.buildS3Client();
        CommLib.clearBucket(s3Client, bucketName);
        s3Client.createBucket(bucketName);
        CommLib.setBucketVersioning(s3Client, bucketName, BucketVersioningConfiguration.ENABLED);
        for (String objectName : objectNames) {
            for (int i = 0; i < versionNum; i++) {
                putObject(bucketName, objectName, filePathList.get(i));
            }
        }
    }

    @Test(enabled = false)//SEQUOIADBMAINSTREAM-3974
    private void test() throws Exception {
        String prefix = "dir";
        String delimiter = "%";
        String keyMarker = objectNames[0];
        String versionIdMarker = "4";

        //list versions by prefix/delimiter/currentversionId/key
        VersionListing vsList = listVersionsByPreDelimiter(bucketName, prefix, delimiter,keyMarker,versionIdMarker);
        List<String> keys = new ArrayList<String>();
        keys.add( objectNames[1]);
        checkResult(vsList,prefix,delimiter,new ArrayList<String>(),keys);
        runSuccess = true;
    }

    @AfterClass
    private void tearDown() {
        try {
            if (runSuccess) {
                CommLib.clearBucket(s3Client, bucketName);
                TestTools.LocalFile.removeFile(localPath);
            }
        } finally {
            if (s3Client != null) {
                s3Client.shutdown();
            }
        }
    }

    private void checkResult(VersionListing vsList, String prefix, String delimiter, List<String> commonPrefixes, List<String> expKeys) throws Exception {
        Assert.assertEquals(vsList.getBucketName(), bucketName);
        Assert.assertEquals(vsList.getPrefix(), prefix);
        Assert.assertEquals(vsList.getDelimiter(), delimiter);
        List<String> actCommonPrefixes = vsList.getCommonPrefixes();
        Assert.assertEquals(actCommonPrefixes.size(), commonPrefixes.size());
        Assert.assertEquals(actCommonPrefixes, commonPrefixes,
                "actCommonPrefixes = " + actCommonPrefixes.toString() + ",expCommonPrefixes=" + commonPrefixes.toString());
        List<S3VersionSummary> vsSummaryList = vsList.getVersionSummaries();
        Assert.assertEquals(vsSummaryList.size(), expKeys.size() * versionNum);
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


    private VersionListing listVersionsByPreDelimiter(String bucketName, String prefix, String delimiter,String keyMarker,String versionIdMarker) {
        ListVersionsRequest request = new ListVersionsRequest();
        request.setBucketName(bucketName);
        request.setPrefix(prefix);
        request.setDelimiter(delimiter);
        request.setKeyMarker(keyMarker);
        request.setVersionIdMarker(versionIdMarker);
        return s3Client.listVersions(request);
    }

    private PutObjectResult putObject(String bucketName, String key, String filePath) {
        PutObjectRequest request = new PutObjectRequest(bucketName, key, new File(filePath));
        ObjectMetadata metaData = new ObjectMetadata();
        metaData.setExpirationTime(new Date());
        metaData.addUserMetadata("meta-1", "16401");
        request.withMetadata(metaData);
        return s3Client.putObject(request);
    }
}
