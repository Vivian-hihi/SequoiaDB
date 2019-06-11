package com.sequoias3.object;

import com.amazonaws.AmazonServiceException;
import com.amazonaws.services.s3.AmazonS3;
import com.amazonaws.services.s3.model.GetObjectRequest;
import com.amazonaws.services.s3.model.S3Object;
import com.amazonaws.services.s3.model.S3ObjectInputStream;
import com.sequoiadb.commlib.GroupMgr;
import com.sequoiadb.commlib.GroupWrapper;
import com.sequoiadb.commlib.NodeWrapper;
import com.sequoiadb.commlib.SdbTestBase;
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.exception.ReliabilityException;
import com.sequoiadb.fault.KillNode;
import com.sequoiadb.task.FaultMakeTask;
import com.sequoiadb.task.OperateTask;
import com.sequoiadb.task.TaskMgr;
import com.sequoias3.commlibs3.CommLibS3;
import com.sequoias3.commlibs3.S3TestBase;
import com.sequoias3.commlibs3.TestTools;
import com.sequoias3.commlibs3.s3utils.ObjectUtils;
import com.sequoias3.commlibs3.s3utils.UserUtils;
import org.testng.Assert;
import org.testng.SkipException;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import java.io.File;

/**
 * test content: 更新对象过程中db端节点异常 
 * testlink-case: seqDB-16461
 * @author wangkexin
 * @Date 2019.01.09
 * @version 1.00
 */
public class UpdateObjectWithKillDataNode16462 extends S3TestBase {
	private GroupMgr groupMgr = null;
	private String userName = "user16461";
	private String bucketName = "bucket16461";
	private String keyName = "key16461";
	private String roleName = "normal";
	private String oldContext = "oldcontent16461";
	private String newContext = "newcontent16461";
	private String dataGroupName = null;
	private File localPath = null;
	//TODO:单词写错了
	private String[] acessKeys = null;
	private AmazonS3 s3Client = null;
	private boolean runSuccess = false;

	@BeforeClass
	private void setUp() throws Exception {
		localPath = new File(SdbTestBase.workDir + File.separator + TestTools.getClassName());
		TestTools.LocalFile.removeFile(localPath);
		TestTools.LocalFile.createDir(localPath.toString());
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
		//TODO：建议创建多个对象，并发进行更新，一个对象在一个线程内循环进行更新，检查对象内容时不好检查
		s3Client.putObject(bucketName, keyName, oldContext);
	}

	//TODO:测试步骤与文本用例有点不符，异常恢复后需要检查更新失败的对象、需要重新更新失败的对象和检查结果
	@Test
	public void testUpdateObject() throws Exception {
		try {
			GroupWrapper dataGroup = groupMgr.getGroupByName(dataGroupName);
			NodeWrapper priNode = dataGroup.getMaster();
			//TODO:需要强杀集群中所有的主节点
			FaultMakeTask faultTask = KillNode.getFaultMakeTask(priNode.hostName(), priNode.svcName(), 1);
			TaskMgr mgr = new TaskMgr(faultTask);
			
			UpdateObjectTask uTask = new UpdateObjectTask();
			mgr.addTask(uTask);
			mgr.execute();
			Assert.assertEquals(mgr.isAllSuccess(), true, mgr.getErrorMsg());

			// check whether the cluster is normal and lsn consistency ,the
			// longest waiting time is 600S
			//TODO:可以不用检查
			Assert.assertEquals(groupMgr.checkBusinessWithLSN(600), true, "checkBusinessWithLSN() occurs timeout");

			checkCurrentObjectResult();
		} catch (ReliabilityException e) {
			//TODO:非预期异常，不要进行捕获，抛出去
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
				TestTools.LocalFile.removeFile(localPath);
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

	private class UpdateObjectTask extends OperateTask {
		@Override
		public void exec() throws Exception {
			AmazonS3 s3Client = CommLibS3.buildS3Client(acessKeys[0], acessKeys[1]);
			try {
				for (int i = 0; i < 100; i++) {
					s3Client.putObject(bucketName, keyName, newContext);
				}
			}catch(AmazonServiceException e){
				//TODO:线程内对异常进行处理，不要使用Assert.assertEquals，非预期异常建议抛出去
				Assert.assertEquals(e.getErrorCode(), "GetDBConnectFail");
			} finally {
				if (s3Client != null) {
					s3Client.shutdown();
				}
			}
		}
	}
	
	private void checkCurrentObjectResult() throws Exception {
		GetObjectRequest request = new GetObjectRequest(bucketName, keyName);
		S3Object object = s3Client.getObject(request);
		S3ObjectInputStream s3is = object.getObjectContent();		
		String downloadPath = TestTools.LocalFile.initDownloadPath(localPath, TestTools.getMethodName(),
				Thread.currentThread().getId());
		ObjectUtils.inputStream2File(s3is,downloadPath);
		s3is.close();
        String actMd5 = TestTools.getMD5(downloadPath);
        
		String expMd5 = TestTools.getMD5(newContext.getBytes());
		Assert.assertEquals(actMd5, expMd5, "etag is wrong ");
	}
}