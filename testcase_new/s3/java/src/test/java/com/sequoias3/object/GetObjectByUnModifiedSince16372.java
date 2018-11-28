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
 * @Description: seqDB-16371 :: 指定ifModifiedSince条件，获取对象
 * @author fanyu
 * @Date:2018年11月10日
 * @version:1.0
 */

public class GetObjectByUnModifiedSince16372 extends S3TestBase {
    private boolean runSuccess = false;
    private String bucketName = "bucket16372";
    private String objectName = "object16372";
    private AmazonS3 s3Client = null;
    private int fileSize = 1;
    private File localPath = null;
    private List<String> filePathList = new ArrayList<String>();
    private List<PutObjectResult> objectVSList = new ArrayList<PutObjectResult>();
    private int fileNum = 5;
    private Calendar cal = Calendar.getInstance();

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

        // random version
        Random random = new Random();
        int randomIndex = random.nextInt(fileNum-1);
        String randomVersionId = objectVSList.get(randomIndex).getVersionId();

        //the object has been modified since now-one_month
        cal.set(Calendar.MONTH,cal.get(Calendar.MONTH)-1);
        S3Object object = getObjectByVersion(bucketName, objectName,randomVersionId,cal.getTime());
        //AmazonS3  Java driver handles error,so it returns null
        Assert.assertNull(object);

        //the object has not been modified since now+one_year
        cal.set(Calendar.YEAR,cal.get(Calendar.YEAR)+1);
        S3Object object1 = getObjectByVersion(bucketName, objectName, randomVersionId,cal.getTime());
        //check the content
        String filePath = filePathList.get(randomIndex);
        checkResult(object1,filePath);
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

    private void checkResult(S3Object object, String filePath)throws  Exception{
        Assert.assertEquals(object.getObjectMetadata().getETag(),TestTools.getMD5(filePath));
        S3ObjectInputStream s3ObjectInputStream = null;
        try {
            s3ObjectInputStream = object.getObjectContent();
            String downloadPath = TestTools.LocalFile.initDownloadPath(localPath, TestTools.getMethodName(),
                    Thread.currentThread().getId());
            ObjectUtils.inputStream2File(s3ObjectInputStream, downloadPath);
            Assert.assertEquals(TestTools.getMD5(downloadPath), TestTools.getMD5(filePath));
        }finally{
            if(s3ObjectInputStream != null){
                s3ObjectInputStream.close();
            }
        }
    }

    private S3Object getObjectByVersion(String bucketName, String objectName, String versionId,Date unmodifiedSince) {
        GetObjectRequest request = new GetObjectRequest(bucketName, objectName);
        ResponseHeaderOverrides overrideHeaders = new ResponseHeaderOverrides();
        overrideHeaders.setCacheControl("CacheControl");
        overrideHeaders.setContentDisposition("disposition");
        request.withResponseHeaders(overrideHeaders);
        request.withVersionId(versionId);
        request.withUnmodifiedSinceConstraint(unmodifiedSince);
        return s3Client.getObject(request);
    }

    private PutObjectResult putObject(String bucketName, String key, String filePath) {
        PutObjectRequest request = new PutObjectRequest(bucketName, key, new File(filePath));
        ObjectMetadata metaData = new ObjectMetadata();
        metaData.setContentLanguage("English");
        metaData.setContentLength(10L);
        metaData.addUserMetadata("meta-1", "12346788");
        request.withMetadata(metaData);
        return s3Client.putObject(request);
    }
}
