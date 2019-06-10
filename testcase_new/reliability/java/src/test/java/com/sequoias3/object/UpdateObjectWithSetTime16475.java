package com.sequoias3.object;


import com.amazonaws.services.s3.AmazonS3;
import com.amazonaws.services.s3.model.BucketVersioningConfiguration;
import com.amazonaws.services.s3.model.ObjectMetadata;
import com.amazonaws.services.s3.model.S3Object;
import com.amazonaws.services.s3.model.S3ObjectInputStream;
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
import java.util.Calendar;

/**
 * @Description seqDB-16475 ::开启版本控制，更新对象过程中sdb节点所在主机时间跳变
 * @author fanyu
 * @Date 2019.01.17
 * @version 1.00
 */
public class UpdateObjectWithSetTime16475 extends S3TestBase {
    private boolean runSuccess = false;
    private AmazonS3 s3Client = null;
    private int fileSize = 1024 * 200;
    private String filePath = null;
    private String updatePath = null;
    private String updatePath1 = null;
    private String objectName = "object16475";
    private int versionNum = 10;
    private File localPath = null;

    @BeforeClass
    private void setUp() throws IOException {
        localPath = new File(S3TestBase.workDir + File.separator + TestTools.getClassName());
        TestTools.LocalFile.removeFile(localPath);
        TestTools.LocalFile.createDir(localPath.toString());
        filePath = localPath + File.separator + "localFile_" + fileSize + ".txt";
        updatePath = localPath + File.separator + "localFile_" + (fileSize + 1024 * 200) + ".txt";
        updatePath1 = localPath + File.separator + "localFile_" + (fileSize + 1024 * 400) + ".txt";
        TestTools.LocalFile.createFile(filePath, fileSize);
        TestTools.LocalFile.createFile(updatePath, fileSize + 1024 * 200);
        TestTools.LocalFile.createFile(updatePath1, fileSize + 1024 * 400);
        s3Client = CommLibS3.buildS3Client();
        CommLibS3.clearBucket(s3Client, bucketName);
        s3Client.createBucket(bucketName);
        CommLibS3.setBucketVersioning(s3Client, bucketName, BucketVersioningConfiguration.ENABLED);
        s3Client.putObject(bucketName,objectName, new File(filePath));
    }

    @Test
    public void test() throws Exception {
        //in the past
        Calendar cal = Calendar.getInstance();
        cal.set(Calendar.YEAR,cal.get(Calendar.YEAR)-1);
        //set time
        TaskMgr mgr = new TaskMgr();
        mgr.addTask(new PutObject());
        mgr.addTask(new SetTime(S3TestBase.s3HostName,cal.getTime().getTime()));
        mgr.execute();
        Assert.assertTrue(mgr.isAllSuccess(), mgr.getErrorMsg());
        S3Object currObj = s3Client.getObject(bucketName,objectName);
        chectGetResult(currObj,String.valueOf(versionNum-1),updatePath);
        //put again
        s3Client.putObject(bucketName, objectName, new File(updatePath1));
        S3Object currObj1 = s3Client.getObject(bucketName,objectName);
        chectGetResult(currObj1,String.valueOf(versionNum),updatePath1);

        //in the furtue
        cal.set(Calendar.YEAR,cal.get(Calendar.YEAR)+2);
        //set time
        TaskMgr mgr1 = new TaskMgr();
        mgr1.addTask(new PutObject());
        mgr1.addTask(new SetTime(S3TestBase.s3HostName,cal.getTime().getTime()));
        mgr1.execute();
        Assert.assertTrue(mgr1.isAllSuccess(), mgr.getErrorMsg());
        S3Object currObj2 = s3Client.getObject(bucketName,objectName);
        chectGetResult(currObj2,String.valueOf(2*versionNum-1),updatePath);
        //put again
        s3Client.putObject(bucketName, objectName, new File(updatePath1));
        S3Object currObj3 = s3Client.getObject(bucketName,objectName);
        chectGetResult(currObj3,String.valueOf(2*versionNum),updatePath1);
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
              TestTools.restoreSystemTime(S3TestBase.s3HostName);
        }
    }


    private class PutObject  extends OperateTask {
        @Override
        public void exec() throws Exception {
            for (int i = 1; i < versionNum; i++) {
                s3Client.putObject(bucketName, objectName, new File(updatePath));
            }
        }
    }

    private void chectGetResult(S3Object object,String versionId,String filePath)throws  Exception{
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
            Assert.assertEquals(TestTools.getMD5(downloadPath), TestTools.getMD5(filePath));
        }finally {
            if(s3InputStream != null){
                s3InputStream.close();
            }
        }
    }
}
