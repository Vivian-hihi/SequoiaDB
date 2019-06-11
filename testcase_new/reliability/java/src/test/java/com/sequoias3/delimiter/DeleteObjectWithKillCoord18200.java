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
	//TODO：单词写错了
	private String[] acessKeys = null;
	private AmazonS3 s3Client = null;
	private boolean runSuccess = false;

	@BeforeClass
	private void setUp() throws Exception {
		groupMgr = GroupMgr.getInstance();
		//TODO: s3用例只关注s3功能，可以不用检测LSN
		if (!groupMgr.checkBusiness()) {
			throw new SkipException("checkBusiness failed");
		}
		CommLibS3.clearUser(userName);
		acessKeys = UserUtils.createUser(userName, roleName);
		s3Client = CommLibS3.buildS3Client(acessKeys[0], acessKeys[1]);
		CommLibS3.clearBucket(s3Client, bucketName);
		s3Client.createBucket(bucketName);
		//TODO:"Enabled"建议使用亚马逊定义的枚举值 BucketVersioningConfiguration.ENABLED
		CommLibS3.setBucketVersioning(s3Client, bucketName, "Enabled");
		DelimiterUtils.putBucketDelimiter(bucketName, delimiter, acessKeys[0]);
		DelimiterUtils.checkCurrentDelimiteInfo(bucketName, delimiter, acessKeys[0]);
		//TODO:建议创建多个版本对象，只创建一个版本可能测不到点，建议测试并发连续删除当前版本，
		// 主要测试历史版本元数据往当前元数据表中移动是否有问题
		s3Client.putObject(bucketName, keyName, context);
	}

	@Test
	public void testDeleteObject() throws Exception {
		try {
			// kill coord node
			//TODO:使用groupMgr获取所有coord节点，强杀所有的coord节点，因为客户端不知道服务端连接的是哪个coord节点
			FaultMakeTask faultTask = KillNode.getFaultMakeTask(SdbTestBase.hostName, SdbTestBase.serviceName, 2);
			TaskMgr mgr = new TaskMgr(faultTask);

			DeleteObjectTask dTask = new DeleteObjectTask();
			mgr.addTask(dTask);
			mgr.execute();
			Assert.assertEquals(mgr.isAllSuccess(), true, mgr.getErrorMsg());

			// check whether the cluster is normal and lsn consistency ,the
			// longest waiting time is 600S
			//TODO:s3这边可以不用检查LSN
			Assert.assertEquals(groupMgr.checkBusinessWithLSN(600), true, "checkBusinessWithLSN() occurs timeout");

			//TODO:失败后，建议先检查没有被删除的版本，再进行删除
			s3Client.deleteVersion(bucketName, keyName, "0");
			Assert.assertFalse(s3Client.doesObjectExist(bucketName, keyName));

			//TODO:异常直接抛出去，不用catch
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
				//TODO:不要在子线程里面进行Assert.assertEquals，因为当不是预期结果时，
				// 无法定位是哪一行报的错，建议不是预期结果时，将异常抛出去
				Assert.assertEquals(e.getErrorCode(), "GetDBConnectFail");
			} finally {
				if (s3Client != null) {
					s3Client.shutdown();
				}
			}
		}
	}
}