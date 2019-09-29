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
import com.sequoiadb.commlib.GroupMgr;
import com.sequoiadb.commlib.GroupWrapper;
import com.sequoiadb.commlib.NodeWrapper;
import com.sequoiadb.fault.KillNode;
import com.sequoiadb.task.FaultMakeTask;
import com.sequoiadb.task.OperateTask;
import com.sequoiadb.task.TaskMgr;
import com.sequoias3.commlibs3.CommLibS3;
import com.sequoias3.commlibs3.S3TestBase;
import com.sequoias3.commlibs3.s3utils.PrivilegeUtils;

/**
 * @Description seqDB-19482:获取桶acl过程中db端节点异常
 * @Author huangxiaoni
 * @Date 2019.09.26
 */
public class GeBucketAclAndKillData19482 extends S3TestBase {
    private boolean runSuccess = false;
    private int threadNum = 20;
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
        // 如果配置的桶acl与桶默认acl相同，这里可以不配置，或者修改下配置，与默认桶acl配置不同
        grant = new Grant(new CanonicalGrantee(ownerId), Permission.FullControl);
        for (int i = 0; i < threadNum; i++) {
            String bucketName = bucketNameBase + i;
            CommLibS3.clearBucket(adminS3, bucketName);
            adminS3.createBucket(bucketName);
            bucketNames.add(bucketName);

            PrivilegeUtils.setBucketAclByBody(adminS3, bucketName, grant);
            setBucketAclFailList.add(bucketName);
        }
    }

    @Test
    public void test() throws Exception {
        TaskMgr mgr = new TaskMgr();
        // get bucket acl
        for (String bucketName : bucketNames) {
            mgr.addTask(new ThreadGetBucketAcl(bucketName));
        }

        // kill data node
        GroupMgr groupMgr = GroupMgr.getInstance();
        List<GroupWrapper> dataGroups = groupMgr.getAllDataGroup();
        for (int i = 0; i < dataGroups.size(); i++) {
            String groupName = dataGroups.get(i).getGroupName();
            GroupWrapper group = groupMgr.getGroupByName(groupName);
            NodeWrapper node = group.getMaster();
            FaultMakeTask faultTask = KillNode.getFaultMakeTask(node, 0);
            mgr.addTask(faultTask);
        }

        mgr.execute();
        Assert.assertTrue(mgr.isAllSuccess(), mgr.getErrorMsg());
        // TODO ：需要增加集群环境恢复检测的步骤
        // TODO: 如果再次获取配置桶信息是要获取所有配置过的桶acl，建议将集合setBucketAclFailList名称修改下
        // get bucket acl again, and check results
        for (String bucketName : setBucketAclFailList) {
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

    private class ThreadGetBucketAcl extends OperateTask {
        private String bucketName;

        private ThreadGetBucketAcl(String bucketName) {
            this.bucketName = bucketName;
        }

        @Override
        public void exec() throws Exception {
            AmazonS3 s3 = null;
            try {
                s3 = CommLibS3.buildS3Client();
                // TODO:
                // 这里不需要getBucketAcl这个步骤了，因为PrivilegeUtils.checkSetBucketAclResult中已经包含此步骤
                s3.getBucketAcl(bucketName);
                PrivilegeUtils.checkSetBucketAclResult(s3, bucketName, grant);
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
