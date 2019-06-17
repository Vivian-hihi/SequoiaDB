package com.sequoias3.object;


import com.amazonaws.services.s3.AmazonS3;
import com.amazonaws.services.s3.model.*;
import com.sequoiadb.task.FaultMakeTask;
import com.sequoiadb.task.OperateTask;
import com.sequoiadb.task.TaskMgr;
import com.sequoias3.commlibs3.CommLibS3;
import com.sequoias3.commlibs3.S3TestBase;
import com.sequoias3.commlibs3.TestTools;
import com.sequoias3.commlibs3.s3utils.ObjectUtils;
import com.sequoias3.commlibs3.s3utils.S3NodeRestart;
import com.sequoias3.commlibs3.s3utils.bean.S3NodeWrapper;
import org.testng.Assert;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import java.io.File;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;
import java.util.Random;
import java.util.concurrent.CopyOnWriteArrayList;

/**
 * @Description seqDB-16469:更新对象过程中SequiaS3Client端异常
 * @author fanyu
 * @version 1.00
 * @Date 2019.01.21
 */
public class UpdateObjectReStartS3N16469 extends S3TestBase {
    private boolean runSuccess = false;
    private AmazonS3 s3Client = null;
    private int fileSize = 1024 * 200;
    private int objectNums = 10;
    private int versionNums = 2;
    private String filePath = null;
    private String updatePath = null;
    private String objectNameBase = "PutObject16473";//TODO:1、对象名需要更新用例ID
    private List<String> objectNames = new ArrayList<String>();
    private List<String> objectNameList = new CopyOnWriteArrayList<String>();
    private File localPath = null;

    @BeforeClass
    private void setUp() throws IOException {
        localPath = new File(S3TestBase.workDir + File.separator + TestTools.getClassName());
        TestTools.LocalFile.removeFile(localPath);
        TestTools.LocalFile.createDir(localPath.toString());
        filePath = localPath + File.separator + "localFile_" + fileSize + ".txt";
        updatePath = localPath + File.separator + "localFile_" + (fileSize + 1024 * 200) + ".txt";
        TestTools.LocalFile.createFile(filePath, fileSize);
        TestTools.LocalFile.createFile(updatePath, fileSize + 1024 * 200);
        s3Client = CommLibS3.buildS3Client();
        CommLibS3.clearBucket(s3Client, bucketName);
        s3Client.createBucket(bucketName);
        CommLibS3.setBucketVersioning(s3Client, bucketName, BucketVersioningConfiguration.ENABLED);
        for (int i = 0; i < objectNums; i++) {
            objectNames.add(objectNameBase + "_" + i + "_" + TestTools.getRandomString(1));
            s3Client.putObject(bucketName, objectNames.get(i), new File(filePath));
        }
    }

    @Test
    public void test() throws Exception {
        FaultMakeTask faultMakeTask = S3NodeRestart.getFaultMakeTask(new S3NodeWrapper(), 1, 10);
        TaskMgr mgr = new TaskMgr(faultMakeTask);
        for (int i = 0; i < objectNums; i++) {
            mgr.addTask(new PutObject(objectNames.get(i)));
        }
        mgr.execute();
        mgr.isAllSuccess();
        List<Exception> eList = mgr.getExceptions();
        for (Exception e : eList) {
            if (!e.getMessage().contains("Unable to execute HTTP request")) {
                throw e;
            }
        }
        s3Client = CommLibS3.buildS3Client();
        //put again
        objectNames.removeAll(objectNameList);
        for (String objectName : objectNames) {
            for (int i = 0; i < versionNums; i++) {
                PutObjectResult obj = s3Client.putObject(bucketName, objectName, new File(updatePath));
                checkPutResult(obj);
            }
        }
        //TODO:2、建议补充检测故障前更新对象（可随机检测）
        if (!objectNames.isEmpty()) {
            int index = new Random().nextInt(objectNames.size());
            String versionId = "1";
            S3Object s3Object = s3Client.getObject(new GetObjectRequest(bucketName, objectNames.get(index)));
            chectGetResult(s3Object, objectNames.get(index), versionId, updatePath);

            String versionId1 = "0";
            S3Object s3Object1 = s3Client.getObject(new GetObjectRequest(bucketName, objectNames.get(index), versionId1));
            chectGetResult(s3Object1, objectNames.get(index), versionId1, filePath);
        }
        runSuccess = true;
    }

    @AfterClass
    private void tearDown() throws Exception {
        try {
            if (runSuccess) {
                CommLibS3.clearBucket(s3Client, bucketName);
            }
        } finally {
            s3Client.shutdown();
        }
    }

    public class PutObject extends OperateTask {
        private String objectName = null;

        public PutObject(String objectName) {
            this.objectName = objectName;
        }

        @Override
        public void exec() throws Exception {
            PutObjectResult obj = s3Client.putObject(bucketName, this.objectName, new File(updatePath));
            checkPutResult(obj);
            objectNameList.add(this.objectName);

        }
    }

    private void checkPutResult(PutObjectResult obj) throws IOException {
        Assert.assertEquals(obj.getETag(), TestTools.getMD5(updatePath));
        Assert.assertEquals(obj.getETag(), TestTools.getMD5(updatePath));
        ObjectMetadata metadata = obj.getMetadata();
        Assert.assertTrue(Integer.parseInt(metadata.getVersionId()) == 1, metadata.getVersionId());
    }

    private void chectGetResult(S3Object object, String objectName, String versionId, String filePath) throws Exception {
        Assert.assertEquals(object.getKey(), objectName);
        Assert.assertEquals(object.getBucketName(), bucketName);
        ObjectMetadata objectMetadata = object.getObjectMetadata();
        Assert.assertEquals(objectMetadata.getETag(), TestTools.getMD5(filePath));
        Assert.assertEquals(objectMetadata.getVersionId(), versionId);
        S3ObjectInputStream s3InputStream = null;
        try {
            s3InputStream = object.getObjectContent();
            String downloadPath = TestTools.LocalFile.initDownloadPath(localPath, TestTools.getMethodName(),
                    Thread.currentThread().getId());
            ObjectUtils.inputStream2File(s3InputStream, downloadPath);
            Assert.assertEquals(TestTools.getMD5(downloadPath), TestTools.getMD5(filePath));
        } finally {
            if (s3InputStream != null) {
                s3InputStream.close();
            }
        }
    }
}
