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
 * test content: 开启版本控制，更新对象过程中db端节点异常
 * testlink-case: seqDB-16461
 * @author wangkexin
 * @Date 2019.01.09
 * @version 1.00
 */
public class UpdateObjectWithKillDataNode16461 extends S3TestBase {
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
		//TODO:"Enabled"建议使用亚马逊的枚举值
		CommLibS3.setBucketVersioning(s3Client, bucketName, "Enabled");
		//TODO:建议创建多个对象，并发对对象内容进行更新，一个对象可能测不到点
		s3Client.putObject(bucketName, keyName, oldContext);
	}

	//TODO:测试步骤与文本用例有点不相符，异常恢复后，需要对更新成功的对象进行检查（历史版本和当前版本），
	// 对于更新失败的对象的需要重新更新，然后进行检查
	@Test
	public void testUpdateObject() throws Exception {
		try {
			GroupWrapper dataGroup = groupMgr.getGroupByName(dataGroupName);
			NodeWrapper priNode = dataGroup.getMaster();
            //TODO:需要强杀集群中所有数据主节点
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