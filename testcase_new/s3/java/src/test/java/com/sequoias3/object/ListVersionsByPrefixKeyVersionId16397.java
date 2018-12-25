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
 * @Description:  seqDB-16397 ::带prefix、keyMarker和versionIdMarker查询对象版本列表，不匹配prefix
 * @author fanyu
 * @Date:2018年11月19日
 * @version:1.0
 */

public class ListVersionsByPrefixKeyVersionId16397 extends S3TestBase {
    private boolean runSuccess = false;
    private String bucketName = "bucket16397";
    private String[] objectNames = {"dir16397/subdir16397A","dir16397/dir16397A/dir16397AB","dir16397A","dir16397B"};
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
        for(int i = 0; i < versionNum; i++) {
            String filePath = localPath + File.separator + "localFile_" + (fileSize+i) + ".txt";
            TestTools.LocalFile.createFile(filePath, fileSize+i);
            filePathList.add(filePath);
        }
        s3Client = CommLib.buildS3Client();
        CommLib.clearBucket(s3Client,bucketName);
        s3Client.createBucket(bucketName);
        CommLib.setBucketVersioning(s3Client, bucketName, BucketVersioningConfiguration.ENABLED);
        for (String objectName : objectNames) {
            for(int i = 0; i < versionNum; i++) {
                putObject(bucketName, objectName, filePathList.get(i));
            }
        }
    }

    @Test
    private void test() throws Exception {
        //prefix is error
        String prefix = "subdir";
        String keyMarker = "dir16397A";
        String versionIdMarker = "0";
        //list by prefix/keyMarker/versionIdMarker
        VersionListing vsList = listVersionsByPreKeyVersion(bucketName,prefix,keyMarker,versionIdMarker);

       //check
        Assert.assertEquals(vsList.getPrefix(),prefix);
        Assert.assertEquals(vsList.getKeyMarker(),keyMarker);
        Assert.assertEquals(vsList.getVersionIdMarker(),versionIdMarker);
        Assert.assertEquals(vsList.getCommonPrefixes().size(),0);
        Assert.assertEquals(vsList.getVersionSummaries().size(),0);
        runSuccess = true;
    }

    @AfterClass
    private void tearDown() {
        try {
            if (runSuccess) {
                CommLib.clearBucket(s3Client,bucketName);
                TestTools.LocalFile.removeFile(localPath);
            }
        } finally {
            if (s3Client != null) {
                s3Client.shutdown();
            }
        }
    }
  //TODO:1、多个用例都用到这段代码，建议提取公共方法
    private VersionListing listVersionsByPreKeyVersion(String bucketName,String prefix,String keyMarker,String versionIdMarker){
        ListVersionsRequest request = new ListVersionsRequest();
        request.setBucketName(bucketName);
        request.setPrefix(prefix);
        request.setKeyMarker(keyMarker);
        request.setVersionIdMarker(versionIdMarker);
        return s3Client.listVersions(request);
    }
  //TODO:2、多个用例都用到这段代码，建议提取公共方法
    private PutObjectResult putObject(String bucketName, String key, String filePath) {
        PutObjectRequest request = new PutObjectRequest(bucketName, key, new File(filePath));
        ObjectMetadata metaData = new ObjectMetadata();
        metaData.setExpirationTime(new Date());
        metaData.addUserMetadata("meta-1", "16394");//TODO:3、这里的用例自定义元数据值时用例ID吗？建议和用例ID保持一致
        request.withMetadata(metaData);
        return s3Client.putObject(request);
    }
}
