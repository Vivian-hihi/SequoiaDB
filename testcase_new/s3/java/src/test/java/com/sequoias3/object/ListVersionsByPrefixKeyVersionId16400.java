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
import java.util.Collections;
import java.util.Date;
import java.util.List;

/**
 * @Description: seqDB-16400 :: 带prefix、keyMarker和versionIdMarker匹配查询对象版本列表
 * @author fanyu
 * @Date:2018年11月19日
 * @version:1.0
 */

public class ListVersionsByPrefixKeyVersionId16400 extends S3TestBase {
    private boolean runSuccess1 = false;
    private boolean runSuccess2 = false;
    private String bucketName = "bucket16400";
    private String[] objectNames = {"dir16400/subdir16400A","dir16400/dir16400A/dir16400AB","dirsub16400A","dirsub16400B"};
    private List<String> sortObjectNames = new ArrayList<String>();
    private AmazonS3 s3Client = null;
    private int fileSize = 3;
    private int versionNum = 400;
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
            sortObjectNames.add(objectName);
        }
        Collections.sort(sortObjectNames);
    }

    @Test
    private void testRecordsLt1K() throws Exception {
        String prefix = "dirsub";
        String keyMarker = "dirsub16400A";
        String versionIdMarker = "399";
        //list by prefix/keyMarker/versionIdMarker
        VersionListing vsList = listVersionsByPreKeyVersion(bucketName,prefix,keyMarker,versionIdMarker);
        //check
        checkResult(vsList,false,prefix,keyMarker,versionIdMarker,versionNum*2-1,sortObjectNames.subList(2,4));
        runSuccess1 = true;
    }

    @Test
    private void testRecordsGt1K() throws Exception {
        String prefix = "dir";
        //s3'versions is in an ascending order by key  same as sortObjectNames
        String keyMarker = sortObjectNames.get(0);
        String versionIdMarker = "400";
        //list by prefix/keyMarker/versionIdMarker
        VersionListing vsList = listVersionsByPreKeyVersion(bucketName,prefix,keyMarker,versionIdMarker);
        //check
        //TODO:3、文本用例中是分两种场景验证，一种时一次返回所有对象，还有一种时多次返回匹配对象，用例实现没有覆盖到
        checkResult(vsList,true,prefix,keyMarker,versionIdMarker,vsList.getMaxKeys(),sortObjectNames.subList(0,3));

        //list by prefix/keyMarker/versionIdMarker
        String keyMarker1 = vsList.getNextKeyMarker();
        String versionIdMarker1 = vsList.getNextVersionIdMarker();
        VersionListing vsList1 = listVersionsByPreKeyVersion(bucketName,prefix,keyMarker1,versionIdMarker1);
        //check
        checkResult(vsList1,false,prefix,keyMarker1,versionIdMarker1,versionNum*sortObjectNames.size()-vsList1.getMaxKeys()
                ,sortObjectNames.subList(2,4));
        runSuccess2 = true;
    }

    @AfterClass
    private void tearDown() {
        try {
            if (runSuccess1 && runSuccess2) {
                CommLib.clearBucket(s3Client,bucketName);
                TestTools.LocalFile.removeFile(localPath);
            }
        } finally {
            if (s3Client != null) {
                s3Client.shutdown();
            }
        }
    }

    private void checkResult(VersionListing vsList,boolean turncated,String prefix,String keyMarker,String versionIdMarker,int size,List<String> sortedObjectNames)throws  Exception {
        Assert.assertEquals(vsList.isTruncated(),turncated);
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
        Assert.assertEquals(actKeys,sortedObjectNames,"actObjectNames = " + actKeys + ",keys = " + sortedObjectNames);
    }

    //TODO:1、建议提取公共方法，不用每个用例都写相同代码
    private VersionListing listVersionsByPreKeyVersion(String bucketName,String prefix,String keyMarker,String versionIdMarker){
        ListVersionsRequest request = new ListVersionsRequest();
        request.setBucketName(bucketName);
        request.setPrefix(prefix);
        request.setKeyMarker(keyMarker);
        request.setVersionIdMarker(versionIdMarker);
        return s3Client.listVersions(request);
    }
   //TODO:2、建议提取公共方法，不用每个用例都写相同代码
    private PutObjectResult putObject(String bucketName, String key, String filePath) {
        PutObjectRequest request = new PutObjectRequest(bucketName, key, new File(filePath));
        ObjectMetadata metaData = new ObjectMetadata();
        metaData.setExpirationTime(new Date());
        metaData.addUserMetadata("meta-1", "16394");
        request.withMetadata(metaData);
        return s3Client.putObject(request);
    }
}
