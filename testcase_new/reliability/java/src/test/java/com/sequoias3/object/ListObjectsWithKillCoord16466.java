package com.sequoias3.object;

import com.amazonaws.services.s3.AmazonS3;
import com.amazonaws.services.s3.model.AmazonS3Exception;
import com.amazonaws.services.s3.model.ListObjectsV2Request;
import com.amazonaws.services.s3.model.ListObjectsV2Result;
import com.amazonaws.services.s3.model.S3ObjectSummary;
import com.sequoiadb.exception.ReliabilityException;
import com.sequoiadb.fault.KillNode;
import com.sequoiadb.task.FaultMakeTask;
import com.sequoiadb.task.OperateTask;
import com.sequoiadb.task.TaskMgr;
import com.sequoias3.commlibs3.CommLibS3;
import com.sequoias3.commlibs3.S3TestBase;
import com.sequoias3.commlibs3.TestTools;
import org.testng.Assert;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import java.io.File;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;

/**
 * @Description seqDB-16466 :: 获取对象列表过程中db(coord)端节点异常
 * @author fanyu
 * @Date 2019.01.17
 * @version 1.00
 */
public class ListObjectsWithKillCoord16466 extends S3TestBase {
    private boolean runSuccess = false;
    private String bucketName = "bucket16466";
    private String objectNameBase = "/aa/bb/object16466";
    private List<String> objectNames = new ArrayList<String>();
    private AmazonS3 s3Client = null;
    private int fileSize = 1024 * 1;
    private int objectNums = 100;
    private File localPath = null;
    private String filePath = null;

    @BeforeClass
    private void setUp() throws IOException {
        localPath = new File(S3TestBase.workDir + File.separator + TestTools.getClassName());
        filePath = localPath + File.separator + "localFile_" + fileSize + ".txt";

        TestTools.LocalFile.removeFile(localPath);
        TestTools.LocalFile.createDir(localPath.toString());
        TestTools.LocalFile.createFile(filePath, fileSize);
        s3Client = CommLibS3.buildS3Client();
        CommLibS3.clearBucket(s3Client, bucketName);
        s3Client.createBucket(bucketName);
        for (int i = 0; i < objectNums; i++) {
            String objectName = objectNameBase + "_" + i + "_" + TestTools.getRandomString(1);
            objectNames.add(objectName);
            s3Client.putObject(bucketName, objectName, new File(filePath));
        }
    }

    @Test
    public void test() throws ReliabilityException, IOException {
        //kill coord when list objects
        FaultMakeTask faultTask = KillNode.getFaultMakeTask(S3TestBase.hostName, S3TestBase.serviceName, 2);
        TaskMgr mgr = new TaskMgr(faultTask);
        ListObject listTask = new ListObject();
        mgr.addTask(listTask);
        mgr.execute();
        Assert.assertTrue(mgr.isAllSuccess(), mgr.getErrorMsg());
        //list objects again
        listObjectsAndCheck();
        runSuccess = true;
    }

    @AfterClass
    private void tearDown() {
        try {
            if (runSuccess) {
                CommLibS3.clearBucket(s3Client, bucketName);
                TestTools.LocalFile.removeFile(localPath);
            }
        } finally {
            s3Client.shutdown();
        }
    }

    private class ListObject extends OperateTask {
        @Override
        public void exec() throws IOException {
            try {
                listObjectsAndCheck();
            } catch (AmazonS3Exception e) {
                if (e.getStatusCode() != 500) {
                    throw e;
                }
            }
        }
    }

    private void listObjectsAndCheck() throws IOException {
        ListObjectsV2Result objectsV2Result;
        ListObjectsV2Request req = new ListObjectsV2Request().withBucketName(bucketName);
        do {
            objectsV2Result = s3Client.listObjectsV2(req);
            req.setContinuationToken(objectsV2Result.getNextContinuationToken());
            List<S3ObjectSummary> objects = objectsV2Result.getObjectSummaries();
            //check
            checkResult(objects);
            System.out.println(objectsV2Result.isTruncated());
        } while (objectsV2Result.isTruncated());
    }

    private void checkResult(List<S3ObjectSummary> objects) throws IOException {
        for (S3ObjectSummary objectSummary : objects) {
            Assert.assertEquals(objectSummary.getBucketName(), bucketName);
            Assert.assertTrue(objectNames.contains(objectSummary.getKey()),
                    "objectNames=" + objectNames.toString() +
                            ",objectSummary.getKey() = " + objectSummary.getKey());
            Assert.assertEquals(objectSummary.getETag(), TestTools.getMD5(filePath));
            Assert.assertEquals(objectSummary.getSize(), fileSize);
        }
    }
}
