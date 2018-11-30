package com.sequoias3.object;

import com.amazonaws.services.s3.AmazonS3;
import com.amazonaws.services.s3.model.*;
import com.sequoias3.testcommon.CommLib;
import com.sequoias3.testcommon.S3TestBase;
import com.sequoias3.testcommon.TestTools;
import com.sequoias3.testcommon.s3utils.ObjectUtils;
import org.testng.Assert;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import java.io.File;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Date;
import java.util.List;
import java.util.Random;

/**
 * @Description: seqDB-16362 :: 带versionId获取对象，指定range范围
 * @author fanyu
 * @Date:2018年11月12日
 * @version:1.0
 */

public class GetObjectByRange16362 extends S3TestBase {
    private boolean runSuccess = false;
    private String bucketName = "bucket16362";
    private String key = "object16362";
    private AmazonS3 s3Client = null;
    private int fileSize = 204800;
    private File localPath = null;
    private List<String> filePathList = new ArrayList<String>();
    private List<PutObjectResult> objectVSList = new ArrayList<PutObjectResult>();
    private int fileNum = 9;

    @BeforeClass
    private void setUp() throws IOException {
        localPath = new File(S3TestBase.workDir + File.separator + TestTools.getClassName());
        TestTools.LocalFile.removeFile(localPath);
        TestTools.LocalFile.createDir(localPath.toString());
        String filePath = null;
        for (int i = 0; i < fileNum; i++) {
            filePath = localPath + File.separator + "localFile_" + (fileSize + i) + ".txt";
            TestTools.LocalFile.createFile(filePath, fileSize+i);
            filePathList.add(filePath);
        }
        s3Client = CommLib.buildS3Client();
        CommLib.clearBucket(s3Client,bucketName);
        s3Client.createBucket(bucketName);
        CommLib.setBucketVersioning(s3Client,bucketName, BucketVersioningConfiguration.ENABLED);
    }

    @Test
    private void test() throws Exception {
        for (int i = 0; i < fileNum; i++) {
            objectVSList.add(putObject(bucketName, key, filePathList.get(i)));
        }

        //random version
        Random random = new Random();
        int randomIndex = random.nextInt(fileNum);
        int currfileSize = fileSize+ randomIndex;
        String downloadPath = TestTools.LocalFile.initDownloadPath(localPath, TestTools.getMethodName(),
                Thread.currentThread().getId());
        String randomVersionId = objectVSList.get(randomIndex).getVersionId();

        int interval = 1024 * 100;
        int start = 0;
        int end = interval;
        for (int i = 0; i < currfileSize / interval; i++) {
            S3Object object = getObjectByVersion(bucketName, key, randomVersionId, start, end);
            S3ObjectInputStream s3InputStream = null;
            try {
                ObjectUtils.inputStream2File(object.getObjectContent(), downloadPath);
                if(i < currfileSize/interval-1) {
                    start = end + 1;
                    if (i == (currfileSize / interval - 2)) {
                        end = currfileSize;
                    } else {
                        end = end + interval;
                    }
                }
            } finally {
                if (s3InputStream != null) {
                    s3InputStream.close();
                }
            }
        }
        Assert.assertEquals(TestTools.getMD5(downloadPath), TestTools.getMD5(filePathList.get(randomIndex)));
        runSuccess = true;
    }

    @AfterClass
    private void tearDown() {
        try {
            if (runSuccess) {
                CommLib.clearBucket(s3Client,bucketName);
                TestTools.LocalFile.removeFile(localPath);
            }
        } finally {
            if (s3Client != null) {
                s3Client.shutdown();
            }
        }
    }

    private S3Object getObjectByVersion(String bucketName, String key, String versionId, long start, long end) {
        GetObjectRequest request = new GetObjectRequest(bucketName, key);
        ResponseHeaderOverrides overrideHeaders = new ResponseHeaderOverrides();
        overrideHeaders.setCacheControl("CacheControl");
        overrideHeaders.setContentDisposition("disposition");
        request.setRange(start, end);
        request.withResponseHeaders(overrideHeaders);
        request.withVersionId(versionId);
        return s3Client.getObject(request);
    }

    private PutObjectResult putObject(String bucketName, String key, String filePath) {
        PutObjectRequest request = new PutObjectRequest(bucketName, key, new File(filePath));
        ObjectMetadata metaData = new ObjectMetadata();
        metaData.setExpirationTime(new Date());
        metaData.addUserMetadata("meta-1", "12346788");
        request.withMetadata(metaData);
        return s3Client.putObject(request);
    }
}
