package com.sequoias3.partupload.concurrent;

import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.security.MessageDigest;
import java.util.List;
import java.util.concurrent.CopyOnWriteArrayList;

import org.apache.commons.codec.binary.Hex;
import org.testng.Assert;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import com.amazonaws.services.s3.AmazonS3;
import com.amazonaws.services.s3.model.BucketVersioningConfiguration;
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
 * test content: 开启版本控制，多次分段上传相同对象 testlink-case:seqDB-18694
 * 
 * @author wangkexin
 * @Date 2019.7.30
 * @version 1.00
 */
public class UploadPart18694 extends S3TestBase {
	private boolean runSuccess = false;
	private String bucketName = "bucket18694";
	private String keyName = "key18694";
	private AmazonS3 s3Client = null;
	private long fileSize = 500 * 1024;
	private long partSize = 100 * 1024;
	private File localPath = null;
	private File file = null;
	private String filePath = null;
	private List<PartETag> partEtags = new CopyOnWriteArrayList<>();
	private int[] partNums1 = new int[] { 1, 3, 5 };
	private int[] partNums2 = new int[] { 2, 3, 5 };

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
		CommLib.setBucketVersioning(s3Client, bucketName, BucketVersioningConfiguration.ENABLED);
	}

	@Test
	private void testUpload() throws Exception {
		// 指定多个分段上传对象
		String uploadId1 = putObject(partNums1);
		List<PartETag> partEtags1 = partEtags;

		// 再次指定多个分段上传对象
		partEtags = new CopyOnWriteArrayList<>();
		String uploadId2 = putObject(partNums2);
		List<PartETag> partEtags2 = partEtags;

		// 完成分段上传
		PartUploadUtils.completeMultipartUpload(s3Client, bucketName, keyName, uploadId1, partEtags1);
		PartUploadUtils.completeMultipartUpload(s3Client, bucketName, keyName, uploadId2, partEtags2);
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

	private String putObject(int[] partNums) throws Exception {
		String uploadId = PartUploadUtils.initPartUpload(s3Client, bucketName, keyName);
		ThreadExecutor es = new ThreadExecutor();//TODO 并发建议直接写在test里面
		for (int i : partNums) {
			es.addWorker(new ThreadUploadPart18694(i, uploadId));
		}
		es.run();
		return uploadId;
	}

	class ThreadUploadPart18694 {
		private AmazonS3 s3Client = CommLib.buildS3Client();
		private int partNumber;
		private String uploadId;

		public ThreadUploadPart18694(int partNumber, String uploadId) {
			this.partNumber = partNumber;
			this.uploadId = uploadId;
		}

		@ExecuteOrder(step = 1, desc = "分段上传对象")
		public void putObject() {
			try {
				long filepositon = (partNumber - 1) * partSize;
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
		String actMd5List = ObjectUtils.getMd5OfObject(s3Client, localPath, bucketName, keyName, "1");
		String expMd5List = getMd5(partNums2);
		Assert.assertEquals(actMd5List, expMd5List, "version id = 1");

		actMd5List = ObjectUtils.getMd5OfObject(s3Client, localPath, bucketName, keyName, "0");
		expMd5List = getMd5(partNums1);
		Assert.assertEquals(actMd5List, expMd5List, "version id = 0");
	}

	private String getMd5(int[] partNums) throws IOException {//TODO 不能直接用公共方法？
		FileInputStream fileInputStream = null;
		int length = (int) file.length();
		try {
			MessageDigest md5 = MessageDigest.getInstance("MD5");
			fileInputStream = new FileInputStream(file);
			byte[] buffer = new byte[length];
			if (fileInputStream.read(buffer) != -1) {
				for (int i = 0; i < partNums.length; i++) {
					md5.update(buffer, (int) ((partNums[i] - 1) * partSize), (int) partSize);
				}
			}
			return new String(Hex.encodeHex(md5.digest()));
		} catch (Exception e) {
			e.printStackTrace();
			return null;//TODO ??为什么需要返回null？这里可以不用catch
		} finally {
			if (fileInputStream != null) {
				fileInputStream.close();
			}
		}
	}
}
