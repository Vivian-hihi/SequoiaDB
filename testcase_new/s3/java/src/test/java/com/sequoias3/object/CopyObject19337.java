package com.sequoias3.object;

import java.util.Date;

import org.testng.Assert;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import com.amazonaws.services.s3.AmazonS3;
import com.amazonaws.services.s3.model.CopyObjectRequest;
import com.amazonaws.services.s3.model.CopyObjectResult;
import com.sequoias3.testcommon.CommLib;
import com.sequoias3.testcommon.S3TestBase;

/**
 * @Description seqDB-19337:指定ifUnModifiedSince和ifModifiedSince条件复制对象，
 *              源对象不匹配ifUnModifiedSince
 * @author wuyan
 * @Date 2019.09.19
 * @version 1.00
 */
public class CopyObject19337 extends S3TestBase {
    private boolean runSuccess = false;
    private String bucketName = "bucket19337";
    private String srcKeyName = "/src/bb%/object19337";
    private String destKeyName = "/dest/object19337";
    private AmazonS3 s3Client = null;

    @BeforeClass
    private void setUp() {

        s3Client = CommLib.buildS3Client();
        CommLib.clearBucket(s3Client, bucketName);
        s3Client.createBucket(bucketName);
        CommLib.setBucketVersioning(s3Client, bucketName, "Enabled");
        s3Client.putObject(bucketName, srcKeyName, "testcontent1");
        s3Client.putObject(bucketName, srcKeyName, "testcontent2");
    }

    @Test
    public void testCopyObject() throws Exception {
        // TODO 建议获取源对象的LastModified时间，而不是获取本地时间
        // set date 2 minutes early than the current time
        long currentTimestamp = new Date().getTime();
        long beforeTimestamp = currentTimestamp - 2 * 60 * 1000l;
        Date beforeDate = new Date(beforeTimestamp);

        // set date 3 minutes early than current time
        long timestamp = currentTimestamp - 3 * 60 * 1000l;
        Date noModifiedDate = new Date(timestamp);

        // copyObject
        CopyObjectRequest request = new CopyObjectRequest(bucketName, srcKeyName, bucketName, destKeyName);
        request.withModifiedSinceConstraint(beforeDate).withUnmodifiedSinceConstraint(noModifiedDate);
        CopyObjectResult result = s3Client.copyObject(request);
        Assert.assertNull(result, "does not match object!");
        Assert.assertFalse(s3Client.doesObjectExist(bucketName, destKeyName), "the destObject should be no exist!");

        runSuccess = true;
    }

    @AfterClass
    private void tearDown() {
        try {
            if (runSuccess) {
                CommLib.clearBucket(s3Client, bucketName);
            }
        } finally {
            s3Client.shutdown();
        }
    }
}
