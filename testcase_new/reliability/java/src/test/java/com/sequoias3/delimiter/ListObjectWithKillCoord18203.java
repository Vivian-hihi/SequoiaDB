package com.sequoias3.delimiter;

import com.amazonaws.services.s3.AmazonS3;
import com.amazonaws.services.s3.model.AmazonS3Exception;
import com.amazonaws.services.s3.model.BucketVersioningConfiguration;
import com.amazonaws.services.s3.model.ListVersionsRequest;
import com.amazonaws.services.s3.model.VersionListing;
import com.sequoiadb.commlib.SdbTestBase;
import com.sequoiadb.exception.ReliabilityException;
import com.sequoiadb.fault.KillNode;
import com.sequoiadb.task.FaultMakeTask;
import com.sequoiadb.task.OperateTask;
import com.sequoiadb.task.TaskMgr;
import com.sequoias3.commlibs3.CommLibS3;
import com.sequoias3.commlibs3.S3TestBase;
import com.sequoias3.commlibs3.TestTools;
import com.sequoias3.commlibs3.s3utils.DelimiterUtils;
import com.sequoias3.commlibs3.s3utils.ObjectUtils;
import com.sequoias3.commlibs3.s3utils.UserUtils;
import org.springframework.util.LinkedMultiValueMap;
import org.springframework.util.MultiValueMap;
import org.testng.Assert;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import java.io.File;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;

/**
 * test content: 获取对象版本列表过程中db端节点异常 testlink-case: seqDB-18203
 * 
 * @author wangkexin
 * @Date 2019.05.23
 * @version 1.00
 */
public class ListObjectWithKillCoord18203 extends S3TestBase {
	private String userName = "user18200";
	private String bucketName = "bucket18203";
	private String objectName = "/aa/bb/object18203";
	private String delimiter = "?";
	private int objectNums = 100;
	private int versionNum = 3;
	private String[] objectNames = new String[objectNums];
	private AmazonS3 s3Client = null;
	private int fileSize = 1024 * 1;
	private File localPath = null;
	private String filePath = null;
	private String roleName = "normal";
	private List<String> expCommprefixes = new ArrayList<>();
	private MultiValueMap<String, String> expMap = new LinkedMultiValueMap<String, String>();
	private String[] acessKeys = null;
	private boolean runSuccess = false;

	@BeforeClass
	private void setUp() throws IOException {
		localPath = new File(SdbTestBase.workDir + File.separator + TestTools.getClassName());
		filePath = localPath + File.separator + "localFile_" + fileSize + ".txt";

		TestTools.LocalFile.removeFile(localPath);
		TestTools.LocalFile.createDir(localPath.toString());
		TestTools.LocalFile.createFile(filePath, fileSize);

		CommLibS3.clearUser(userName);
		acessKeys = UserUtils.createUser(userName, roleName);
		s3Client = CommLibS3.buildS3Client(acessKeys[0], acessKeys[1]);
		s3Client.createBucket(bucketName);
		CommLibS3.setBucketVersioning(s3Client, bucketName, BucketVersioningConfiguration.ENABLED);
		DelimiterUtils.putBucketDelimiter(bucketName, delimiter, acessKeys[0]);

		for (int i = 0; i < objectNums; i++) {
			String currObjectName = objectName + "_" + i + delimiter + "test.txt";
			for (int v = 0; v < versionNum; v++) {
				s3Client.putObject(bucketName, currObjectName, new File(filePath));
			}
			objectNames[i] = currObjectName;
		}

		expCommprefixes = ObjectUtils.getCommPrefixes(objectNames, "", delimiter);
	}

	@Test
	public void test() throws ReliabilityException, IOException {
		// kill coord when list object versions
		FaultMakeTask faultTask = KillNode.getFaultMakeTask(SdbTestBase.hostName, SdbTestBase.serviceName, 1);
		TaskMgr mgr = new TaskMgr(faultTask);
		ListObject listTask = new ListObject();
		mgr.addTask(listTask);
		mgr.execute();
		Assert.assertTrue(mgr.isAllSuccess(), mgr.getErrorMsg());
		// list objects again
		listVersionsAndCheck();
		runSuccess = true;
	}

	@AfterClass
	private void tearDown() {
		try {
			if (runSuccess) {
				UserUtils.deleteUser(userName);
				TestTools.LocalFile.removeFile(localPath);
			}
		} finally {
			s3Client.shutdown();
		}
	}

	private class ListObject extends OperateTask {
		@Override
		public void exec() throws IOException {
			try {
				listVersionsAndCheck();
			} catch (AmazonS3Exception e) {
				Assert.assertEquals(e.getErrorCode(), "GetDBConnectFail");
			}
		}
	}

	private void listVersionsAndCheck() throws IOException {
		AmazonS3 s3Client = CommLibS3.buildS3Client(acessKeys[0], acessKeys[1]);
		try {
			VersionListing verList = s3Client
					.listVersions(new ListVersionsRequest().withBucketName(bucketName).withDelimiter(delimiter));
			ObjectUtils.checkListVSResults(verList, expCommprefixes, expMap);
		} finally {
			if (s3Client != null) {
				s3Client.shutdown();
			}
		}
	}
}