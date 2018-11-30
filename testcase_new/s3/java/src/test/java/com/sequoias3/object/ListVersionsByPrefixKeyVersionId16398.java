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
 * @Description:   seqDB-16398 :: 带prefix、keyMarker和versionIdMarker查询对象版本列表，不匹配keyMarker
 * @author fanyu
 * @Date:2018年11月19日
 * @version:1.0
 */

public class ListVersionsByPrefixKeyVersionId16398 extends S3TestBase {
    private boolean runSuccess = false;
    private String bucketName = "bucket16398";
    private String[] objectNames = {"dir16398/dir16398A/dir16398AB","dir16398/subdir16398A","dir16398A","dir16398B"};
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
        String prefix = "dir";
        //keyMarker does not exist
        String keyMarker = "air16398C";
        String versionIdMarker = "2";
        //list by prefix/keyMarker/versionIdMarker
        VersionListing vsList = listVersionsByPreKeyVersion(bucketName,prefix,keyMarker,versionIdMarker);
        List<String> expKeys = new ArrayList<String>();
        for(String objectName: objectNames){
            expKeys.add(objectName);
        }
       //check
        checkResult(vsList,prefix,keyMarker,versionIdMarker,objectNames.length*versionNum,expKeys);
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

    private void checkResult(VersionListing vsList,String prefix,String keyMarker,String versionIdMarker,int size,List<String> expKeys)throws  Exception {
        Assert.assertFalse(vsList.isTruncated());
        Assert.assertEquals(vsList.getPrefix(),prefix);
        Assert.assertEquals(vsList.getKeyMarker(),keyMarker);
        Assert.assertEquals(vsList.getVersionIdMarker(),versionIdMarker);
        List<S3VersionSummary> vsSummaryList = vsList.getVersionSummaries();
        Assert.assertEquals(vsSummaryList.size(),size);
        String key = "";
        List<String> actKeys = new ArrayList<String>();
        for (int i = 0; i < vsSummaryList.size(); i++) {
            S3VersionSummary versionSummary = vsSummaryList.get(i);
            Assert.assertEquals(versionSummary.getBucketName(), bucketName);
            if(!key.equals(versionSummary.getKey())){
                actKeys.add(versionSummary.getKey());
            }
            key = versionSummary.getKey();
        }
        Assert.assertEquals(actKeys.toString(),expKeys.toString(),"actObjectNames = " + actKeys + ",keys = " + objectNames);
    }


    private VersionListing listVersionsByPreKeyVersion(String bucketName,String prefix,String keyMarker,String versionIdMarker){
        ListVersionsRequest request = new ListVersionsRequest();
        request.setBucketName(bucketName);
        request.setPrefix(prefix);
        request.setKeyMarker(keyMarker);
        request.setVersionIdMarker(versionIdMarker);
        return s3Client.listVersions(request);
    }

    private PutObjectResult putObject(String bucketName, String key, String filePath) {
        PutObjectRequest request = new PutObjectRequest(bucketName, key, new File(filePath));
        ObjectMetadata metaData = new ObjectMetadata();
        metaData.setExpirationTime(new Date());
        metaData.addUserMetadata("meta-1", "16394");
        request.withMetadata(metaData);
        return s3Client.putObject(request);
    }
}
