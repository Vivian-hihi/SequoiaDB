package com.sequoias3.delimiter;

import com.amazonaws.services.s3.AmazonS3;
import com.sequoiadb.commlib.SdbTestBase;
import com.sequoiadb.task.FaultMakeTask;
import com.sequoiadb.task.OperateTask;
import com.sequoiadb.task.TaskMgr;
import com.sequoias3.commlibs3.CommLibS3;
import com.sequoias3.commlibs3.S3TestBase;
import com.sequoias3.commlibs3.TestTools;
import com.sequoias3.commlibs3.s3utils.DelimiterUtils;
import com.sequoias3.commlibs3.s3utils.ObjectUtils;
import com.sequoias3.commlibs3.s3utils.S3NodeRestart;
import com.sequoias3.commlibs3.s3utils.UserUtils;
import com.sequoias3.commlibs3.s3utils.bean.S3NodeWrapper;
import org.testng.Assert;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import java.io.File;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;
import java.util.Random;
import java.util.concurrent.CopyOnWriteArrayList;

/**
 * test content: 创建对象过程中s3节点异常 testlink-case: seqDB-18206
 * @author wangkexin
 * @Date 2019.05.23
 * @version 1.00
 */
public class CreateObjectWithReStartS3N18206 extends S3TestBase {
	private String userName = "user18206";
	private String bucketName = "bucket18206";
	private int objectNums = 100;
	private String delimiter = "#";
	private String objectName = "object18206";
	private List<String> objectNames = new ArrayList<String>();
	private List<String> putObjectNameList = new CopyOnWriteArrayList<String>();
	private String roleName = "normal";
	//TODO:单词写错了
	private String[] acessKeys = null;
	private AmazonS3 s3Client = null;
	//TODO:建议定义成局部变量
	private File localPath = null;
	private boolean runSuccess = false;

	@BeforeClass
	private void setUp() throws IOException {
		CommLibS3.clearUser(userName);
		acessKeys = UserUtils.createUser(userName, roleName);
		s3Client = CommLibS3.buildS3Client(acessKeys[0], acessKeys[1]);
		s3Client.createBucket(bucketName);
		DelimiterUtils.putBucketDelimiter(bucketName, delimiter, acessKeys[0]);
		for (int i = 0; i < objectNums; i++) {
			objectNames.add(objectName + "_" + i + delimiter + "test.txt");
		}
	}

	@Test
	public void test() throws Exception {
		FaultMakeTask faultMakeTask = S3NodeRestart.getFaultMakeTask(new S3NodeWrapper(), 1, 10);
		TaskMgr mgr = new TaskMgr(faultMakeTask);
		for (int i = 0; i < objectNums; i++) {
			//TODO:"content18206"使用多次，可以定义成一个成员变量
			mgr.addTask(new PutObject(objectNames.get(i), "content18206" + i));
		}
		mgr.execute();
		mgr.isAllSuccess();
		List<Exception> eList = mgr.getExceptions();
		for (Exception e : eList) {
			if (!e.getMessage().contains("Connection refused")) {
				throw e;
			}
		}
		// put remaining objects again
		putRemainObjectsAgain();
		checkRandomObjMd5();
		runSuccess = true;
	}

	@AfterClass
	private void tearDown() throws Exception {
		try {
			if (runSuccess) {
				UserUtils.deleteUser(userName);
				TestTools.LocalFile.removeFile(localPath);
			}
		} finally {
			if (s3Client != null) {
				s3Client.shutdown();
			}
		}
	}

	private class PutObject extends OperateTask {
		private String objectName = null;
		private String content = null;

		public PutObject(String objectName, String content) {
			this.objectName = objectName;
			this.content = content;
		}

		@Override
		public void exec() throws Exception {
			AmazonS3 s3Client = CommLibS3.buildS3Client(acessKeys[0], acessKeys[1]);
			try {
				s3Client.putObject(bucketName, objectName, content);
				putObjectNameList.add(objectName);
			} finally {
				if (s3Client != null) {
					s3Client.shutdown();
				}
			}
		}
	}

	private void putRemainObjectsAgain() {
		List<String> remainObjects = new ArrayList<String>();
		remainObjects.addAll(objectNames);
		remainObjects.removeAll(putObjectNameList);
		//TODO：建议创建s3Client连接移到 test()方法中，因为 putRemainVersionsAgain()，
		// checkRandomObjMd5()都有用到该连接，移到test方法中，比较清晰一点
		s3Client = CommLibS3.buildS3Client(acessKeys[0], acessKeys[1]);
		for (String objectName : remainObjects) {
			int index = objectNames.indexOf(objectName);
			s3Client.putObject(bucketName, objectName, "content18206" + String.valueOf(index));
		}
	}

	private void checkRandomObjMd5() throws Exception {
		int index = new Random().nextInt(objectNames.size());
		String keyName = objectNames.get(index);
		localPath = new File(SdbTestBase.workDir + File.separator + TestTools.getClassName());
		String downfileMd5 = ObjectUtils.getMd5OfObject(s3Client, localPath, bucketName, keyName);
		String content = "content18206" + index;
		String expEtag = TestTools.getMD5(content.getBytes());
		Assert.assertEquals(downfileMd5, expEtag, "keyName = " + keyName);
	}
}