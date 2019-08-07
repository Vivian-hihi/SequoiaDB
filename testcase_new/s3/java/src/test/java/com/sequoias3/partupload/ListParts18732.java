package com.sequoias3.partupload;

import java.io.File;
import java.io.IOException;
import java.util.List;

import org.testng.Assert;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import com.amazonaws.services.s3.AmazonS3;
import com.amazonaws.services.s3.model.PartETag;
import com.sequoias3.testcommon.CommLib;
import com.sequoias3.testcommon.S3TestBase;
import com.sequoias3.testcommon.TestTools;
import com.sequoias3.testcommon.s3utils.ObjectUtils;
import com.sequoias3.testcommon.s3utils.PartUploadUtils;

/**
 * @Description seqDB-18732:带maxparts查询分段列表
 * @Author huangxiaoni
 * @Date 2019.07.26
 */

public class ListParts18732 extends S3TestBase {
    private boolean runSuccess = false;
    private AmazonS3 s3Client;
    private File localPath;
    private String filePath;
    private File file;
    private long fileSize = 5 * 1024 * 1024;
    private int maxPartNumber = 5;
    private String key = "/aa/bb/obj18732";

    @BeforeClass
    private void setUp() throws IOException {
        this.initFile();
        s3Client = CommLib.buildS3Client();
    }

    @Test
    private void test() throws Exception {
        String uploadId = PartUploadUtils.initPartUpload(s3Client, S3TestBase.bucketName, key);
        List<PartETag> partETags = PartUploadUtils.partUpload(s3Client, S3TestBase.bucketName, key, uploadId, 
                file, fileSize / maxPartNumber);

        // check results
        PartUploadUtils.listPartsAndCheckPartNumbers(s3Client, bucketName, key, partETags, uploadId);
        
        PartUploadUtils.completeMultipartUpload(s3Client, bucketName, key, uploadId, partETags);
        File downloadPath = new File(localPath + File.separator + "downloadFile_" + fileSize);
        String downfileMd5 = ObjectUtils.getMd5OfObject(s3Client, downloadPath, bucketName, key);
        Assert.assertEquals(downfileMd5, TestTools.getMD5(filePath), partETags.toString());

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
    
    private void initFile() throws IOException {
        localPath = new File(S3TestBase.workDir + File.separator + TestTools.getClassName());
        filePath = localPath + File.separator + "localFile_" + fileSize + ".txt";
        TestTools.LocalFile.removeFile(localPath);
        TestTools.LocalFile.createDir(localPath.toString());
        TestTools.LocalFile.createFile(filePath, fileSize);
        file = new File(filePath);
    }
}