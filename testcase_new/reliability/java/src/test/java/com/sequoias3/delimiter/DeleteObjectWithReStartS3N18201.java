package com.sequoias3.delimiter;

import com.amazonaws.services.s3.AmazonS3;
import com.sequoiadb.task.FaultMakeTask;
import com.sequoiadb.task.OperateTask;
import com.sequoiadb.task.TaskMgr;
import com.sequoias3.commlibs3.CommLibS3;
import com.sequoias3.commlibs3.S3TestBase;
import com.sequoias3.commlibs3.s3utils.DelimiterUtils;
import com.sequoias3.commlibs3.s3utils.S3NodeRestart;
import com.sequoias3.commlibs3.s3utils.UserUtils;
import com.sequoias3.commlibs3.s3utils.bean.S3NodeWrapper;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import java.util.List;

/**
 * test content: 删除对象过程中S3点异常 testlink-case: seqDB-18201
 * 
 * @author wangkexin
 * @Date 2019.01.30
 * @version 1.00
 */

public class DeleteObjectWithReStartS3N18201 extends S3TestBase {
	private String bucketName = "bucket18201";
	private String userName = "user18201";
	private String keyName = "dir1/dir2/key18201?test.txt";
	private String delimiter = "?";
	private String roleName = "normal";
	private String context = "content18201";
	private String[] acessKeys = null;
	private AmazonS3 s3Client = null;
	private boolean runSuccess = false;

	@BeforeClass
	private void setUp() {
		CommLibS3.clearUser(userName);
		acessKeys = UserUtils.createUser(userName, roleName);
		s3Client = CommLibS3.buildS3Client(acessKeys[0], acessKeys[1]);
		CommLibS3.clearBucket(s3Client, bucketName);
		s3Client.createBucket(bucketName);
		CommLibS3.setBucketVersioning(s3Client, bucketName, "Enabled");
		DelimiterUtils.putBucketDelimiter(bucketName, delimiter, acessKeys[0]);

		s3Client.putObject(bucketName, keyName, context);
	}

	@Test
	public void testCreateRegion() throws Exception {
		FaultMakeTask faultMakeTask = S3NodeRestart.getFaultMakeTask(new S3NodeWrapper(), 1, 10);
		TaskMgr mgr = new TaskMgr(faultMakeTask);
		mgr.addTask(new DeleteObject());
		mgr.execute();
		mgr.isAllSuccess();
		List<Exception> eList = mgr.getExceptions();
		for (Exception e : eList) {
			if (!e.getMessage().contains("Connection refused")) {
				throw e;
			}
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

	private class DeleteObject extends OperateTask {
		@Override
		public void exec() throws Exception {
			AmazonS3 s3Client = CommLibS3.buildS3Client(acessKeys[0], acessKeys[1]);
			try {
				for (int i = 0; i < 100; i++) {
					s3Client.deleteObject(bucketName, keyName);
				}
			} finally {
				if (s3Client != null) {
					s3Client.shutdown();
				}
			}
		}
	}
}
