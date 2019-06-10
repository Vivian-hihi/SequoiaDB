package com.sequoias3.object;


import com.amazonaws.services.s3.AmazonS3;
import com.amazonaws.services.s3.model.*;
import com.sequoiadb.task.OperateTask;
import com.sequoiadb.task.TaskMgr;
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
public class DeleteObjectWithSetTime16476 extends S3TestBase {
    private boolean runSuccess = false;
    private AmazonS3 s3Client = null;
    private int fileSize = 1024 * 200;
    private int objectNum = 100;
    private String filePath = null;
    private String objectNameBase = "object16474";
    private List<String> objectNames = new ArrayList<String>();
    private File localPath = null;

    @BeforeClass
    private void setUp() throws IOException {
        localPath = new File(S3TestBase.workDir + File.separator + TestTools.getClassName());
        TestTools.LocalFile.removeFile(localPath);
        TestTools.LocalFile.createDir(localPath.toString());
        filePath = localPath + File.separator + "localFile_" + fileSize + ".txt";
        TestTools.LocalFile.createFile(filePath,fileSize);
        s3Client = CommLibS3.buildS3Client();
        CommLibS3.clearBucket(s3Client, bucketName);
        s3Client.createBucket(bucketName);
        CommLibS3.setBucketVersioning(s3Client, bucketName, BucketVersioningConfiguration.ENABLED);
        for (int i = 0; i < objectNum; i++) {
            objectNames.add(objectNameBase + "_" + i + "_" + TestTools.getRandomString(1));
            s3Client.putObject(bucketName, objectNames.get(i), new File(filePath));
        }
    }

    @Test
    public void test() throws Exception {
        //in the past
        Calendar cal = Calendar.getInstance();
        //set time
        cal.set(Calendar.YEAR,cal.get(Calendar.YEAR)-1);
        TaskMgr mgr = new TaskMgr();
        for(int i = 0; i < objectNum; i++) {
            mgr.addTask(new DeleteObject(objectNames.get(i)));
        }
        mgr.addTask(new SetTime(S3TestBase.s3HostName,cal.getTime().getTime()));
        mgr.execute();
        Assert.assertTrue(mgr.isAllSuccess(), mgr.getErrorMsg());

        //check result
        for(int i = 0; i < objectNum;i++) {
            Assert.assertNull(s3Client.getObject(bucketName,objectNames.get(i)));
        }
        for(int i = 0; i < objectNum;i++) {
            S3Object obj = s3Client.getObject(new GetObjectRequest(bucketName,objectNames.get(i)).withVersionId("0"));
            chectGetResult(obj,objectNames.get(0),"0",filePath);
        }

        //put agian
        for (int i = 0; i < objectNum; i++) {
            objectNames.add(objectNameBase + "_" + i + "_" + TestTools.getRandomString(1));
            s3Client.putObject(bucketName, objectNames.get(i), new File(filePath));
        }

        //in the furtue
        cal.set(Calendar.YEAR,cal.get(Calendar.YEAR)+2);
        TaskMgr mgr1 = new TaskMgr();
        for(int i = 0; i < objectNum; i++) {
            mgr1.addTask(new DeleteObject(objectNames.get(i)));
        }
        mgr1.addTask(new SetTime(S3TestBase.s3HostName,cal.getTime().getTime()));
        mgr1.execute();
        Assert.assertTrue(mgr.isAllSuccess(), mgr1.getErrorMsg());
        //check result
        for(int i = 0; i < objectNum;i++) {
            Assert.assertNull(s3Client.getObject(bucketName,objectNames.get(i)));
        }
        for(int i = 0; i < objectNum;i++) {
            S3Object obj = s3Client.getObject(new GetObjectRequest(bucketName,objectNames.get(i)).withVersionId("0"));
            chectGetResult(obj,objectNames.get(0),"0",filePath);
        }
        Assert.assertEquals(s3Client.listObjectsV2(bucketName).getKeyCount(),0);
        runSuccess = true;
    }

    @AfterClass
    private void tearDown() {
        try {
            if (runSuccess) {
                CommLibS3.clearBucket(s3Client, bucketName);
            }
        } finally {
            s3Client.shutdown();
        }
    }

    private class SetTime extends OperateTask {
        private String host = null;
        private long date;

        public SetTime(String host, long date) {
            this.host = host;
            this.date = date;
        }

        @Override
        public void exec() throws Exception {
            TestTools.setSystemTime(host,date);
        }
    }

    public class DeleteObject  extends OperateTask {
        private String objectName = null;

        public DeleteObject(String objectName) {
            this.objectName = objectName;
        }

        @Override
        public void exec() throws Exception {
            s3Client.deleteObject(bucketName, objectName);
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
            String downloadPath = TestTools.LocalFile.initDownloadPath(localPath,TestTools.getMethodName(),
                    Thread.currentThread().getId());
            ObjectUtils.inputStream2File(s3InputStream,downloadPath);
            Assert.assertEquals(TestTools.getMD5(downloadPath),TestTools.getMD5(filePath));
        }finally {
            if(s3InputStream != null){
                s3InputStream.close();
            }
        }
    }
}
