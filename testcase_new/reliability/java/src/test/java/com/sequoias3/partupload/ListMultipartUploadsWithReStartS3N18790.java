package com.sequoias3.partupload;

import java.io.File;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;

import org.springframework.util.LinkedMultiValueMap;
import org.springframework.util.MultiValueMap;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import com.amazonaws.services.s3.AmazonS3;
import com.amazonaws.services.s3.model.CreateBucketRequest;
import com.amazonaws.services.s3.model.ListMultipartUploadsRequest;
import com.amazonaws.services.s3.model.MultipartUploadListing;
import com.sequoiadb.task.FaultMakeTask;
import com.sequoiadb.task.OperateTask;
import com.sequoiadb.task.TaskMgr;
import com.sequoiadb.threadexecutor.ThreadExecutor;
import com.sequoiadb.threadexecutor.annotation.ExecuteOrder;
import com.sequoias3.commlibs3.CommLibS3;
import com.sequoias3.commlibs3.S3TestBase;
import com.sequoias3.commlibs3.TestTools;
import com.sequoias3.commlibs3.s3utils.PartUploadUtils;
import com.sequoias3.commlibs3.s3utils.S3NodeRestart;
import com.sequoias3.commlibs3.s3utils.bean.S3NodeWrapper;

//TODO:1、用例注释描述不正确
/**
 * test content: 创建对象过程中s3节点异常 testlink-case: seqDB-18790
 * 
 * @author wangkexin
 * @Date 2019.05.23
 * @version 1.00
 */
public class ListMultipartUploadsWithReStartS3N18790 extends S3TestBase {
    private String bucketName = "bucket18790";
    private String keyName = "key18790";
    private AmazonS3 s3Client = null;
    private long fileSize = 100 * 1024 * 1024;
    MultiValueMap<String, String> expUploads = new LinkedMultiValueMap<String, String>();
    private File localPath = null;
    private String filePath = null;
    private boolean runSuccess = false;

    @BeforeClass
    private void setUp() throws IOException {
        localPath = new File(S3TestBase.workDir + File.separator + TestTools.getClassName());
        filePath = localPath + File.separator + "localFile_" + fileSize + ".txt";

        TestTools.LocalFile.removeFile(localPath);
        TestTools.LocalFile.createDir(localPath.toString());
        TestTools.LocalFile.createFile(filePath, fileSize);

        s3Client = CommLibS3.buildS3Client();
        CommLibS3.clearBucket(s3Client, bucketName);
        s3Client.createBucket(new CreateBucketRequest(bucketName));
    }

    @Test
    public void ListMultipartUploads() throws Exception {
        ThreadExecutor threadExec = new ThreadExecutor();
        for (int i = 0; i < 20; i++) {
            threadExec.addWorker(new InitPartUpload(keyName + "_" + i));
        }
        threadExec.run();

        FaultMakeTask faultMakeTask = S3NodeRestart.getFaultMakeTask(new S3NodeWrapper(), 0, 30);
        TaskMgr mgr = new TaskMgr(faultMakeTask);
        mgr.addTask(new ListMultipartUploads());
        mgr.execute();
        // TODO:2、这里没有校验线程执行结果
        mgr.isAllSuccess();
        List<Exception> eList = mgr.getExceptions();
        for (Exception e : eList) {
            if (!e.getMessage().contains("Unable to execute HTTP request")) {
                throw e;
            }
        }

        s3Client = CommLibS3.buildS3Client();
        listMultipartUploadAgain();
        runSuccess = true;
    }

    @AfterClass
    private void tearDown() throws Exception {
        try {
            if (runSuccess) {
                CommLibS3.clearBucket(s3Client, bucketName);
                // TODO:2、需要补充清理临时文件
            }
        } finally {
            if (s3Client != null) {
                s3Client.shutdown();
            }
        }
    }

    private class ListMultipartUploads extends OperateTask {
        @Override
        public void exec() throws Exception {
            AmazonS3 s3Client = CommLibS3.buildS3Client();
            try {
                Thread.sleep(1000);
                ListMultipartUploadsRequest request = new ListMultipartUploadsRequest(bucketName);
                MultipartUploadListing partUploadList = s3Client.listMultipartUploads(request);
                List<String> expCommonPrefixes = new ArrayList<>();
                PartUploadUtils.checkListMultipartUploadsResults(partUploadList, expCommonPrefixes, expUploads);
            } finally {
                if (s3Client != null) {
                    s3Client.shutdown();
                }
            }
        }
    }

    private void listMultipartUploadAgain() {
        ListMultipartUploadsRequest request = new ListMultipartUploadsRequest(bucketName);
        MultipartUploadListing partUploadList = s3Client.listMultipartUploads(request);
        List<String> expCommonPrefixes = new ArrayList<>();
        PartUploadUtils.checkListMultipartUploadsResults(partUploadList, expCommonPrefixes, expUploads);
    }

    private class InitPartUpload {
        private AmazonS3 s3Client = CommLibS3.buildS3Client();
        private String keyName;

        public InitPartUpload(String keyName) {
            this.keyName = keyName;
        }

        @ExecuteOrder(step = 1)
        private void partUpload() {
            try {
                String uploadId = PartUploadUtils.initPartUpload(s3Client, bucketName, keyName);
                expUploads.add(keyName, uploadId);
            } finally {
                if (s3Client != null) {
                    s3Client.shutdown();
                }
            }
        }
    }
}