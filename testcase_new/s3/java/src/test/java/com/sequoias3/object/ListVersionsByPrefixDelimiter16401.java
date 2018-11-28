package com.sequoias3.object;

import ch.qos.logback.core.net.SyslogOutputStream;
import com.amazonaws.services.s3.AmazonS3;
import com.amazonaws.services.s3.model.*;
import com.amazonaws.util.StringUtils;
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
import java.util.Collections;
import java.util.Date;
import java.util.List;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

/**
 * @Description: seqDB-16401 :: 带prefix和delimiter匹配查询对象版本列表
 * @author fanyu
 * @Date:2018年11月20日
 * @version:1.0
 */

public class ListVersionsByPrefixDelimiter16401 extends S3TestBase {
    private boolean runSuccess = false;
    private String bucketName = "bucket16401";
    private String[] objectNames = {"dir-16401A/16401A.png", "dir-16401B/dir16401B.png", "dir16401.xml", "16401A.txt", "dirsub16401B.doc"};
    private AmazonS3 s3Client = null;
    private int fileSize = 3;
    private int versionNum = 3;
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
        String delimiter = "/";
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
            if(!key.equals(versionSummary.getKey())){
                actKeys.add(versionSummary.getKey());
            }
            key = versionSummary.getKey();
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

    private PutObjectResult putObject(String bucketName, String key, String filePath) {
        PutObjectRequest request = new PutObjectRequest(bucketName, key, new File(filePath));
        ObjectMetadata metaData = new ObjectMetadata();
        metaData.setExpirationTime(new Date());
        metaData.addUserMetadata("meta-1", "16401");
        request.withMetadata(metaData);
        return s3Client.putObject(request);
    }
}
