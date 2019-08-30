package com.sequoias3.partupload.concurrent;

import java.io.File;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import com.amazonaws.services.s3.AmazonS3;
import com.amazonaws.services.s3.model.AbortMultipartUploadRequest;
import com.amazonaws.services.s3.model.AmazonS3Exception;
import com.amazonaws.services.s3.model.PartETag;
import com.amazonaws.services.s3.model.UploadPartRequest;
import com.amazonaws.services.s3.model.UploadPartResult;
import com.sequoiadb.threadexecutor.ThreadExecutor;
import com.sequoiadb.threadexecutor.annotation.ExecuteOrder;
import com.sequoias3.testcommon.CommLib;
import com.sequoias3.testcommon.S3TestBase;
import com.sequoias3.testcommon.TestTools;
import com.sequoias3.testcommon.s3utils.PartUploadUtils;

/**
 * @Description seqDB-18772: upload multiple parts concurrently,the length of
 *              the parts is the same and there is partNum of 1.
 * @author wuyan
 * @Date 2019.07.27
 * @version 1.00
 */
public class UploadPartAndCompleteMultipartUpload18772 extends S3TestBase {
    private boolean runSuccess = false;
    private String keyName = "/aa/object18772";
    private AmazonS3 s3Client = null;
    private File localPath = null;
    private String filePath = null;
    private int fileSize = 1024 * 1024 * 100;
    private int partSize = 1024 * 1024 * 20;
    private String uploadId = null;
    private List<PartETag> partEtags = Collections.synchronizedList(new ArrayList<PartETag>());

    @BeforeClass
    private void setUp() throws IOException {
        localPath = new File(S3TestBase.workDir + File.separator + TestTools.getClassName());
        filePath = localPath + File.separator + "localFile_" + fileSize + ".txt";
        TestTools.LocalFile.removeFile(localPath);
        TestTools.LocalFile.createDir(localPath.toString());
        TestTools.LocalFile.createFile(filePath, fileSize);
        s3Client = CommLib.buildS3Client();
    }

    @Test
    public void uploadParts() throws Exception {
        File file = new File(filePath);
        uploadId = PartUploadUtils.initPartUpload(s3Client, S3TestBase.bucketName, keyName);

        ThreadExecutor threadExec = new ThreadExecutor();
        int partNums = fileSize / partSize;
        for (int i = 0; i < partNums; i++) {
            int partNum = i + 1;
            threadExec.addWorker(new PartUpload(partNum, partSize, file, uploadId));
        }

        // TODO :需要补充完成分段上传成功的结果判断
        threadExec.addWorker(new CompletePartUpload(uploadId));
        threadExec.run();

        // check the upload part info
        PartUploadUtils.listPartsAndCheckPartNumbers(s3Client, S3TestBase.bucketName, keyName, partEtags, uploadId);
        runSuccess = true;
    }

    @AfterClass
    private void tearDown() {
        try {
            if (runSuccess) {
                AbortMultipartUploadRequest request = new AbortMultipartUploadRequest(S3TestBase.bucketName, keyName,
                        uploadId);
                s3Client.abortMultipartUpload(request);
                s3Client.deleteObject(S3TestBase.bucketName, keyName);
                TestTools.LocalFile.removeFile(localPath);
            }
        } finally {
            s3Client.shutdown();
        }
    }

    private class PartUpload {
        private int partNum;
        private int partSize;
        private File file;
        private String uploadId;
        private AmazonS3 s3Client1 = CommLib.buildS3Client();

        private PartUpload(int partNum, int partSize, File file, String uploadId) {
            this.partNum = partNum;
            this.partSize = partSize;
            this.file = file;
            this.uploadId = uploadId;
        }

        @ExecuteOrder(step = 1)
        private void partUpload() {
            try {
                int filePosition = (partNum - 1) * partSize;
                UploadPartRequest partRequest = new UploadPartRequest().withFile(file).withFileOffset(filePosition)
                        .withPartNumber(partNum).withPartSize(partSize).withBucketName(S3TestBase.bucketName)
                        .withKey(keyName).withUploadId(uploadId);
                UploadPartResult uploadPartResult = s3Client1.uploadPart(partRequest);
                partEtags.add(uploadPartResult.getPartETag());
            } finally {
                if (s3Client1 != null) {
                    s3Client1.shutdown();
                }
            }
        }
    }

    private class CompletePartUpload {
        private String uploadId;
        private AmazonS3 s3Client2 = CommLib.buildS3Client();

        private CompletePartUpload(String uploadId) {
            this.uploadId = uploadId;
        }

        @ExecuteOrder(step = 1)
        private void completeMultipartUpload() {
            try {
                PartUploadUtils.completeMultipartUpload(s3Client, S3TestBase.bucketName, keyName, uploadId, partEtags);
            } catch (AmazonS3Exception e) {
                int errCode = e.getStatusCode();
                // 400: InvalidPart
                if (errCode != 400) {
                    throw e;
                }
            } finally {
                if (s3Client2 != null) {
                    s3Client2.shutdown();
                }
            }
        }
    }
}
