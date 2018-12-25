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
 * @Description: seqDB-16402 ::带prefix、delimiter查询对象版本列表，不匹配delimiter
 * @author fanyu
 * @Date:2018年11月20日
 * @version:1.0
 */

public class ListVersionsByPrefixDelimiter16402 extends S3TestBase {
    private boolean runSuccess = false;
    private String bucketName = "bucket16402";
    private String[] objectNames = {"dir-16402", "dir16401B.png/", "16401.xml", "16401A.txt", "dirsub/16401B.doc"};
    private AmazonS3 s3Client = null;
    private int fileSize = 3;
    private int versionNum = 2;
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

    @Test
    private void test() throws Exception {
        String prefix = "dir";
        String delimiter = "//";
        VersionListing vsList = listVersionsByPreDelimiter(bucketName, prefix, delimiter);
        List<String> commPrefixes = getCommPrefixes(objectNames, prefix, delimiter);
        List<String> versionKeys = getKeys(objectNames, prefix, delimiter);
        checkResult(vsList, prefix, delimiter, commPrefixes, versionKeys);
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
            if(!key.equals(versionSummary.getKey())){//TODO:1、这里没有必要用if判断，可以直接存放实际结果,相同key增加比较versionId
                actKeys.add(versionSummary.getKey());
            }
            key = versionSummary.getKey();//TODO:2、同上，这样写不简洁，也没有比较versionID            
        }
        Assert.assertEquals(actKeys.toString(),expKeys.toString(),"actObjectNames = " + actKeys + ",keys = " + expKeys);
    }

    private List<String> getCommPrefixes(String[] objectNames, String prefix, String delimiter) {
        List<String> commPrefixes = new ArrayList<String>();
        for (String objectName : objectNames) {
            if (objectName.startsWith(prefix)) {
                int end = objectName.indexOf(delimiter, prefix.length());
                if (end != -1) {
                    commPrefixes.add( objectName.substring(0, end + delimiter.length()));
                }
            }
        }
        return commPrefixes;
    }

    private List<String> getKeys(String[] objectNames, String prefix, String delimiter) {
        List<String> keys = new ArrayList<String>();
        for (String objectName : objectNames) {
            if (objectName.startsWith(prefix)) {
                int index = objectName.indexOf(delimiter, prefix.length());
                if (index == -1) {
                    keys.add(objectName);
                }
            }
        }
        return keys;
    }

    private VersionListing listVersionsByPreDelimiter(String bucketName, String prefix, String delimiter) {
        ListVersionsRequest request = new ListVersionsRequest();
        request.setBucketName(bucketName);
        request.setPrefix(prefix);
        request.setDelimiter(delimiter);
        return s3Client.listVersions(request);
    }

    //TODO:1、建议提取公共方法
    private PutObjectResult putObject(String bucketName, String key, String filePath) {
        PutObjectRequest request = new PutObjectRequest(bucketName, key, new File(filePath));
        ObjectMetadata metaData = new ObjectMetadata();
        metaData.setExpirationTime(new Date());
        metaData.addUserMetadata("meta-1", "16401");
        request.withMetadata(metaData);
        return s3Client.putObject(request);
    }
}
