package com.sequoias3.partupload.concurrent;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.OutputStream;
import java.io.RandomAccessFile;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.Map.Entry;
import java.util.Set;

import org.testng.Assert;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.DataProvider;
import org.testng.annotations.Test;

import com.amazonaws.services.s3.AmazonS3;
import com.amazonaws.services.s3.model.ListPartsRequest;
import com.amazonaws.services.s3.model.PartETag;
import com.amazonaws.services.s3.model.PartListing;
import com.amazonaws.services.s3.model.PartSummary;
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
 * @Description seqDB-18769:上传相同分段，其中分段长度不同
 * @Author huangxiaoni
 * @Date 2019.07.26
 */
// TODO:1.这个用例不是并发场景用例，文本用例描述错误，和18682属于同一个；另外这个用例实现有点复杂，第二个分段并发上传，预期结果需要去判断最后一个上传成功的分段去比较
public class UploadPart18769 extends S3TestBase {
	private int runSuccessNum = 0;
	private int expRunSuccessNum = 2;
	private File localPath;
	private String filePath1;
	private String filePath2;
	private String filePath3;
	private String expFilePath;
	private File file1;
	private File file2;
	private File file3;
	private int fileSize = 35 * 1024 * 1024;
	private int firstPartSize = 5 * 1024 * 1024;
	private int fileId = 0;

	private AmazonS3 s3Client;
	private String keyBase = "obj18769";
	private List<String> keys = new ArrayList<>();
	private String uploadId;
	private Map<File, PartETag> secondPartETags = new HashMap<File, PartETag>();

	@DataProvider(name = "secondPartSize")
	private Object[][] generateFirstPartSize() {
		// parameter : secondPartSize1, secondPartSize2, key
		return new Object[][] {
				// test point a: secondPartSize1 < secondPartSize2....
				new Object[] { 6 * 1024 * 1024, 7 * 1024 * 1024, 15 * 1024 * 1024, keyBase + "_1" },
				// test point b: secondPartSize1 > secondPartSize2....
				new Object[] { 20 * 1024 * 1024, 10 * 1024 * 1024, 9 * 1024 * 1024, keyBase + "_2" } };
	}

	@BeforeClass
	private void setUp() throws IOException {
		this.initFile();
		s3Client = CommLib.buildS3Client();
	}

	@Test(dataProvider = "secondPartSize")
	private void test(int secondPartSize1, int secondPartSize2, int secondPartSize3, String key) throws Exception {
		expFilePath = this.createFile(0);
		keys.add(key);
		List<PartETag> partETags = new ArrayList<>();

		// init part upload
		uploadId = PartUploadUtils.initPartUpload(s3Client, S3TestBase.bucketName, key);

		// upload first part
		PartETag firstPartETag = this.uploadFirstPart(file1, key, firstPartSize);
		partETags.add(firstPartETag);

		// upload second part
		ThreadExecutor threadExec = new ThreadExecutor();
		threadExec.addWorker(new ThreadUploadSecondPart(file1, key, secondPartSize1));
		threadExec.addWorker(new ThreadUploadSecondPart(file2, key, secondPartSize2));
		threadExec.addWorker(new ThreadUploadSecondPart(file3, key, secondPartSize3));
		threadExec.run();

		// get second part info, make sure the successfull part
		// get the secondPartETagStr
		ListPartsRequest request = new ListPartsRequest(bucketName, key, uploadId);
		PartListing parts = s3Client.listParts(request);
		PartSummary partSummary = parts.getParts().get(1);
		// get the secondPartETag
		File actFile = null;
		PartETag secondPartETag = null;
		Set<Entry<File, PartETag>> entrySet = secondPartETags.entrySet();
		Iterator<Map.Entry<File, PartETag>> it = entrySet.iterator();
		while (it.hasNext()) {
			Map.Entry<File, PartETag> entry = it.next();
			actFile = entry.getKey();
			secondPartETag = entry.getValue();
			if (secondPartETag.getETag().equals(partSummary.getETag())) {
				break;
			}
		}
		partETags.add(secondPartETag);

		// upload
		PartUploadUtils.completeMultipartUpload(s3Client, bucketName, key, uploadId, partETags);

		// check results
		fileId++;
		long expFileSize = firstPartSize + partSummary.getSize();
		File downloadPath = new File(localPath + File.separator + "download_" + expFileSize + "_" + fileId);
		String downfileMd5 = ObjectUtils.getMd5OfObject(s3Client, downloadPath, bucketName, key);
		File[] files = { file1, file2, file3 };
		this.readFile(file1.getPath(), 0, firstPartSize, expFilePath);
		for (File file : files) {
			if (actFile == file) {
				this.readFile(file.getPath(), firstPartSize, (int) partSummary.getSize(), expFilePath);
				break;
			}
		}
		Assert.assertEquals(downfileMd5, TestTools.getMD5(expFilePath),
				"actFile = " + actFile + ", secondPartSize = " + partSummary.getSize());

		// clear
		runSuccessNum++;
		TestTools.LocalFile.removeFile(expFilePath);
		TestTools.LocalFile.removeFile(downloadPath);
	}

