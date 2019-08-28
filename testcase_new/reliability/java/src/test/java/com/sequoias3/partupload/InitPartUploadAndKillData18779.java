package com.sequoias3.partupload;

import java.io.File;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.Map;

import org.springframework.util.LinkedMultiValueMap;
import org.springframework.util.MultiValueMap;
import org.testng.Assert;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import com.amazonaws.services.s3.AmazonS3;
import com.amazonaws.services.s3.model.AmazonS3Exception;
import com.amazonaws.services.s3.model.ListMultipartUploadsRequest;
import com.amazonaws.services.s3.model.MultipartUpload;
import com.amazonaws.services.s3.model.MultipartUploadListing;
import com.amazonaws.services.s3.model.PartETag;
import com.sequoias3.commlibs3.CommLibS3;
import com.sequoias3.commlibs3.GroupMgr;
import com.sequoias3.commlibs3.GroupWrapper;
import com.sequoias3.commlibs3.NodeWrapper;
import com.sequoias3.commlibs3.S3TestBase;
import com.sequoias3.commlibs3.TestTools;
import com.sequoias3.commlibs3.s3utils.PartUploadUtils;
import com.sequoias3.fault.KillNode;
import com.sequoias3.task.FaultMakeTask;
import com.sequoias3.task.OperateTask;
import com.sequoias3.task.TaskMgr;

/**
 * @Description seqDB-18779 :初始化上传对象过程中db端节点故障
 * @author wuyan
 * @Date 2019.08.12
 * @version 1.00
 */
public class InitPartUploadAndKillData18779 extends S3TestBase {
	private boolean runSuccess = false;
	private AmazonS3 s3Client = null;
	private String bucketName = "bucket18779";
	private String baseKeyName = "/object18779.png";
	private int objectNums = 20;
	private int fileSize = 1024 * 5;
	private String filePath = null;
	private File localPath = null;
	private File file = null;
	private List<String> keyNames = new ArrayList<>();
	private List<String> keyNamesByInitedPart = new ArrayList<>();
	MultiValueMap<String, String> uploads = new LinkedMultiValueMap<String, String>();

	@BeforeClass
	private void setUp() throws IOException {
		localPath = new File(S3TestBase.workDir + File.separator + TestTools.getClassName());
		TestTools.LocalFile.removeFile(localPath);
		TestTools.LocalFile.createDir(localPath.toString());
		filePath = localPath + File.separator + "localFile_" + fileSize + ".txt";
		TestTools.LocalFile.createFile(filePath, fileSize);
		file = new File(filePath);
		s3Client = CommLibS3.buildS3Client();
		CommLibS3.clearBucket(s3Client, bucketName);
		s3Client.createBucket(bucketName);
		for (int i = 0; i < objectNums; i++) {
			String keyName = "/dir" + i + baseKeyName;
			keyNames.add(keyName);
		}

	}

	// http://jira:8080/browse/SEQUOIADBMAINSTREAM-4843
	@Test(enabled = false)
	public void test() throws Exception {
		TaskMgr mgr = new TaskMgr();
		mgr.addTask(new InitPartUpload());

		GroupMgr groupMgr = GroupMgr.getInstance();
		List<GroupWrapper> glist = groupMgr.getAllDataGroup();
		for (int i = 0; i < glist.size(); i++) {
			String groupName = glist.get(i).getGroupName();
			GroupWrapper group = groupMgr.getGroupByName(groupName);
			NodeWrapper node = group.getMaster();
			FaultMakeTask faultTask = KillNode.getFaultMakeTask(node, 0);
			mgr.addTask(faultTask);
			System.out.println("KillNode:i=" + i + "" + node.hostName() + ":" + node.svcName());
		}

		mgr.execute();
		Assert.assertTrue(mgr.isAllSuccess(), mgr.getErrorMsg());
		Assert.assertTrue(groupMgr.checkBusinessWithLSN(120), "node start fail!");

		checkResult(s3Client);
		runSuccess = true;
	}

