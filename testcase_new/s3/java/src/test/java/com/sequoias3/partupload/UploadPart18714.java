package com.sequoias3.partupload;

import java.io.File;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;

import org.testng.Assert;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import com.amazonaws.services.s3.AmazonS3;
import com.amazonaws.services.s3.model.AmazonS3Exception;
import com.amazonaws.services.s3.model.CreateBucketRequest;
import com.amazonaws.services.s3.model.PartETag;
import com.amazonaws.services.s3.model.UploadPartRequest;
import com.amazonaws.services.s3.model.UploadPartResult;
import com.sequoias3.testcommon.CommLib;
import com.sequoias3.testcommon.S3TestBase;
import com.sequoias3.testcommon.TestTools;
import com.sequoias3.testcommon.s3utils.PartUploadUtils;

/**
 * test content: 上传分段，其中指定key和uploadId不一致 testlink-case: seqDB-18714
 * 
 * @author wangkexin
 * @Date 2019.8.1
 * @version 1.00
 */
public class UploadPart18714 extends S3TestBase {
    private boolean runSuccess = false;
    private String bucketName = "bucket18714";
    private String keyNameA = "key18714a";
    private String keyNameB = "key18714b";
    private AmazonS3 s3Client = null;
    private long fileSize = 10 * 1024;
    private File localPath = null;
    private File file = null;
    private String filePath = null;

    @BeforeClass
    private void setUp() throws IOException {
        localPath = new File(S3TestBase.workDir + File.separator + TestTools.getClassName());
        filePath = localPath + File.separator + "localFile_" + fileSize + ".txt";

        TestTools.LocalFile.removeFile(localPath);
        TestTools.LocalFile.createDir(localPath.toString());
        TestTools.LocalFile.createFile(filePath, fileSize);
        file = new File(filePath);

        s3Client = CommLib.buildS3Client();
        CommLib.clearBucket(s3Client, bucketName);
        s3Client.createBucket(new CreateBucketRequest(bucketName));
    }

    @Test
    private void testUpload() throws Exception {
        // 对象A正在上传
        List<PartETag> partEtags = new ArrayList<>();
        String uploadIdA = uploadObjectA(partEtags);

        // 初始化对象B
        PartUploadUtils.initPartUpload(s3Client, bucketName, keyNameB);
        String wrongUploadId = "18714";
        // 上传分段指定uploadId不存在
        try {
            PartUploadUtils.partUpload(s3Client, bucketName, keyNameB, wrongUploadId, file);
            Assert.fail("upload part with non-existent uploadId should fail.");
        } catch (AmazonS3Exception e) {
            Assert.assertEquals(e.getErrorCode(), "NoSuchUpload");
        }

        // 上传分段指定uploadId为对象A的uploadId
        try {
            PartUploadUtils.partUpload(s3Client, bucketName, keyNameB, uploadIdA, file);
            Assert.fail("upload part with uploadId of other keys should fail.");
        } catch (AmazonS3Exception e) {
            Assert.assertEquals(e.getErrorCode(), "NoSuchUpload");
        }

        // 对象A完成分段上传
        PartUploadUtils.completeMultipartUpload(s3Client, bucketName, keyNameA, uploadIdA, partEtags);
        Assert.assertFalse(s3Client.doesObjectExist(bucketName, keyNameB));
        runSuccess = true;
    }

    @AfterClass
    private void tearDown() {
        try {
            if (runSuccess) {
                CommLib.clearBucket(s3Client, bucketName);
                TestTools.LocalFile.removeFile(localPath);
            }
        } finally {
            s3Client.shutdown();
        }
    }

    private String uploadObjectA(List<PartETag> partEtags) {
        String uploadId = PartUploadUtils.initPartUpload(s3Client, bucketName, keyNameA);
        long partSize = 5 * 1024;
        UploadPartRequest partRequest = new UploadPartRequest().withFile(file).withFileOffset(0).withPartNumber(1)
                .withPartSize(partSize).withBucketName(bucketName).withKey(keyNameA).withUploadId(uploadId);
        UploadPartResult uploadPartResult = s3Client.uploadPart(partRequest);
        partEtags.add(uploadPartResult.getPartETag());
        return uploadId;
    }
}
