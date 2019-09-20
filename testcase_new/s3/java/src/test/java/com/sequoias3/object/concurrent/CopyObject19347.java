package com.sequoias3.object.concurrent;

import java.io.File;
import java.io.IOException;

import org.testng.Assert;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.DataProvider;
import org.testng.annotations.Test;

import com.amazonaws.services.s3.AmazonS3;
import com.sequoias3.testcommon.CommLib;
import com.sequoias3.testcommon.S3TestBase;
import com.sequoias3.testcommon.TestTools;
import com.sequoias3.testcommon.s3utils.ObjectUtils;

/**
 * @Description seqDB-19347:并发指定相同源对象复制对象，目标对象不同
 * @author wuyan
 * @Date 2019.09.20
 * @version 1.00
 */
public class CopyObject19347 extends S3TestBase {
    @DataProvider(name = "copyObjectProvider", parallel = true)
    public Object[][] generateKeyName() {
        return new Object[][] {
                // the parameter is srcBucketName, destBucketName,destKeyName
                // test a: destBucketName is the same as source bucketName
                new Object[] { bucketNameA, bucketNameA, "/dest//aa%maa/bb%object19347" },
                new Object[] { bucketNameA, bucketNameA, "/dest/object19347" },
                new Object[] { bucketNameA, bucketNameA, "/dest/c/object19347" },
                // test b: destBucketName is different from source bucketName
                new Object[] { bucketNameA, bucketNameB, "/dest/aa/object19347" },
                new Object[] { bucketNameA, bucketNameB, "/dest/bb/object19347" },
                new Object[] { bucketNameA, bucketNameB, "/dest/cc/object19347" } };

    }

    private boolean runSuccess = false;
    private String srcKeyName = "src/bb%object19347";
    private String bucketNameA = "bucket19347a";
    private String bucketNameB = "bucket19347b";
    private AmazonS3 s3Client = null;
    private int fileSize = 1024 * 1024 * 10;
    private File localPath = null;
    private String filePath = null;
    private int successNum = 0;

    @BeforeClass
    private void setUp() throws IOException {
        localPath = new File(S3TestBase.workDir + File.separator + TestTools.getClassName());
        filePath = localPath + File.separator + "localFile_" + fileSize + ".txt";

        TestTools.LocalFile.removeFile(localPath);
        TestTools.LocalFile.createDir(localPath.toString());
        TestTools.LocalFile.createFile(filePath, fileSize);
        s3Client = CommLib.buildS3Client();
        CommLib.clearBucket(s3Client, bucketNameA);
        CommLib.clearBucket(s3Client, bucketNameB);

        s3Client.createBucket(bucketNameA);
        s3Client.createBucket(bucketNameB);
        s3Client.putObject(bucketNameA, srcKeyName, new File(filePath));
    }

    @Test(dataProvider = "copyObjectProvider")
    public void testCopyObject(String srcBucketName, String destBucketName, String destKeyName) throws Exception {
        s3Client.copyObject(srcBucketName, srcKeyName, destBucketName, destKeyName);
        checkObjectContent(destBucketName, destKeyName);

        successNum++;
        // all threads(6) execute successfully, then set runsuccess=true
        if (successNum == 6) {
            runSuccess = true;
        }
    }

    @AfterClass
    private void tearDown() {
        try {
            if (runSuccess) {
                CommLib.clearBucket(s3Client, bucketNameA);
                CommLib.clearBucket(s3Client, bucketNameB);
                TestTools.LocalFile.removeFile(localPath);
            }
        } finally {
            s3Client.shutdown();
        }
    }

    private void checkObjectContent(String bucketName, String keyName) throws Exception {
        String downfileMd5 = ObjectUtils.getMd5OfObject(s3Client, localPath, bucketName, keyName);
        Assert.assertEquals(downfileMd5, TestTools.getMD5(filePath));
    }
}
