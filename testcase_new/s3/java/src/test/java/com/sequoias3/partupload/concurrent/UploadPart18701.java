package com.sequoias3.partupload.concurrent;

import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.security.MessageDigest;
import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.CopyOnWriteArrayList;

import org.apache.commons.codec.binary.Hex;
import org.testng.Assert;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import com.amazonaws.services.s3.AmazonS3;
import com.amazonaws.services.s3.model.CreateBucketRequest;
import com.amazonaws.services.s3.model.PartETag;
import com.amazonaws.services.s3.model.UploadPartRequest;
import com.amazonaws.services.s3.model.UploadPartResult;
import com.sequoiadb.threadexecutor.ThreadExecutor;
import com.sequoiadb.threadexecutor.annotation.ExecuteOrder;
import com.sequoias3.testcommon.CommLib;
import com.sequoias3.testcommon.S3TestBase;
import com.sequoias3.testcommon.TestTools;
import com.sequoias3.testcommon.s3utils.ObjectUtils;
import com.sequoias3.testcommon.s3utils.PartUploadUtils;

/**
 * test content: 开启检测开关，上传多个分段不存在partNum为1、不连续分段号且分段长度不一致
 * testlink-case:seqDB-18701
 * 
 * @author wangkexin
 * @Date 2019.7.30
 * @version 1.00
 */
public class UploadPart18701 extends S3TestBase {
	private boolean runSuccess = false;
	private String bucketName = "bucket18701";
	private String keyName = "key18701";
	private AmazonS3 s3Client = null;
	private long m = 1024 * 1024;
	private long fileSize = 70 * m;
	private File localPath = null;
	private File file = null;
	private String filePath = null;
	private String uploadId = "";
	private List<PartETag> partEtags = new CopyOnWriteArrayList<>();
	private List<long[]> partList = new ArrayList<>();

	@BeforeClass
	private void setUp() throws IOException {
		localPath = new File(S3TestBase.workDir + File.separator + TestTools.getClassName());
		filePath = localPath + File.separator + "localFile_" + fileSize + ".txt";

		TestTools.LocalFile.removeFile(localPath);
		TestTools.LocalFile.createDir(localPath.toString());
		TestTools.LocalFile.createFile(filePath, fileSize);
		file = new File(filePath);

		s3Client = CommLib.buildS3Client();
		CommLib.clearBucket(s3Client, bucketName);
		s3Client.createBucket(new CreateBucketRequest(bucketName));

		long[] parts = new long[] { 0, 10 * m, 1 };
		partList.add(parts);
		parts = new long[] { 10 * m, 5 * m, 3 };
		partList.add(parts);
		parts = new long[] { 20 * m, 10 * m, 5 };
		partList.add(parts);
		parts = new long[] { 50 * m, 20 * m, 60 };
		partList.add(parts);
	}

	@Test
	private void testUpload() throws Exception {
		uploadId = PartUploadUtils.initPartUpload(s3Client, bucketName, keyName);
		ThreadExecutor es = new ThreadExecutor();
		for (int i = 0; i < partList.size(); i++) {
			es.addWorker(new ThreadUploadPart18701(partList.get(i)));
		}
		es.run();

		// 完成分段上传
		PartUploadUtils.completeMultipartUpload(s3Client, bucketName, keyName, uploadId, partEtags);
		checkResult();
		runSuccess = true;
	}

	@AfterClass
	private void tearDown() {
		try {
			if (runSuccess) {
				CommLib.clearBucket(s3Client, bucketName);
				TestTools.LocalFile.removeFile(localPath);
			}
		} finally {
			s3Client.shutdown();
		}
	}

	class ThreadUploadPart18701 {
		private AmazonS3 s3Client = CommLib.buildS3Client();
		private long filepositon;
		private long partSize;
		private int partNumber;

		public ThreadUploadPart18701(long[] parts) {
			this.filepositon = parts[0];
			this.partSize = parts[1];
			this.partNumber = (int) parts[2];
		}

		@ExecuteOrder(step = 1, desc = "分段上传对象")
		public void putObject() {
			try {
				UploadPartRequest partRequest = new UploadPartRequest().withFile(file).withFileOffset(filepositon)
						.withPartNumber(partNumber).withPartSize(partSize).withBucketName(bucketName).withKey(keyName)
						.withUploadId(uploadId);
				UploadPartResult uploadPartResult = s3Client.uploadPart(partRequest);
				partEtags.add(uploadPartResult.getPartETag());
			} finally {
				if (s3Client != null) {
					s3Client.shutdown();
				}
			}
		}
	}

	private void checkResult() throws Exception {
		FileInputStream fileInputStream = null;
		int length = (int) file.length();
		try {
			MessageDigest md5 = MessageDigest.getInstance("MD5");
			fileInputStream = new FileInputStream(file);
			byte[] buffer = new byte[length];
			if (fileInputStream.read(buffer) != -1) {
				for (int i = 0; i < partList.size(); i++) {
					md5.update(buffer, (int) partList.get(i)[0], (int) partList.get(i)[1]);
				}
			}
			String expMd5 = new String(Hex.encodeHex(md5.digest()));
			String actMd5 = ObjectUtils.getMd5OfObject(s3Client, localPath, bucketName, keyName);
			Assert.assertEquals(actMd5, expMd5);
		} catch (Exception e) {
			e.printStackTrace();
		} finally {
			if (fileInputStream != null) {
				fileInputStream.close();
			}
		}
	}
}
