package com.sequoias3.user;

import java.util.List;
import java.util.UUID;

import org.json.JSONObject;
import org.json.XML;
import org.springframework.http.HttpStatus;
import org.springframework.web.client.HttpClientErrorException;
import org.springframework.web.client.HttpServerErrorException;
import org.testng.Assert;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import com.amazonaws.services.s3.AmazonS3;
import com.amazonaws.services.s3.model.Bucket;
import com.sequoias3.testcommon.CommLib;
import com.sequoias3.testcommon.S3TestBase;

/**
 * @Description: seqDB-16261:管理员删除普通用户
 *  @author fanyu
 * @Date:2018年10月30日
 * @version:1.0
 */

public class DeleteNormalUser16261 extends S3TestBase {
    private String name1 = "DeleteNormalUser16261A";
    private String name2 = "DeleteNormalUser16261B";
    private int num = 10;
    private boolean runSuccess = false;

    @BeforeClass
    private void setUp() {
        try {
            UserUtils.deleteUser(name1, UserUtils.accessKeyId, true);
        } catch (HttpClientErrorException e) {
            if (e.getStatusCode() != (HttpStatus.NOT_FOUND)) {
                e.printStackTrace();
                Assert.fail(e.getResponseBodyAsString());
            }
        }

        try {
            UserUtils.deleteUser(name2, UserUtils.accessKeyId, true);
        } catch (HttpClientErrorException e) {
            if (e.getStatusCode() != (HttpStatus.NOT_FOUND)) {
                e.printStackTrace();
                Assert.fail(e.getMessage());
            }
        }
    }

    @Test
    private void test() {
        // create user
        JSONObject userJSON1 = UserUtils.createUser(name1, UserCommDefind.normal, UserUtils.accessKeyId);
        JSONObject userJSON2 = UserUtils.createUser(name2, UserCommDefind.admin, UserUtils.accessKeyId);

        // get the accessKeyID and secretAccessKey from userJSON
        JSONObject json1 = userJSON1.getJSONObject(UserCommDefind.accessKeys);
        String accessKeyID1 = json1.getString(UserCommDefind.accessKeyID);
        String secretAccessKey1 = json1.getString(UserCommDefind.secretAccessKey);

        JSONObject json2 = userJSON2.getJSONObject(UserCommDefind.accessKeys);
        String accessKeyID2 = json2.getString(UserCommDefind.accessKeyID);
        String secretAccessKey2 = json2.getString(UserCommDefind.secretAccessKey);

        // the user creates bucket
        for (int i = 0; i < num; i++) {
            craeteBucketAndObject(accessKeyID1, secretAccessKey1, name1 + i);
            craeteBucketAndObject(accessKeyID2, secretAccessKey2, name2 + i);
        }

        // delete user
        UserUtils.deleteUser(name1, UserUtils.accessKeyId, true);

        // check: user1 does not exist; user2's bucket exist
        getUser(name1);
        checkBucketResult(accessKeyID2, secretAccessKey2, name2, name2);
        runSuccess = true;
    }

    @AfterClass
    private void tearDown() {
        if (runSuccess) {
            UserUtils.deleteUser(name2, UserUtils.accessKeyId, true);
        }
    }

    private void craeteBucketAndObject(String accessKeyID, String secretAccessKey, String name) {
        AmazonS3 s3Client = null;
        try {
            s3Client = CommLib.buildS3Client(accessKeyID, secretAccessKey);
            // create bucket
            s3Client.createBucket(name.toLowerCase());
            // create object
           //s3Client.putObject(name, name + "_" + UUID.randomUUID(), UUID.randomUUID().toString());
        } finally {
            if (s3Client != null) {
                s3Client.shutdown();
            }
        }
    }

    private void getUser(String name) {
        try {
            UserUtils.getUser(name1, UserUtils.accessKeyId);
            Assert.fail("exp fail but act success");
        } catch (HttpClientErrorException e) {
            String errorMsg = e.getResponseBodyAsString();
            JSONObject json1 = XML.toJSONObject(errorMsg);
            if (!json1.getJSONObject(UserCommDefind.error).getString(UserCommDefind.errorCode).contains("NoSuchUser")) {
                e.printStackTrace();
                Assert.fail(e.getMessage());
            }
        }
    }

    private void checkBucketResult(String accessKeyId, String secretAccessKey, String baseBucketName, String userName) {
        AmazonS3 s3Client = null;
        try {
            s3Client = CommLib.buildS3Client(accessKeyId, secretAccessKey);
            // create one bucket,check the bucket name and owner name
            List<Bucket> buckets = s3Client.listBuckets();
            Assert.assertEquals(buckets.size(), num, "buckets = " + buckets.toString());
            for (int i = 0; i < num; i++) {
                String actOwner = buckets.get(i).getOwner().getDisplayName();
                String actBucketName = buckets.get(i).getName();
                Assert.assertEquals(actBucketName, baseBucketName.toLowerCase() + i);
                Assert.assertEquals(actOwner, userName.toLowerCase());
            }
        } finally {
            if (s3Client != null) {
                s3Client.shutdown();
            }
        }
    }
}
