package com.sequoias3.delimiter;

import com.amazonaws.AmazonServiceException;
import com.amazonaws.services.s3.AmazonS3;
import com.sequoiadb.commlib.GroupMgr;
import com.sequoiadb.commlib.SdbTestBase;
import com.sequoiadb.exception.ReliabilityException;
import com.sequoiadb.fault.KillNode;
import com.sequoiadb.task.FaultMakeTask;
import com.sequoiadb.task.OperateTask;
import com.sequoiadb.task.TaskMgr;
import com.sequoias3.commlibs3.CommLibS3;
import com.sequoias3.commlibs3.S3TestBase;
import com.sequoias3.commlibs3.s3utils.DelimiterUtils;
import com.sequoias3.commlibs3.s3utils.UserUtils;
import org.testng.Assert;
import org.testng.SkipException;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

/**
 * test content: 开启版本控制，删除对象过程中db端节点异常 testlink-case: seqDB-18200
 * 
 * @author wangkexin
 * @Date 2019.05.10
 * @version 1.00
 */
public class DeleteObjectWithKillCoord18200 extends S3TestBase {
	private GroupMgr groupMgr = null;
	private String userName = "user18200";
	private String bucketName = "bucket18200";
	private String keyName = "dir1/key18200?test.txt";
	private String delimiter = "?";
	private String roleName = "normal";
	private String context = "content18200";
	private String[] acessKeys = null;
	private AmazonS3 s3Client = null;
	private boolean runSuccess = false;

	@BeforeClass
	private void setUp() throws Exception {
		groupMgr = GroupMgr.getInstance();
		if (!groupMgr.checkBusiness()) {
			throw new SkipException("checkBusiness failed");
		}
		CommLibS3.clearUser(userName);
		acessKeys = UserUtils.createUser(userName, roleName);
		s3Client = CommLibS3.buildS3Client(acessKeys[0], acessKeys[1]);
		CommLibS3.clearBucket(s3Client, bucketName);
		s3Client.createBucket(bucketName);
		CommLibS3.setBucketVersioning(s3Client, bucketName, "Enabled");

		DelimiterUtils.putBucketDelimiter(bucketName, delimiter, acessKeys[0]);
		DelimiterUtils.checkCurrentDelimiteInfo(bucketName, delimiter, acessKeys[0]);

		s3Client.putObject(bucketName, keyName, context);
	}

	@Test
	public void testDeleteObject() throws Exception {
		try {
			// kill coord node
			FaultMakeTask faultTask = KillNode.getFaultMakeTask(SdbTestBase.hostName, SdbTestBase.serviceName, 2);
			TaskMgr mgr = new TaskMgr(faultTask);

			DeleteObjectTask dTask = new DeleteObjectTask();
			mgr.addTask(dTask);
			mgr.execute();
			Assert.assertEquals(mgr.isAllSuccess(), true, mgr.getErrorMsg());

			// check whether the cluster is normal and lsn consistency ,the
			// longest waiting time is 600S
			Assert.assertEquals(groupMgr.checkBusinessWithLSN(600), true, "checkBusinessWithLSN() occurs timeout");

			s3Client.deleteVersion(bucketName, keyName, "0");
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
				s3Client.deleteVersion(bucketName, keyName, "0");
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