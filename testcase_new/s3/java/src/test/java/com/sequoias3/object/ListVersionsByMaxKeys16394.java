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
 * @Description: seqDB-16396 ::带maxkeys查询对象版本列表
 * @author fanyu
 * @Date:2018年11月15日
 * @version:1.0
 */

public class ListVersionsByMaxKeys16394 extends S3TestBase {
    private boolean runSuccess1 = false;
    private boolean runSuccess2 = false;
    private boolean runSuccess3 = false;
    private boolean runSuccess4 = false;
    private String bucketName = "bucket16395";
    private String[] objectNames = {"dir16394/dir16394A/dir16394AB","dir16394/subdir16394A","dir16394A","dir16394B"};
    private AmazonS3 s3Client = null;
    private int fileSize = 3;
    private File localPath = null;
    private String filePath = null;

    @BeforeClass
    private void setUp() throws IOException {
        localPath = new File(S3TestBase.workDir + File.separator + TestTools.getClassName());
        TestTools.LocalFile.removeFile(localPath);
        TestTools.LocalFile.createDir(localPath.toString());
        filePath = localPath + File.separator + "localFile_" + fileSize + ".txt";
        TestTools.LocalFile.createFile(filePath, fileSize);
        s3Client = CommLib.buildS3Client();
        CommLib.clearBucket(s3Client,bucketName);
        s3Client.createBucket(bucketName);
        CommLib.setBucketVersioning(s3Client, bucketName, BucketVersioningConfiguration.ENABLED);
        for (String objectName : objectNames) {
            putObject(bucketName,objectName,filePath);
        }
    }

    @Test
    private void testMaxKeysLtRecords() throws Exception {
        int maxKeys = 1;
        VersionListing list= listVersionsByMaxKeys(bucketName,maxKeys);
        List<String> expKeys =  new ArrayList<String>();
        expKeys.add(objectNames[0]);
        checkResult(list,maxKeys,expKeys,true);
        runSuccess1 = true;
    }

    @Test
    private void testMaxKeysGtRecords() throws Exception {
        int maxKeys = 5;
        VersionListing list= listVersionsByMaxKeys(bucketName,maxKeys);
        List<String> expKeys =  new ArrayList<String>();
        for(int i = 0; i < objectNames.length; i++){
            expKeys.add(objectNames[i]) ;
        }
        checkResult(list,objectNames.length,expKeys,false);
        runSuccess2 = true;
    }

    @Test
    private void testMaxKeysEqRecords() throws Exception {
        int maxKeys = 4;
        VersionListing list= listVersionsByMaxKeys(bucketName,maxKeys);
        List<String> expKeys =  new ArrayList<String>();
        for(int i = 0; i < objectNames.length; i++){
            expKeys.add(objectNames[i]) ;
        }
        checkResult(list,objectNames.length,expKeys,false);
        runSuccess3 = true;
    }

    @Test
    private void testMaxKeysEqMiddle() throws Exception {
        int maxKeys = 3;
        VersionListing list= listVersionsByMaxKeys(bucketName,maxKeys);
        List<String> expKeys =  new ArrayList<String>();
        for(int i = 0; i < maxKeys; i++){
            expKeys.add(objectNames[i]) ;
        }
        checkResult(list,maxKeys,expKeys,true);
        runSuccess4 = true;
    }

    @AfterClass
    private void tearDown() {
        try {
            if (runSuccess1 && runSuccess2 && runSuccess3 && runSuccess4) {
                CommLib.clearBucket(s3Client,bucketName);
                TestTools.LocalFile.removeFile(localPath);
            }
        } finally {
            if (s3Client != null) {
                s3Client.shutdown();
            }
        }
    }

    private void checkResult(VersionListing list,int maxResult,List<String> expKeys,boolean isTruncated)throws  Exception {
        Assert.assertEquals(list.isTruncated(),isTruncated);
        if(maxResult >= objectNames.length) {
            Assert.assertNull(list.getNextKeyMarker());
            Assert.assertEquals(list.getNextVersionIdMarker(), "");
        }else{
            Assert.assertNotNull(list.getNextKeyMarker());
            Assert.assertNotNull( list.getNextVersionIdMarker());
        }
        List<S3VersionSummary> vsSummaryList = list.getVersionSummaries();
        Assert.assertEquals(vsSummaryList.size(), maxResult, vsSummaryList.toArray().toString());
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

    private VersionListing listVersionsByMaxKeys(String bucketName,int maxKeys){
        ListVersionsRequest request = new ListVersionsRequest();
        request.setBucketName(bucketName);
        request.setMaxResults(maxKeys);
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
