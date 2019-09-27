package com.sequoias3.privilege;

import java.io.IOException;

import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import com.amazonaws.services.s3.AmazonS3;
import com.amazonaws.services.s3.model.AccessControlList;
import com.amazonaws.services.s3.model.CanonicalGrantee;
import com.amazonaws.services.s3.model.CreateBucketRequest;
import com.amazonaws.services.s3.model.Grant;
import com.amazonaws.services.s3.model.Permission;
import com.sequoias3.testcommon.CommLib;
import com.sequoias3.testcommon.S3TestBase;
import com.sequoias3.testcommon.s3utils.PrivilegeUtils;
import com.sequoias3.testcommon.s3utils.UserUtils;
import com.sequoias3.user.UserCommDefind;

/**
 * @Description seqDB-19457:为相同用户多次赋予对象访问权限
 * @Author huangxiaoni
 * @Date 2019.09.24
 */

public class SetObjectAcl19457 extends S3TestBase {
    private boolean runSuccess = false;
    private String tcId = "19457";
    private AmazonS3 adminS3 = null;
    private AmazonS3 userS3 = null;
    private String userName = "user" + tcId;
    private String userType = UserCommDefind.normal;
    private String bucketName = "bucket" + tcId;
    private String keyName = "key" + tcId;
    private String fileContent = "test";

    @BeforeClass
    private void setUp() throws IOException {
        adminS3 = CommLib.buildS3Client();
        CommLib.clearBucket(adminS3, bucketName);
        CommLib.clearUser(userName);
        String[] acessKeys = UserUtils.createUser(userName, userType);
        userS3 = CommLib.buildS3Client(acessKeys[0], acessKeys[1]);
    }

    /*
     * adminS3 create bucket and put object, and set object acl, then authorezed
     * to userS3
     */
    @Test
    private void test() throws Exception {
        adminS3.createBucket(new CreateBucketRequest(bucketName));
        adminS3.putObject(bucketName, keyName, fileContent);

        // set object acl
        setObjectAclAndCheckResults(Permission.FullControl);
        // set object acl again, same Permission
        setObjectAclAndCheckResults(Permission.FullControl);
        // set object acl again, different Permission
        setObjectAclAndCheckResults(Permission.Read);

        runSuccess = true;
    }

    @AfterClass
    private void tearDown() {
        try {
            if (runSuccess) {
                CommLib.clearBucket(adminS3, bucketName);
                CommLib.clearUser(userName);
            }
        } finally {
            adminS3.shutdown();
            userS3.shutdown();
        }
    }

    private void setObjectAclAndCheckResults(Permission permission) {
        // adminS3 set object acl, authorezed to userS3
        AccessControlList acl = new AccessControlList();
        acl.setOwner(adminS3.getS3AccountOwner());
        Grant grant = new Grant(new CanonicalGrantee(userS3.getS3AccountOwner().getId()), permission);
        acl.grantAllPermissions(grant);
        adminS3.setObjectAcl(bucketName, keyName, acl);
        // userS3 get object acl and check results
        PrivilegeUtils.checkSetObjectAclResult(userS3, bucketName, keyName, grant);
    }
}