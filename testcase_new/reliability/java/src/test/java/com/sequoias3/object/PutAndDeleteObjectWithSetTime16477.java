package com.sequoias3.object;


import com.amazonaws.services.s3.AmazonS3;
import com.amazonaws.services.s3.model.*;
import com.sequoias3.commlibs3.CommLibS3;
import com.sequoias3.commlibs3.S3TestBase;
import com.sequoias3.commlibs3.TestTools;
import com.sequoias3.commlibs3.s3utils.ObjectUtils;
import org.testng.Assert;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import java.io.File;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Calendar;
import java.util.List;

/**
 * @Description seqDB-16472 ::开启版本控制，创建对象过程中SequoiaS3和sdb节点网络异常
 * @author fanyu
 * @Date 2019.01.17
 * @version 1.00
 */
public class PutAndDeleteObjectWithSetTime16477 extends S3TestBase {
    private boolean runSuccess = false;
    private AmazonS3 s3Client = null;
    private int fileSize = 1024 * 200;
    private List<String> filePathList = new ArrayList<String>();
    private String objectName = "object16477";
    private int  verisonNum = 4;
    private File localPath = null;

    @BeforeClass
    private void setUp() throws IOException {
        localPath = new File(S3TestBase.workDir + File.separator + TestTools.getClassName());
        TestTools.LocalFile.removeFile(localPath);
        TestTools.LocalFile.createDir(localPath.toString());
        for(int i = 0; i <verisonNum; i++ ) {
            String filePath = localPath + File.separator + "localFile_" + (fileSize+i*1024)+ ".txt";
            TestTools.LocalFile.createFile(filePath, fileSize);
            filePathList.add(filePath);
        }
        s3Client = CommLibS3.buildS3Client();
        CommLibS3.clearBucket(s3Client, bucketName);
        s3Client.createBucket(bucketName);
        CommLibS3.setBucketVersioning(s3Client, bucketName, BucketVersioningConfiguration.ENABLED);
    }

    @Test
    public void test() throws Exception {
        Calendar cal = Calendar.getInstance();
        s3Client.putObject(bucketName,objectName,new File(filePathList.get(0)));
        //set time: now-oneMonth
        cal.set(Calendar.MONTH,cal.get(cal.MONTH)-1);
        TestTools.setSystemTime(S3TestBase.s3HostName,cal.getTime().getTime());
        //put object
        s3Client.putObject(bucketName,objectName,new File(filePathList.get(1)));
        //restore time
        TestTools.restoreSystemTime(S3TestBase.s3HostName);
        //put object
        s3Client.putObject(bucketName,objectName,new File(filePathList.get(2)));
        s3Client.putObject(bucketName,objectName,new File(filePathList.get(3)));
        //delete version:3
        String versionId = "3";
        s3Client.deleteVersion(bucketName,objectName,versionId);
        //check result
        try {
            S3Object object = s3Client.getObject(new GetObjectRequest(bucketName, objectName).withVersionId(versionId));
        }catch(AmazonS3Exception e){
            if(e.getStatusCode() != 404){
                Assert.fail(e.getMessage());
            }
        }
        S3Object currObject = s3Client.getObject(bucketName,objectName);
        chectGetResult(currObject,objectName,"2",filePathList.get(2));
        runSuccess = true;
    }

    @AfterClass
    private void tearDown() throws Exception {
        try {
            if (runSuccess) {
                CommLibS3.clearBucket(s3Client, bucketName);
            }
        } finally {
            TestTools.restoreSystemTime(S3TestBase.s3HostName);
            s3Client.shutdown();
        }
    }

    private void chectGetResult(S3Object object,String objectName,String versionId,String filePath)throws  Exception{
        Assert.assertEquals(object.getKey(),objectName);
        Assert.assertEquals(object.getBucketName(),bucketName);
        ObjectMetadata objectMetadata = object.getObjectMetadata();
        Assert.assertEquals(objectMetadata.getETag(), TestTools.getMD5(filePath));
        Assert.assertEquals(objectMetadata.getVersionId(),versionId);
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
}
