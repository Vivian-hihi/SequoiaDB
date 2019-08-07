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
import com.amazonaws.services.s3.model.PartETag;
import com.amazonaws.services.s3.model.UploadPartRequest;
import com.amazonaws.services.s3.model.UploadPartResult;
import com.sequoias3.testcommon.CommLib;
import com.sequoias3.testcommon.S3TestBase;
import com.sequoias3.testcommon.TestTools;
import com.sequoias3.testcommon.s3utils.ObjectUtils;
import com.sequoias3.testcommon.s3utils.PartUploadUtils;

/**
 * @Description seqDB-18709:开启分段检测，不开启版本控制，相同key不同uploadId多次分段上传
 * @Author huangxiaoni
 * @Date 2019.07.26
 */

public class UploadPart18709 extends S3TestBase {
    private boolean runSuccess = false;
    private AmazonS3 s3Client;
    private File localPath;
    private String filePath1;
    private String filePath2;
    private File file1;
    private File file2;
    private int fileSize = 30 * 1024 * 1024;
    private int maxPartNumber = 5;
    private String key = "/aa/bb/obj18706";
    List<PartETag> partETags = new ArrayList<>();

    @BeforeClass
    private void setUp() throws IOException {
        this.initFile();
        s3Client = CommLib.buildS3Client();
    }

    @Test
    private void test() throws Exception {
        String uploadId = PartUploadUtils.initPartUpload(s3Client, S3TestBase.bucketName, key);
        this.partUpload(uploadId);
        PartUploadUtils.completeMultipartUpload(s3Client, bucketName, key, 
                uploadId, partETags);

        // check results
        String downfileMd5 = 
                ObjectUtils.getMd5OfObject(s3Client, localPath, bucketName, key);
        Assert.assertEquals(downfileMd5, TestTools.getMD5(filePath2), partETags.toString());
        
        runSuccess = true;
    }

    @AfterClass
    private void tearDown() {
        try {
            if (runSuccess) {
                s3Client.deleteObject(S3TestBase.bucketName, key);
                TestTools.LocalFile.removeFile(localPath);
            }
        } finally {
            s3Client.shutdown();
        }
    }

    private List<PartETag> partUpload(String uploadId){
        File file = file1;
        for (int i = 0; i < 2; i++) {
            if (i != 0) {
                file = file2;
            }
            // part upload
            long fileOffset = 0;
            long partSize = fileSize / maxPartNumber;
            for (int j = 0; j < maxPartNumber; j++) {
                UploadPartRequest partRequest = new UploadPartRequest().withFile(file)
                        .withFileOffset(fileOffset).withPartNumber(j + 1).withPartSize(partSize)
                        .withBucketName(bucketName).withKey(key).withUploadId(uploadId);
                UploadPartResult partResult = s3Client.uploadPart(partRequest);
                
                if (i != 0) {
                    partETags.add(partResult.getPartETag());
                }
                fileOffset = partSize * (j + 1);
            }
        }
        return partETags;
    }
    
    private void initFile() throws IOException {
        localPath = new File(S3TestBase.workDir + File.separator + TestTools.getClassName());
        TestTools.LocalFile.removeFile(localPath);
        TestTools.LocalFile.createDir(localPath.toString());

        String filePathBase = localPath + File.separator + "localFile_" + fileSize;
        filePath1 = filePathBase + "_1.txt";
        filePath2 = filePathBase + "_2.txt";
        TestTools.LocalFile.createFile(filePath1, fileSize);
        TestTools.LocalFile.createFile(filePath2, fileSize);
        file1 = new File(filePath1);
        file2 = new File(filePath2);
    }
}