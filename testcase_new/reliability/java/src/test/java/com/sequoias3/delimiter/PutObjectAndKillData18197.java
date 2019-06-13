package com.sequoias3.delimiter;

import com.amazonaws.services.s3.AmazonS3;
import com.amazonaws.services.s3.model.AmazonS3Exception;
import com.amazonaws.services.s3.model.ObjectMetadata;
import com.amazonaws.services.s3.model.PutObjectResult;
import com.amazonaws.services.s3.model.S3Object;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.commlib.GroupMgr;
import com.sequoiadb.commlib.GroupWrapper;
import com.sequoiadb.commlib.SdbTestBase;
import com.sequoiadb.fault.KillNode;
import com.sequoiadb.task.FaultMakeTask;
import com.sequoiadb.task.OperateTask;
import com.sequoiadb.task.TaskMgr;
import com.sequoias3.commlibs3.CommLibS3;
import com.sequoias3.commlibs3.S3TestBase;
import com.sequoias3.commlibs3.TestTools;
import com.sequoias3.commlibs3.s3utils.ObjectUtils;
import org.testng.Assert;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import java.io.File;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.CopyOnWriteArrayList;

/**
 * //TODO:注释标错了
 * @Description seqDB-18197 ::开启版本控制，创建对象过程中sdb端节点故障
 * @author wuyan
 * @Date 2019.01.17
 * @version 1.00
 */
public class PutObjectAndKillData18197 extends S3TestBase {
	private boolean runSuccess = false;
	private AmazonS3 s3Client = null;
	private int fileSize = 1024 * 200;
	private int objectNums = 50;
	private String filePath = null;
	private String delimiter = "?";
	private String objectNameBase = "PutObject18197";
	private List<String> objectNames = new ArrayList<String>();
	private List<String> putSuccessObjectName = new CopyOnWriteArrayList<String>();
	private File localPath = null;
	private Sequoiadb sdb = null;

	@BeforeClass
	private void setUp() throws IOException {
		localPath = new File(SdbTestBase.workDir + File.separator + TestTools.getClassName());
		TestTools.LocalFile.removeFile(localPath);
		TestTools.LocalFile.createDir(localPath.toString());
		filePath = localPath + File.separator + "localFile_" + fileSize + ".txt";
		TestTools.LocalFile.createFile(filePath, fileSize);
		s3Client = CommLibS3.buildS3Client();
		CommLibS3.clearBucket(s3Client, bucketName);
		s3Client.createBucket(bucketName);
		sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "");
	}

	@Test
	public void test() throws Exception {
		// get the source group master hostname and port
		GroupMgr groupMgr = GroupMgr.getInstance();
		List<GroupWrapper> glist = groupMgr.getAllDataGroup();
		String groupName = glist.get(0).getGroupName();
		//TODO:GroupWrapper可以获取主节点，不需要通过sdb获取主节点,所以可以省去用例里面new Sequoiadb的步骤且setup里面
		//sdb的连接没有释放
		String destHostName = sdb.getReplicaGroup(groupName).getMaster().getHostName();
		int destPort = sdb.getReplicaGroup(groupName).getMaster().getPort();
		System.out.println("KillNode:" + destHostName + ":" + destPort);

		// create concurrent tasks
		FaultMakeTask faultTask = KillNode.getFaultMakeTask(destHostName, String.valueOf(destPort), 3, 10);
		TaskMgr mgr = new TaskMgr(faultTask);

		for (int i = 0; i < objectNums; i++) {
			String subKeyName = objectNameBase + i + "_" + delimiter;
			objectNames.add(subKeyName);
			mgr.addTask(new PutObject(objectNames.get(i)));
		}

		mgr.execute();
		Assert.assertTrue(mgr.isAllSuccess(), mgr.getErrorMsg());
		//TODO：是否需要考虑停sdb节点可能带来其他的后果，以下打印信息可以去掉
		System.out.println("----check ojbect");
		for (String objectName : putSuccessObjectName) {
			getObjectAndCheckResult(bucketName, objectName);
		}

		// put again
		objectNames.removeAll(putSuccessObjectName);
		for (String objectName : objectNames) {
			PutObjectResult obj = s3Client.putObject(bucketName, objectName, new File(filePath));
			checkPutResult(obj);
		}
		runSuccess = true;
	}

	@AfterClass
	private void tearDown() {
		try {
			if (runSuccess) {
				CommLibS3.clearBucket(s3Client, bucketName);
				//TODO:没有删除本地创建的文件
			}
		} finally {
			s3Client.shutdown();
		}
	}

	public class PutObject extends OperateTask {
		private String objectName = null;
		private AmazonS3 s3Client1 = CommLibS3.buildS3Client();

		public PutObject(String objectName) {
			this.objectName = objectName;
		}

		@Override
		public void exec() throws Exception {
			try {
				//TODO:打印信息建议去掉
				System.out.println("---begin to put object:" + objectName);
				s3Client1.putObject(bucketName, this.objectName, new File(filePath));
				System.out.println("---end to put object:" + objectName);
				putSuccessObjectName.add(this.objectName);

			} catch (AmazonS3Exception e) {
				System.out.println("---put object:" + this.objectName + "  e:" + e.getStatusCode());
				if (e.getStatusCode() != 500) {
					//TODO:可以通过new Exception("",e)将objectName带出去;比如：throw new Exception(objectName,e)
					throw e;
				}
			} finally {
				if (s3Client1 != null) {
					s3Client1.shutdown();
				}
			}
		}
	}

	//TODO:checkPutResult(PutObjectResult obj)就几行代码，建议直接写在s3Client.putObject()后面
	private void checkPutResult(PutObjectResult obj) throws IOException {
		//TODO：下面两行比较了相同的内容
		Assert.assertEquals(obj.getETag(), TestTools.getMD5(filePath));
		Assert.assertEquals(obj.getETag(), TestTools.getMD5(filePath));
		ObjectMetadata metadata = obj.getMetadata();
		//TODO:打印信息建议去掉
		System.out.println("versionId = " + metadata.getVersionId());
		Assert.assertEquals(metadata.getVersionId(), null, "the versionId is:" + metadata.getVersionId());
	}

	private void getObjectAndCheckResult(String bucketName, String key) throws Exception {
		S3Object object = s3Client.getObject(bucketName, key);
		ObjectMetadata metadata = object.getObjectMetadata();
		//TODO:注释写错了
		// check the versionId is maximum versionId:20
		String versionId = metadata.getVersionId();
		String curVersionId = "null";
		Assert.assertEquals(versionId, curVersionId);

		// check the etag equal to the md5 of the last update content
		String etag = metadata.getETag();
		//TODO:Assert.assertEquals后面建议加上bucketName和key,出问题方便定位
		Assert.assertEquals(etag, TestTools.getMD5(filePath));

		// chect the content
		String downfileMd5 = ObjectUtils.getMd5OfObject(s3Client, localPath, bucketName, key);
		Assert.assertEquals(downfileMd5, TestTools.getMD5(filePath));
	}
}
