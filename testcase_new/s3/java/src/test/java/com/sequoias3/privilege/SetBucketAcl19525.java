package com.sequoias3.privilege;

import java.io.File;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;

import org.testng.Assert;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import com.amazonaws.services.s3.AmazonS3;
import com.amazonaws.services.s3.model.CannedAccessControlList;
import com.amazonaws.services.s3.model.CanonicalGrantee;
import com.amazonaws.services.s3.model.CreateBucketRequest;
import com.amazonaws.services.s3.model.Grant;
import com.amazonaws.services.s3.model.PartETag;
import com.amazonaws.services.s3.model.Permission;
import com.sequoias3.testcommon.CommLib;
import com.sequoias3.testcommon.S3TestBase;
import com.sequoias3.testcommon.TestTools;
import com.sequoias3.testcommon.s3utils.ObjectUtils;
import com.sequoias3.testcommon.s3utils.PartUploadUtils;
import com.sequoias3.testcommon.s3utils.PrivilegeUtils;
import com.sequoias3.testcommon.s3utils.UserUtils;

/**
 * @Description seqDB-19525:配置桶acl，向桶中分段上传对象
 * @Author wangkexin
 * @Date 2019.09.25
 */
public class SetBucketAcl19525 extends S3TestBase {
    private boolean runSuccess = false;
    private String ownerName = "owner19525";
    private String userName = "user19525";
    private String roleName = "normal";
    private String bucketName = "bucket19525";
    private String keyName = "key19525";
    private long fileSize = 10 * 1024 * 1024;
    private File localPath = null;
    private File file = null;
    private String filePath = null;
    private String[] acessKeys = null;
    private AmazonS3 userS3Client = null;
    private AmazonS3 ownerS3Client = null;
    private String ownerId;

    @BeforeClass
    private void setUp() throws IOException {
        localPath = new File(S3TestBase.workDir + File.separator + TestTools.getClassName());
        filePath = localPath + File.separator + "localFile_" + fileSize + ".txt";

        TestTools.LocalFile.removeFile(localPath);
        TestTools.LocalFile.createDir(localPath.toString());
        TestTools.LocalFile.createFile(filePath, fileSize);
        file = new File(filePath);

        // create a user
        CommLib.clearUser(ownerName);
        acessKeys = UserUtils.createUser(ownerName, roleName);
        ownerS3Client = CommLib.buildS3Client(acessKeys[0], acessKeys[1]);
        ownerId = ownerS3Client.getS3AccountOwner().getId();

        // create another uesr
        CommLib.clearUser(userName);
        acessKeys = UserUtils.createUser(userName, roleName);
        userS3Client = CommLib.buildS3Client(acessKeys[0], acessKeys[1]);

        CommLib.clearBucket(ownerS3Client, bucketName);
        ownerS3Client.createBucket(new CreateBucketRequest(bucketName));
    }

    @Test
    private void testSetBucketAcl() throws Exception {
        // put bucket acl
        ownerS3Client.setBucketAcl(bucketName, CannedAccessControlList.PublicReadWrite);

        // part upload object by other user
        List<PartETag> partEtags = new ArrayList<>();
        String uploadId = PartUploadUtils.initPartUpload(userS3Client, bucketName, keyName);
        partEtags = PartUploadUtils.partUpload(userS3Client, bucketName, keyName, uploadId, file);
        PartUploadUtils.completeMultipartUpload(userS3Client, bucketName, keyName, uploadId, partEtags);

        // check put object result
        String actMd5 = ObjectUtils.getMd5OfObject(userS3Client, localPath, bucketName, keyName);
        String expMd5 = TestTools.getMD5(filePath);
        Assert.assertEquals(actMd5, expMd5);

        // check object default acl by other user
        Grant expGrant = new Grant(new CanonicalGrantee(ownerId), Permission.FullControl);
        PrivilegeUtils.checkSetObjectAclResult(userS3Client, bucketName, keyName, expGrant);

        // force delete owner
        UserUtils.deleteUser(ownerName, S3TestBase.s3AccessKeyId, true);
        runSuccess = true;
    }

    @AfterClass
    private void tearDown() {
        try {
            if (runSuccess) {
                CommLib.clearUser(userName);
            }
        } finally {
            ownerS3Client.shutdown();
            userS3Client.shutdown();
        }
    }
}
