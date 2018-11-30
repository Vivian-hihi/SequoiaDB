package com.sequoias3.object;

import com.amazonaws.services.s3.AmazonS3;
import com.amazonaws.services.s3.model.AmazonS3Exception;
import com.sequoias3.testcommon.CommLib;
import com.sequoias3.testcommon.S3TestBase;
import org.testng.Assert;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import java.io.IOException;

/**
 * @author fanyu
 * @Description: seqDB-16368 :: 指定桶不存在
 * @Date:2018年11月12日
 * @version:1.0
 */

public class GetObjectByInexistenceBucket16368 extends S3TestBase {
    private boolean runSuccess = false;
    private String bucketName = "bucket16368";
    private String inexistbucketName = "inexistence16368";
    private String key = "a16368/b16368/b16368";
    private AmazonS3 s3Client = null;

    @BeforeClass
    private void setUp() throws IOException {
        s3Client = CommLib.buildS3Client();
        try {
            s3Client.deleteObject(bucketName, key);
        } catch (AmazonS3Exception e) {
            if (e.getStatusCode() != 404) {
                Assert.fail(e.getMessage());
            }
        }
        try {
            s3Client.deleteBucket(bucketName);
        } catch (AmazonS3Exception e) {
            if (e.getStatusCode() != 404) {
                Assert.fail(e.getMessage());
            }
        }
    }

    @Test
    private void test() throws Exception {
        s3Client.createBucket(bucketName);
        s3Client.putObject(bucketName, key, "1234");
        try {
            s3Client.getObject(inexistbucketName, key);
            Assert.fail("exp fail but act success");
        } catch (AmazonS3Exception e) {
            if (e.getStatusCode() != 404) {
                Assert.fail(e.getMessage());
            }
        }
        runSuccess = true;
    }

    @AfterClass
    private void tearDown() {
        try {
            if(runSuccess) {
                s3Client.deleteObject(bucketName, key);
                s3Client.deleteBucket(bucketName);
            }
        } finally {
            if (s3Client != null) {
                s3Client.shutdown();
            }
        }
    }
}
