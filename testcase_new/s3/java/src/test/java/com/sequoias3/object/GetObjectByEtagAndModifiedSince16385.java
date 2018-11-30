package com.sequoias3.object;

import com.amazonaws.services.s3.AmazonS3;
import com.amazonaws.services.s3.model.*;
import com.sequoias3.testcommon.CommLib;
import com.sequoias3.testcommon.S3TestBase;
import com.sequoias3.testcommon.TestTools;
import com.sequoias3.testcommon.s3utils.ObjectUtils;
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
 * @Description: seqDB-16385 :: 指定ifNoneMatch/ifMatch/ifModifiedSince/ifNoneModifiedSince条件获取对象
 * @author fanyu
 * @Date:2018年11月14日
 * @version:1.0
 */

public class GetObjectByEtagAndModifiedSince16385 extends S3TestBase {
    private boolean runSuccess = false;
    private String bucketName = "bucket16385";
    private String objectName = "object16385";
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
            objectVSList.add(putObject(bucketName, objectName, filePathList.get(i)));
        }

        //get history eTag
        String histETag1 = objectVSList.get(fileNum-3).getETag();
        String histEtag2 = objectVSList.get(fileNum-2).getETag();

        //get histroy version
        String  versionId1 = objectVSList.get(fileNum-3).getVersionId();
        String  versionId2 = objectVSList.get(fileNum-2).getVersionId();
        String  versionid3 = objectVSList.get(fileNum-1).getVersionId();

        //get the lastModified of the version
        Date modified = getLastModified(bucketName,objectName,versionId1);
        Date unModified  = getLastModified(bucketName,objectName,versionid3);

        //get object by matchingETag/nonMatchingETag/modifiedSince/unModifiedSince
        S3Object currObject = getObjectByEtagAndModify(bucketName, objectName,versionId2,histEtag2,histETag1,modified,unModified);

        //check the eTag and the content of object
        String currPath = filePathList.get(fileNum - 2);
        chectResult(currObject,currPath);
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

    private void chectResult(S3Object object,String filePath)throws  Exception{
        Assert.assertEquals(object.getObjectMetadata().getETag(), TestTools.getMD5(filePath));
        S3ObjectInputStream s3InputStream = null;
        try {
            s3InputStream = object.getObjectContent();
            String downloadPath = TestTools.LocalFile.initDownloadPath(localPath, TestTools.getMethodName(),
                    Thread.currentThread().getId());
            ObjectUtils.inputStream2File(s3InputStream,downloadPath);
            Assert.assertEquals(TestTools.getMD5(downloadPath), TestTools.getMD5(filePath));
        }finally {
            if(s3InputStream != null){
                s3InputStream.close();
            }
        }
    }

    private Date getLastModified(String bucketName,String objectName,String versionId){
        GetObjectRequest getObjectRequest = new GetObjectRequest(bucketName,objectName,versionId);
        S3Object object = s3Client.getObject(getObjectRequest);
        return  object.getObjectMetadata().getLastModified();
    }

    private S3Object getObjectByEtagAndModify(String bucketName, String objectName,String versionId, String matchETag, String noMatchETag,Date modifiedSince,Date unModifiedSince) {
        GetObjectRequest request = new GetObjectRequest(bucketName, objectName);
        ResponseHeaderOverrides overrideHeaders = new ResponseHeaderOverrides();
        overrideHeaders.setCacheControl("CacheControl");
        overrideHeaders.setContentDisposition("disposition");
        request.withResponseHeaders(overrideHeaders);
        request.withMatchingETagConstraint(matchETag);
        request.withNonmatchingETagConstraint(noMatchETag);
        request.withModifiedSinceConstraint(modifiedSince);
        request.withUnmodifiedSinceConstraint(unModifiedSince);
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
