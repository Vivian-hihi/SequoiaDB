package com.sequoias3.privilege;

import java.io.IOException;
import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.CopyOnWriteArrayList;

import org.testng.Assert;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import com.amazonaws.AmazonServiceException;
import com.amazonaws.services.s3.AmazonS3;
import com.amazonaws.services.s3.model.CanonicalGrantee;
import com.amazonaws.services.s3.model.Grant;
import com.amazonaws.services.s3.model.Permission;
import com.sequoiadb.task.FaultMakeTask;
import com.sequoiadb.task.OperateTask;
import com.sequoiadb.task.TaskMgr;
import com.sequoias3.commlibs3.CommLibS3;
import com.sequoias3.commlibs3.S3TestBase;
import com.sequoias3.commlibs3.s3utils.PrivilegeUtils;
import com.sequoias3.commlibs3.s3utils.S3NodeRestart;
import com.sequoias3.commlibs3.s3utils.bean.S3NodeWrapper;

/**
 * @Description seqDB-19481:更新桶acl过程中s3节点异常
 * @Author huangxiaoni
 * @Date 2019.09.26
 */
public class SeBucketAclAndKillS319481 extends S3TestBase {
    private boolean runSuccess = false;
    private int threadNum = 50;
    // TODO ： 下方用例编号有误
    private String tcId = "19469";
    private AmazonS3 adminS3 = null;
    private String ownerId;
    private String bucketNameBase = "bucket" + tcId + "a";
    private List<String> bucketNames = new ArrayList<>();
    private Grant grant;
    private List<String> setBucketAclFailList = new CopyOnWriteArrayList<String>();

    @BeforeClass
    private void setUp() throws IOException {
        adminS3 = CommLibS3.buildS3Client();
        ownerId = adminS3.getS3AccountOwner().getId();
        for (int i = 0; i < threadNum; i++) {
            String bucketName = bucketNameBase + i;
            CommLibS3.clearBucket(adminS3, bucketName);
            adminS3.createBucket(bucketName);
            bucketNames.add(bucketName);

            Grant grantFirst = new Grant(new CanonicalGrantee(ownerId), Permission.FullControl);
            PrivilegeUtils.setBucketAclByBody(adminS3, bucketName, grantFirst);
        }

        grant = new Grant(new CanonicalGrantee(ownerId), Permission.Read);
    }

    @Test
    public void test() throws Exception {
        TaskMgr mgr = new TaskMgr();
        // set bucket acl
        for (String bucketName : bucketNames) {
            mgr.addTask(new ThreadSetBucketAcl(bucketName));
        }

        // kill data node
        FaultMakeTask faultMakeTask = S3NodeRestart.getFaultMakeTask(new S3NodeWrapper(), 1, 10);
        mgr.addTask(faultMakeTask);

        mgr.execute();
        Assert.assertTrue(mgr.isAllSuccess(), mgr.getErrorMsg());

        // TODO ：这里应该再次配置在故障时配置失败的桶acl
        // set failed bucket acl again
        for (String bucketName : setBucketAclFailList) {
            PrivilegeUtils.setBucketAclByBody(adminS3, bucketName, grant);
        }

        // check all bucket restults
        for (String bucketName : bucketNames) {
            // TODO
            // :这里检查可以成功的话说明setBucketAclFailList中包含了所有的桶，请确认现有线程数是否可以撞到测试点
            PrivilegeUtils.checkSetBucketAclResult(adminS3, bucketName, grant);
        }

        runSuccess = true;
    }

    @AfterClass
    private void tearDown() {
        try {
            if (runSuccess) {
                for (String bucketName : bucketNames) {
                    CommLibS3.clearBucket(adminS3, bucketName);
                }
            }
        } finally {
            if (adminS3 != null)
                adminS3.shutdown();

        }
    }

    private class ThreadSetBucketAcl extends OperateTask {
        private String bucketName;

        private ThreadSetBucketAcl(String bucketName) {
            this.bucketName = bucketName;
        }

        @Override
        public void exec() throws Exception {
            AmazonS3 s3 = null;
            try {
                s3 = CommLibS3.buildS3Client();
                PrivilegeUtils.setBucketAclByBody(s3, bucketName, grant);
                // TODO ：下面这个集合应该存储的是配置桶acl成功的桶名集合
                setBucketAclFailList.add(bucketName);
            } catch (AmazonServiceException e) {
                if (e.getStatusCode() != 500) {
                    throw e;
                }
            } finally {
                if (s3 != null) {
                    s3.shutdown();
                }
            }
        }
    }
}
