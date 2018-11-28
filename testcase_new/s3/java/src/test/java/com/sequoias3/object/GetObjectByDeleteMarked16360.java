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
import java.util.*;

/**
 * @Description: seqDB-16360 ::带versionId获取删除标记的对象
 * @author fanyu
 * @Date:2018年11月12日
 * @version:1.0
 */

public class GetObjectByDeleteMarked16360 extends S3TestBase {
    private boolean runSuccess = false;
    private String bucketName = "bucket16360";
    private String key = "object16360";
    private AmazonS3 s3Client = null;
    private int fileSize = 3;
    private File localPath = null;
    private List<String> filePathList = new ArrayList<String>();
    private List<PutObjectResult> objectVSList = new ArrayList<PutObjectResult>();
    private int fileNum = 3;

    @BeforeClass
    private void setUp() throws IOException {
        localPath = new File(S3TestBase.workDir + File.separator + TestTools.getClassName());
        TestTools.LocalFile.removeFile(localPath);
        TestTools.LocalFile.createDir(localPath.toString());
        String filePath = null;
        for (int i = 0; i < fileNum; i++) {
            filePath = localPath + File.separator + "localFile_" + (fileSize + i) + ".txt";
            TestTools.LocalFile.createFile(filePath, fileSize + i);
            filePathList.add(filePath);
        }
        s3Client = CommLib.buildS3Client();
        CommLib.clearBucket(s3Client,bucketName);
        s3Client.createBucket(bucketName);
        CommLib.setBucketVersioning(s3Client,bucketName, BucketVersioningConfiguration.ENABLED);
    }

    @Test
    private void test() throws Exception {
        // create multiple versions object in the bucket
        for (int i = 0; i < fileNum; i++) {
            objectVSList.add(putObject(bucketName, key, filePathList.get(i)));
        }
        //get the id of history version
        String histVersionId = objectVSList.get(0).getVersionId();
        //delete version by versionId
        s3Client.deleteVersion(bucketName, key, histVersionId);
        //get the deleted version
        try {
            getObjectByVersion(bucketName, key, histVersionId);
            Assert.fail("exp failed but act success");
        } catch (AmazonS3Exception e) {
            if (e.getStatusCode() != 404) {
                Assert.fail(e.getMessage());
            }
        }

        // get the id of current version
        String currVersionId = objectVSList.get(fileNum - 1).getVersionId();
        //delete version by versionId
        s3Client.deleteVersion(bucketName, key, currVersionId);
        //get the deleted version
        try {
            getObjectByVersion(bucketName, key, currVersionId);
            Assert.fail("exp failed but act success");
        } catch (AmazonS3Exception e) {
            if (e.getStatusCode() != 404) {
                Assert.fail(e.getMessage());
            }
        }
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

    private S3Object getObjectByVersion(String bucketName, String objectName, String versionId) {
        GetObjectRequest request = new GetObjectRequest(bucketName, objectName);
        ResponseHeaderOverrides overrideHeaders = new ResponseHeaderOverrides();
        overrideHeaders.setCacheControl("CacheControl");
        overrideHeaders.setContentDisposition("disposition");
        request.withResponseHeaders(overrideHeaders);
        request.withVersionId(versionId);
        return s3Client.getObject(request);
    }

    private PutObjectResult putObject(String bucketName, String key, String filePath) {
        PutObjectRequest request = new PutObjectRequest(bucketName, key, new File(filePath));
        ObjectMetadata metaData = new ObjectMetadata();
        metaData.setExpirationTime(new Date());
        metaData.addUserMetadata("meta-1", "12346788");
        request.withMetadata(metaData);
        return s3Client.putObject(request);
    }
}
