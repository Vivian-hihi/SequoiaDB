package com.sequoias3.object;

import com.amazonaws.AmazonServiceException;
import com.amazonaws.services.s3.AmazonS3;
import com.sequoiadb.commlib.GroupMgr;
import com.sequoiadb.commlib.GroupWrapper;
import com.sequoiadb.commlib.NodeWrapper;
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.exception.ReliabilityException;
import com.sequoiadb.fault.KillNode;
import com.sequoiadb.task.FaultMakeTask;
import com.sequoiadb.task.OperateTask;
import com.sequoiadb.task.TaskMgr;
import com.sequoias3.commlibs3.CommLibS3;
import com.sequoias3.commlibs3.S3TestBase;
import com.sequoias3.commlibs3.s3utils.UserUtils;
import org.testng.Assert;
import org.testng.SkipException;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

/**
 * test content: 开启版本控制，删除对象过程中db端节点异常 testlink-case: seqDB-16464
 * 
 * @author wangkexin
 * @Date 2019.01.14
 * @version 1.00
 */
public class DeleteObjectKillDataNode16464 extends S3TestBase {
	private GroupMgr groupMgr = null;
	private String userName = "user16464";
	private String bucketName = "bucket16464";
	private String keyName = "key16464";
	private String roleName = "normal";
	private String context = "content16464";
	private String dataGroupName = null;
	private String[] acessKeys = null;
	private AmazonS3 s3Client = null;
	private boolean runSuccess = false;

	@BeforeClass
	private void setUp() throws Exception {
		groupMgr = GroupMgr.getInstance();
		if (!groupMgr.checkBusiness()) {
			throw new SkipException("checkBusiness failed");
		}
		dataGroupName = groupMgr.getAllDataGroupName().get(0);
		CommLibS3.clearUser(userName);
		acessKeys = UserUtils.createUser(userName, roleName);
		s3Client = CommLibS3.buildS3Client(acessKeys[0], acessKeys[1]);
		s3Client.createBucket(bucketName);
		CommLibS3.setBucketVersioning(s3Client, bucketName, "Enabled");
		s3Client.putObject(bucketName, keyName, context);
	}

	@Test
	public void testDeleteObject() throws Exception {
		try {
			GroupWrapper dataGroup = groupMgr.getGroupByName(dataGroupName);
			NodeWrapper priNode = dataGroup.getMaster();

			FaultMakeTask faultTask = KillNode.getFaultMakeTask(priNode.hostName(), priNode.svcName(), 1);
			TaskMgr mgr = new TaskMgr(faultTask);

			DeleteObjectTask dTask = new DeleteObjectTask();
			mgr.addTask(dTask);
			mgr.execute();
			Assert.assertEquals(mgr.isAllSuccess(), true, mgr.getErrorMsg());

			// check whether the cluster is normal and lsn consistency ,the
			// longest waiting time is 600S
			Assert.assertEquals(groupMgr.checkBusinessWithLSN(600), true, "checkBusinessWithLSN() occurs timeout");

			Assert.assertFalse(s3Client.doesObjectExist(bucketName, keyName));

		} catch (ReliabilityException e) {
			e.printStackTrace();
			Assert.fail(e.getMessage());
		}
		runSuccess = true;
	}

	@AfterClass
	private void tearDown() throws Exception {
		try {
			if (runSuccess) {
				UserUtils.deleteUser(userName);
			}
		} catch (BaseException e) {
			Assert.fail("clean up failed:" + e.getMessage());
		} finally {
			if (s3Client != null) {
				s3Client.shutdown();
			}
		}
	}

	private class DeleteObjectTask extends OperateTask {
		@Override
		public void exec() throws Exception {
			AmazonS3 s3Client = CommLibS3.buildS3Client(acessKeys[0], acessKeys[1]);
			try {
				for (int i = 0; i < 100; i++) {
					s3Client.deleteVersion(bucketName, keyName, "0");
				}
			} catch (AmazonServiceException e) {
				Assert.assertEquals(e.getErrorCode(), "GetDBConnectFail");
			} finally {
				if (s3Client != null) {
					s3Client.shutdown();
				}
			}
		}
	}
}