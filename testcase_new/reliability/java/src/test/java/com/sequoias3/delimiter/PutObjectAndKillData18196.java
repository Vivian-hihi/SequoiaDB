package com.sequoias3.delimiter;

import com.amazonaws.services.s3.AmazonS3;
import com.amazonaws.services.s3.model.AmazonS3Exception;
import com.amazonaws.services.s3.model.BucketVersioningConfiguration;
import com.amazonaws.services.s3.model.ObjectMetadata;
import com.amazonaws.services.s3.model.PutObjectResult;
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
 * @Description seqDB-18196 ::开启版本控制，创建对象过程中sdb端节点故障
 * @author wuyan
 * @Date 2019.01.17
 * @version 1.00
 */
public class PutObjectAndKillData18196 extends S3TestBase {
	private boolean runSuccess = false;
	private AmazonS3 s3Client = null;
	private int fileSize = 1024 * 200;
	private int objectNums = 50;
	private int versionNums = 2;
	private String filePath = null;
	private String delimiter = "?";
	private String objectNameBase = "PutObject18196";
	private List<String> objectNames = new ArrayList<String>();
	private List<String> objectNameList = new CopyOnWriteArrayList<String>();
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
		CommLibS3.setBucketVersioning(s3Client, bucketName, BucketVersioningConfiguration.ENABLED);
		for (int i = 0; i < objectNums; i++) {
			objectNames.add(objectNameBase + i + "_" + delimiter);
		}
		// DelimiterUtils.putBucketDelimiter(bucketName, delimiter);
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
		FaultMakeTask faultTask = KillNode.getFaultMakeTask(destHostName, String.valueOf(destPort), 5, 50);
		TaskMgr mgr = new TaskMgr(faultTask);
		for (int i = 0; i < objectNums; i++) {
			mgr.addTask(new PutObject(objectNames.get(i)));
		}
		mgr.execute();
		Assert.assertTrue(mgr.isAllSuccess(), mgr.getErrorMsg());
		//TODO：是否需要考虑停sdb节点可能带来其他的后果
		// put again
		objectNames.removeAll(objectNameList);
		for (String objectName : objectNames) {
			for (int i = 0; i < versionNums; i++) {
				PutObjectResult obj = s3Client.putObject(bucketName, objectName, new File(filePath));
				checkPutResult(obj);
			}
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

		public PutObject(String objectName) {
			this.objectName = objectName;
		}

		@Override
		public void exec() throws Exception {
			try {
				//TODO:打印信息是否可以去掉
				System.out.println("---begin to putobject:" + objectName);
				for (int i = 0; i < versionNums; i++) {
					PutObjectResult obj = s3Client.putObject(bucketName, this.objectName, new File(filePath));
					checkPutResult(obj);
					objectNameList.add(this.objectName);
				}
				System.out.println("---end to putobject:" + objectName);
			} catch (AmazonS3Exception e) {
				System.out.println("---put object fail:" + objectName + " e=" + e.getStatusCode());
				if (e.getStatusCode() != 500) {
					//TODO:可以通过new Exception("",e)将objectName带出去;比如：throw new Exception(objectName,e)
					throw e;
				}
			}
		}
	}

	private void checkPutResult(PutObjectResult obj) throws IOException {
		//TODO：下面两行比较了相同的内容
		Assert.assertEquals(obj.getETag(), TestTools.getMD5(filePath));
		Assert.assertEquals(obj.getETag(), TestTools.getMD5(filePath));
		ObjectMetadata metadata = obj.getMetadata();
		//TODO:打印信息建议去掉
		System.out.println("versionId = " + metadata.getVersionId());
		//TODO:这里给一个范围可能更加严谨一点：[0,versionNums)，比较失败的时候带上objectName和版本以后方便定位
		Assert.assertTrue(Integer.parseInt(metadata.getVersionId()) < versionNums, metadata.getVersionId());
	}
}
