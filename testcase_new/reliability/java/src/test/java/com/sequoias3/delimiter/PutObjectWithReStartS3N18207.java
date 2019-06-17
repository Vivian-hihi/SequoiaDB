package com.sequoias3.delimiter;


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
import java.util.concurrent.CopyOnWriteArrayList;

/**
 * @Description seqDB-18207 :: 增加对象过程中s3节点异常
 * @author fanyu
 * @version 1.00
 * @Date 2019.01.17
 */
public class PutObjectWithReStartS3N18207 extends S3TestBase {
    private boolean runSuccess = false;
    private AmazonS3 s3Client = null;
    private int fileSize = 1024 * 200;
    private int objectNums = 100;
    private String filePath = null;
    private String objectNameBase = "PutObject18207";
    private List<String> objectNames = new ArrayList<String>();
    private List<String> objectNameList = new CopyOnWriteArrayList<String>();
    private String delimiter = "#";
    private File localPath = null;

    @BeforeClass
    private void setUp() throws IOException {
        localPath = new File(S3TestBase.workDir + File.separator + TestTools.getClassName());
        TestTools.LocalFile.removeFile(localPath);
        TestTools.LocalFile.createDir(localPath.toString());
        filePath = localPath + File.separator + "localFile_" + fileSize + ".txt";
        TestTools.LocalFile.createFile(filePath);
        s3Client = CommLibS3.buildS3Client();
        CommLibS3.clearBucket(s3Client, bucketName);
        s3Client.createBucket(bucketName);
        CommLibS3.setBucketVersioning(s3Client,bucketName, BucketVersioningConfiguration.ENABLED);
        for (int i = 0; i < objectNums; i++) {
            objectNames.add(objectNameBase + "_#" + i + "_#" + TestTools.getRandomString(1));
        }
    }

    @Test
    public void test() throws Exception {
        //restart s3
        FaultMakeTask faultMakeTask = S3NodeRestart.getFaultMakeTask(new S3NodeWrapper(), 1, 10);
        TaskMgr mgr = new TaskMgr(faultMakeTask);
        //TODO:1、用例中的更新符未设置，桶中还是使用默认更新符/
        for (int i = 0; i < objectNums; i++) {
            mgr.addTask(new PutObject(objectNames.get(i), filePath));
        }
        mgr.execute();
        mgr.isAllSuccess();
        List<Exception> eList = mgr.getExceptions();
        for (Exception e : eList) {
            if (!e.getMessage().contains("Unable to execute HTTP request")) {
                throw e;
            }
        }
        //put again
        System.out.println("objectNameList = " + objectNameList.size());
        objectNames.removeAll(objectNameList);
        s3Client = CommLibS3.buildS3Client();
        for (String objectName : objectNames) {
            s3Client.putObject(bucketName, objectName, new File(filePath));
        }
        for (String objectName : objectNameList) {
            checkResult(objectName);
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
            if (s3Client != null) {
                s3Client.shutdown();
            }
        }
    }

       private class PutObject extends OperateTask {
        private String objectName = null;
        private String filePath = null;

        public PutObject(String objectName, String filePath) {
            this.objectName = objectName;
            this.filePath = filePath;
        }

        @Override
        public void exec() throws Exception {
            s3Client.putObject(bucketName, this.objectName, new File(filePath));
            objectNameList.add(this.objectName);
        }
    }

    private void checkResult(String objectName) throws Exception {
        S3Object obj = s3Client.getObject(bucketName, objectName);
        S3ObjectInputStream s3is = obj.getObjectContent();
        String downloadPath = TestTools.LocalFile.initDownloadPath(localPath,TestTools.getMethodName(),
                Thread.currentThread().getId());
        ObjectUtils.inputStream2File(s3is, downloadPath);
        s3is.close();
        String actMd5 = TestTools.getMD5(downloadPath);
        String expMd5 = TestTools.getMD5(filePath);

        Assert.assertEquals(obj.getKey(), objectName);
        Assert.assertEquals(actMd5, expMd5);

        // 通过携带delimiter查询对象列表的对外映射场景检测目录表是否生成新目录，对象元数据表和目录表中数据通过连接db手工校验
        ListObjectsV2Request request = new ListObjectsV2Request().withBucketName(bucketName).withEncodingType("url");
        request.withDelimiter(delimiter);
        ListObjectsV2Result result = s3Client.listObjectsV2(request);
        List<String> commonPrefixes = result.getCommonPrefixes();
        Assert.assertEquals(commonPrefixes.size(), 1);
        Assert.assertEquals(result.getObjectSummaries().size(), 0);
    }
}
