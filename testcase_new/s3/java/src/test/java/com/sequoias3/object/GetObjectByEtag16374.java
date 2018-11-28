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

import java.io.*;
import java.util.*;

/**
 * @Description: seqDB-16374 :: 指定ifMatch和ifNoneMatch条件获取对象，不匹配ifMatch
 * @author fanyu
 * @Date:2018年11月13日
 * @version:1.0
 */

public class GetObjectByEtag16374 extends S3TestBase {
    private boolean runSuccess = false;
    private String bucketName = "bucket16374";
    private String objectName = "object16374";
    private AmazonS3 s3Client = null;
    private int fileSize = 3;
    private File localPath = null;
    private List<String> filePathList = new ArrayList<String>();
    private List<PutObjectResult> objectVSList = new ArrayList<PutObjectResult>();
    private int fileNum = 10;

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
            objectVSList.add( putObject(bucketName, objectName, filePathList.get(i)));
        }

        // get history version eTag
        String histETag1 = objectVSList.get(fileNum - 3).getETag();
        String histETag2 = objectVSList.get(fileNum-2).getETag();

        //get history versionId
        String currVersionId = objectVSList.get(fileNum - 1).getVersionId();

        //get object by eTag
        S3Object object = getObjectByVersion(bucketName, objectName, currVersionId, histETag1, histETag2);
        Assert.assertNull(object);
        runSuccess = true;
    }

    @AfterClass
    private void tearDown() {
        try {
            if(runSuccess) {
                CommLib.clearBucket(s3Client,bucketName);
                TestTools.LocalFile.removeFile(localPath);
            }
        } finally {
            if (s3Client != null) {
                s3Client.shutdown();
            }
        }
    }

    private S3Object getObjectByVersion(String bucketName, String objectName, String versionId, String matchETag, String noMatchETag) {
        GetObjectRequest request = new GetObjectRequest(bucketName, objectName);
        ResponseHeaderOverrides overrideHeaders = new ResponseHeaderOverrides();
        overrideHeaders.setCacheControl("CacheControl");
        overrideHeaders.setContentDisposition("disposition");
        request.withResponseHeaders(overrideHeaders);
        request.withVersionId(versionId);
        request.withMatchingETagConstraint(matchETag);
        request.withNonmatchingETagConstraint(noMatchETag);
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
