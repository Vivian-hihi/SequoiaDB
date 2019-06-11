package com.sequoias3.delimiter;

import com.amazonaws.services.s3.AmazonS3;
import com.amazonaws.services.s3.model.BucketVersioningConfiguration;
import com.amazonaws.services.s3.model.PutObjectResult;
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
import java.util.*;
import java.util.concurrent.CopyOnWriteArrayList;

/**
 * test content: 开启版本控制，创建对象过程中s3节点异常 testlink-case: seqDB-18205
 * 
 * @author wangkexin
 * @Date 2019.05.23
 * @version 1.00
 */
public class CreateObjectWithReStartS3N18205 extends S3TestBase {
	private String userName = "user18205";
	private String bucketName = "bucket18205";
	private int objectVersionNums = 100;
	private String delimiter = "#";
	private String objectName = "object18205" + delimiter + "test.png";
	private List<String> contents = new ArrayList<String>();
	//TODO:建议使用ConcurrentHashMap
	private Map<String, String> versionAndmd5Map = Collections.synchronizedMap(new HashMap<String, String>());
	private List<String> putObjectContentList = new CopyOnWriteArrayList<String>();
	private String roleName = "normal";
	private String[] acessKeys = null;
	private AmazonS3 s3Client = null;
	//TODO：只有checkRandomObjMd5() 方法用到，建议定义成局部变量，用完后删除
	private File localPath = null;
	private boolean runSuccess = false;

	@BeforeClass
	private void setUp() throws IOException {
		CommLibS3.clearUser(userName);
		acessKeys = UserUtils.createUser(userName, roleName);
		s3Client = CommLibS3.buildS3Client(acessKeys[0], acessKeys[1]);
		s3Client.createBucket(bucketName);
		CommLibS3.setBucketVersioning(s3Client, bucketName, BucketVersioningConfiguration.ENABLED);
		DelimiterUtils.putBucketDelimiter(bucketName, delimiter, acessKeys[0]);
		for (int i = 0; i < objectVersionNums; i++) {
			contents.add("content18205" + i);
		}
	}

	@Test
	public void test() throws Exception {
		// 并发多线程上传对象A,使对象A存在多个版本
		FaultMakeTask faultMakeTask = S3NodeRestart.getFaultMakeTask(new S3NodeWrapper(), 1, 10);
		TaskMgr mgr = new TaskMgr(faultMakeTask);
		for (int i = 0; i < objectVersionNums; i++) {
			mgr.addTask(new PutObject(objectName, contents.get(i)));
		}
		mgr.execute();
		mgr.isAllSuccess();
		List<Exception> eList = mgr.getExceptions();
		for (Exception e : eList) {
			if (!e.getMessage().contains("Connection refused")) {
				throw e;
			}
		}
		// 继续上传对象A
		putRemainVersionsAgain();
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
				PutObjectResult result = s3Client.putObject(bucketName, objectName, content);
				versionAndmd5Map.put(result.getVersionId(), TestTools.getMD5(content.getBytes()));
				putObjectContentList.add(content);
			} finally {
				if (s3Client != null) {
					s3Client.shutdown();
				}
			}
		}
	}

	private void putRemainVersionsAgain() {
		// remainObjectContents为剩余未上传版本内容
		List<String> remainObjectContents = new ArrayList<String>();
		remainObjectContents.addAll(contents);
		remainObjectContents.removeAll(putObjectContentList);
		//TODO：建议创建s3Client连接移到 test()方法中，因为 putRemainVersionsAgain()，
		// checkRandomObjMd5()都有用到该连接，移到test方法中，比较清晰一点
		s3Client = CommLibS3.buildS3Client(acessKeys[0], acessKeys[1]);
		for (String content : remainObjectContents) {
			PutObjectResult result = s3Client.putObject(bucketName, objectName, content);
			versionAndmd5Map.put(result.getVersionId(), TestTools.getMD5(content.getBytes()));
		}
	}

	private void checkRandomObjMd5() throws Exception {
		//TODO：个人觉得检查一下当前版本和随机挑一个历史版本进行检查，更加严谨一点
		int version = new Random().nextInt(objectVersionNums);
		localPath = new File(SdbTestBase.workDir + File.separator + TestTools.getClassName());
		String downfileMd5 = ObjectUtils.getMd5OfObject(s3Client, localPath, bucketName, objectName,
				String.valueOf(version));
		String expEtag = versionAndmd5Map.get(String.valueOf(version));
		Assert.assertEquals(downfileMd5, expEtag, "keyName = " + objectName + ", versionid = " + version);
	}
}