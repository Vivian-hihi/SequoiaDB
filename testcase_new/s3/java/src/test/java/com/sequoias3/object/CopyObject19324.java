package com.sequoias3.object;

import org.testng.Assert;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import com.amazonaws.services.s3.AmazonS3;
import com.amazonaws.services.s3.model.AmazonS3Exception;
import com.amazonaws.services.s3.model.CopyObjectRequest;
import com.sequoias3.testcommon.CommLib;
import com.sequoias3.testcommon.S3TestBase;
import com.sequoias3.testcommon.s3utils.UserUtils;

/**
 * @Description seqDB-19324:非桶管理用户复制对象
 * @author wuyan
 * @Date 2019.09.18
 * @version 1.00
 */
// TODO 1.跟文本用例测试点不符；2.文本用例未标记执行结果
public class CopyObject19324 extends S3TestBase {
    private boolean runSuccess = false;
    private String userNameA = "UserA19324";
    private String userNameB = "UserB19324";
    private String roleName = "normal";
    private String srcKeyName = "/src/object19324";
    private String destKeyName = "/dest/object19324";
    private String srcBucketName = "srcbucket19324";
    private String destBucketName = "destbucket19324";
    private AmazonS3 s3ClientA = null;
    private AmazonS3 s3ClientB = null;

    @BeforeClass
    private void setUp() {
        CommLib.clearUser(userNameA);
        CommLib.clearUser(userNameB);
        String[] acessKeys1 = UserUtils.createUser(userNameA, roleName);
        String[] acessKeys2 = UserUtils.createUser(userNameB, roleName);
        s3ClientA = CommLib.buildS3Client(acessKeys1[0], acessKeys1[1]);
        s3ClientB = CommLib.buildS3Client(acessKeys2[0], acessKeys2[1]);

        s3ClientA.createBucket(srcBucketName);
        s3ClientB.createBucket(destBucketName);
        s3ClientA.putObject(srcBucketName, srcKeyName, "test copy object.");
    }

    @Test
    public void testCopyObject() throws Exception {

        try {
            CopyObjectRequest request = new CopyObjectRequest(srcBucketName, srcKeyName, destBucketName, destKeyName);
            s3ClientB.copyObject(request);
            Assert.fail("copyObject must be fail !");
        } catch (AmazonS3Exception e) {
            Assert.assertEquals(e.getErrorCode(), "AccessDenied", e.getStatusCode() + e.getErrorMessage());
        }

        Assert.assertFalse(s3ClientB.doesObjectExist(destBucketName, destKeyName));
        runSuccess = true;
    }

    @AfterClass
    private void tearDown() {
        try {
            if (runSuccess) {
                UserUtils.deleteUser(userNameA);
                UserUtils.deleteUser(userNameB);
            }
        } finally {
            s3ClientA.shutdown();
            s3ClientB.shutdown();
        }
    }

}