	@AfterClass
	private void tearDown() {
		try {
			if (runSuccessNum == expRunSuccessNum) {
				for (String key : keys) {
					s3Client.deleteObject(S3TestBase.bucketName, key);
				}
				TestTools.LocalFile.removeFile(localPath);
			}
		} finally {
			s3Client.shutdown();
		}
	}

	private class ThreadUploadSecondPart {
		private File file;
		private String key;
		private long partSize;
		private long fileOffset = firstPartSize;

		public ThreadUploadSecondPart(File file, String key, long partSize) {
			this.file = file;
			this.key = key;
			this.partSize = partSize;
		}

		@ExecuteOrder(step = 1)
		private void uploadPart() {
			AmazonS3 s3 = null;
			try {
				s3 = CommLib.buildS3Client();
				UploadPartRequest partRequest = new UploadPartRequest().withFile(file).withFileOffset(fileOffset)
						.withPartNumber(2).withPartSize(partSize).withBucketName(bucketName).withKey(key)
						.withUploadId(uploadId);
				UploadPartResult partResult = s3.uploadPart(partRequest);
				secondPartETags.put(file, partResult.getPartETag());
			} finally {
				if (s3 != null) {
					s3.shutdown();
				}
			}
		}
	}

	private PartETag uploadFirstPart(File file, String key, long partSize) {
		UploadPartRequest partRequest = new UploadPartRequest().withFile(file).withFileOffset(0).withPartNumber(1)
				.withPartSize(partSize).withBucketName(bucketName).withKey(key).withUploadId(uploadId);
		UploadPartResult partResult = s3Client.uploadPart(partRequest);
		return partResult.getPartETag();
	}

	private void initFile() throws IOException {
		localPath = new File(S3TestBase.workDir + File.separator + TestTools.getClassName());
		TestTools.LocalFile.removeFile(localPath);
		TestTools.LocalFile.createDir(localPath.toString());

		filePath1 = this.createFile(fileSize);
		filePath2 = this.createFile(fileSize);
		filePath3 = this.createFile(fileSize);
		file1 = new File(filePath1);
		file2 = new File(filePath2);
		file3 = new File(filePath3);
	}

	private String createFile(int fileSize) throws IOException {
		fileId++;
		String filePath = localPath + File.separator + "localFile_" + fileSize + "_" + fileId + ".txt";
		TestTools.LocalFile.createFile(filePath, fileSize);
		return filePath;
	}

	private void readFile(String filePath, int off, int len, String downloadPath)
			throws FileNotFoundException, IOException {
		RandomAccessFile raf = null;
		OutputStream fos = null;
		try {
			raf = new RandomAccessFile(filePath, "rw");
			fos = new FileOutputStream(downloadPath, true);
			int size = off;
			raf.seek(size);
			int readSize = 0;
			byte[] buf = new byte[off + len];
			readSize = raf.read(buf, off, len);
			fos.write(buf, off, readSize);
		} finally {
			if (raf != null)
				raf.close();
			if (fos != null)
				fos.close();
		}
	}
}