	@AfterClass
	private void tearDown() {
		try {
			if (runSuccess) {
				CommLibS3.clearBucket(s3Client, bucketName);
				TestTools.LocalFile.removeFile(localPath);
			}
		} finally {
			if (s3Client != null)
				s3Client.shutdown();

		}
	}

	public class InitPartUpload extends OperateTask {
		private AmazonS3 s3Client1 = CommLibS3.buildS3Client();
		private String keyName;

		@Override
		public void exec() throws Exception {
			try {
				System.out.println("--- begin to init.");
				for (int i = 0; i < objectNums; i++) {
					keyName = keyNames.get(i);
					System.out.println("--- begin to " + keyName);
					String partId = PartUploadUtils.initPartUpload(s3Client1, bucketName, keyName);
					System.out.println("--- end to " + keyName);
					keyNamesByInitedPart.add(keyName);
					uploads.add(keyName, partId);
				}
				System.out.println("---end to init.");
			} catch (AmazonS3Exception e) {
				System.out.println("---e=" + e.getStatusCode() + e.getErrorMessage());
				if (e.getStatusCode() != 500) {
					throw new Exception(keyName, e);
				}
			} finally {
				if (s3Client1 != null) {
					s3Client1.shutdown();
				}
			}
		}
	}

	private void completeMultipartUpload(AmazonS3 s3Client, String keyName, String uploadId) {
		List<PartETag> partEtags = PartUploadUtils.partUpload(s3Client, bucketName, keyName, uploadId, file);
		PartUploadUtils.completeMultipartUpload(s3Client, bucketName, keyName, uploadId, partEtags);
	}

	private void checkResult(AmazonS3 s3Client) {
		keyNames.removeAll(keyNamesByInitedPart);
		System.out.println("---keyNames size=" + keyNames.size());
		for (String keyName : keyNames) {
			String uploadId = PartUploadUtils.initPartUpload(s3Client, bucketName, keyName);
			System.out.println("---keyname=" + keyName + "--uploadId=" + uploadId);
			uploads.add(keyName, uploadId);
		}

		// list multipartUploads to check the parts.

		ListMultipartUploadsRequest request = new ListMultipartUploadsRequest(bucketName);
		MultipartUploadListing result = s3Client.listMultipartUploads(request);
		List<String> expCommonPrefixes = new ArrayList<>();
		checkListMultipartUploadsResults(result, expCommonPrefixes, uploads);
		// if initPartUpload success, than compeletMultipartUpload.
		Object[] successkeyNames = uploads.keySet().toArray();
		for (int i = 0; i < uploads.size(); i++) {
			String keyName = successkeyNames[i].toString();
			String uploadId = uploads.get(keyName).get(0);
			completeMultipartUpload(s3Client, keyName, uploadId);
		}

	}

	private void checkListMultipartUploadsResults(MultipartUploadListing result, List<String> expCommonPrefixes,
			MultiValueMap<String, String> expUploads) {
		Collections.sort(expCommonPrefixes);
		List<String> actCommonPrefixes = result.getCommonPrefixes();
		Assert.assertEquals(actCommonPrefixes, expCommonPrefixes, "actCommonPrefixes = " + actCommonPrefixes.toString()
				+ ",expCommonPrefixes = " + expCommonPrefixes.toString());
		List<MultipartUpload> multipartUploads = result.getMultipartUploads();
		MultiValueMap<String, String> actUploads = new LinkedMultiValueMap<String, String>();
		for (MultipartUpload multipartUpload : multipartUploads) {
			String keyName = multipartUpload.getKey();
			String uploadId = multipartUpload.getUploadId();
			actUploads.add(keyName, uploadId);
		}

		Assert.assertEquals(actUploads.size(), expUploads.size(), "actMap = " + actUploads.size() + " -- "
				+ actUploads.toString() + ",expUpload = " + expUploads.size() + "--" + expUploads.toString());
		for (Map.Entry<String, List<String>> entry : expUploads.entrySet()) {
			// System.out.println("---keyName=" + entry.getKey());
			Assert.assertEquals(actUploads.get(entry.getKey()), expUploads.get(entry.getKey()),
					"actMap = " + actUploads.toString() + ",expMap = " + expUploads.toString());
		}
	}
}
