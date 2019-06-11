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
 * test content:  删除对象过程中db端节点异常 
 * testlink-case: seqDB-16465
 * @author wangkexin
 * @Date 2019.01.14
 * @version 1.00
 */
public class DeleteObjectKillDataNode16465 extends S3TestBase {
	private GroupMgr groupMgr = null;
	private String userName = "user16465";
	private String bucketName = "bucket16465";
	private String keyName = "key16465";
	private String roleName = "normal";
	private String context = "content16465";
	private String dataGroupName = null;
	//TODO:单词写错了
	private String[] acessKeys = null;
	private AmazonS3 s3Client = null;
	private boolean runSuccess = false;

	@BeforeClass
	private void setUp() throws Exception {
		groupMgr = GroupMgr.getInstance();
		//TODO:可以不用检查
		if (!groupMgr.checkBusiness()) {
			throw new SkipException("checkBusiness failed");
		}
		dataGroupName = groupMgr.getAllDataGroupName().get(0);
		CommLibS3.clearUser(userName);
		acessKeys = UserUtils.createUser(userName, roleName);
		s3Client = CommLibS3.buildS3Client(acessKeys[0], acessKeys[1]);
		s3Client.createBucket(bucketName);
		//TODO:建议创建多个对象进行删除，单个对象可以测不到测试点
		s3Client.putObject(bucketName, keyName, context);
	}

	//TODO:异常后的测试步骤与文本用例有点不符
	@Test
	public void testDeleteObject() throws Exception {
		try {
			GroupWrapper dataGroup = groupMgr.getGroupByName(dataGroupName);
			NodeWrapper priNode = dataGroup.getMaster();
            //TODO:需要强杀集群中所有的数据主节点
			FaultMakeTask faultTask = KillNode.getFaultMakeTask(priNode.hostName(), priNode.svcName(), 1);
			TaskMgr mgr = new TaskMgr(faultTask);

			//TODO:创建多个对象进行并发删除
			DeleteObjectTask dTask = new DeleteObjectTask();
			mgr.addTask(dTask);
			mgr.execute();
			Assert.assertEquals(mgr.isAllSuccess(), true, mgr.getErrorMsg());

			// check whether the cluster is normal and lsn consistency ,the
			// longest waiting time is 600S
			//TODO:可以不用检查
			Assert.assertEquals(groupMgr.checkBusinessWithLSN(600), true, "checkBusinessWithLSN() occurs timeout");
			Assert.assertFalse(s3Client.doesObjectExist(bucketName, bucketName));

		} catch (ReliabilityException e) {
			//TODO:非预期异常，抛出去，不要进行捕获
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
			//TODO:非预期异常抛出去，不要使用Assert.fail()
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
					s3Client.deleteObject(bucketName, keyName);
				}
			}catch(AmazonServiceException e){
				//TODO:线程内对异常进行处理，不要使用Assert.assertEquals，非预期异常建议抛出去
				Assert.assertEquals(e.getErrorCode(), "GetDBConnectFail");
			}finally {
				if (s3Client != null) {
					s3Client.shutdown();
				}
			}
		}
	}
}