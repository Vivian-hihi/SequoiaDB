package com.sequoias3.region;

import java.io.File;
import java.util.ArrayList;
import java.util.List;

import org.testng.Assert;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import com.amazonaws.services.s3.AmazonS3;
import com.amazonaws.services.s3.model.CreateBucketRequest;
import com.amazonaws.services.s3.model.S3Object;
import com.sequoias3.testcommon.CommLib;
import com.sequoias3.testcommon.S3TestBase;
import com.sequoias3.testcommon.S3ThreadBase;
import com.sequoias3.testcommon.TestTools;
import com.sequoias3.testcommon.s3utils.ObjectUtils;
import com.sequoias3.testcommon.s3utils.RegionUtils;

/**
 * test content: 并发更新区域和使用区域
 * testlink-case: seqDB-17336
 * @author wangkexin
 * @Date 2019.01.29
 * @version 1.00
 */

public class CreateObjectsInSameRegion17336 extends S3TestBase{
	private String regionName = "Beijing17336";
	private String bucketName = "bucket17336";
	private String keyName = "object17336";
	private AmazonS3 s3Client = null;
	private int fileSize = 1024 * 1024;
	private File localPath = null;
	private String filePath = null;
	private int objectNums = 50;
	private boolean runSuccess = false;

	@BeforeClass
	private void setUp() throws Exception {
		localPath = new File(S3TestBase.workDir + File.separator + TestTools.getClassName());
		filePath = localPath + File.separator + "localFile_" + fileSize + ".txt";
		TestTools.LocalFile.removeFile(localPath);
		TestTools.LocalFile.createDir(localPath.toString());
		TestTools.LocalFile.createFile(filePath, fileSize);
		
		s3Client = CommLib.buildS3Client();
		CommLib.clearBucket(s3Client, bucketName);
		
		if(RegionUtils.headRegion(regionName)){
			RegionUtils.deleteRegion(regionName);
		}
		Region region = new Region();
        region.withName(regionName);
        RegionUtils.putRegion(region);
        s3Client.createBucket(new CreateBucketRequest(bucketName, regionName.toLowerCase()));
	}
	
	@Test
	public void testCreateRegion() throws Exception {
		List<CreateObjectThread> createObjs = new ArrayList<>(objectNums);
		
		for (int i = 0; i < objectNums; i++) {
			String key = keyName + "_" + i;
			createObjs.add(new CreateObjectThread(key));
		}
		for (CreateObjectThread createObjThread : createObjs) {
			createObjThread.start();
		}
		for (CreateObjectThread createObjThread : createObjs) {
			Assert.assertTrue(createObjThread.isSuccess(), createObjThread.getErrorMsg());
		}
		
		runSuccess = true;
	}
	
	@AfterClass
	private void tearDown() throws Exception {
		try{
			if (runSuccess) {
				CommLib.clearBucket(s3Client, bucketName);
				RegionUtils.deleteRegion(regionName);
				TestTools.LocalFile.removeFile(localPath);
			}
		}finally {
			if (s3Client != null) {
				s3Client.shutdown();
			}
		}
	}
	
	private class CreateObjectThread extends S3ThreadBase{
		private String keyName;
		public CreateObjectThread(String keyName) {
			this.keyName = keyName;
		}
		@Override
		public void exec() throws Exception {
			AmazonS3 s3Client = CommLib.buildS3Client();
			try {
				s3Client.putObject(bucketName, keyName, new File(filePath));
				checkResult(s3Client, keyName);
			} finally {
				if (s3Client != null) {
					s3Client.shutdown();
				}
			}
		}
	}
	
	private void checkResult(AmazonS3 s3Client, String keyName) throws Exception{
		S3Object obj = s3Client.getObject(bucketName, keyName);
		Assert.assertEquals(obj.getKey(), keyName);
		String downfileMd5 = ObjectUtils.getMd5OfObject(s3Client, localPath, bucketName, keyName);
		Assert.assertEquals(downfileMd5, TestTools.getMD5(filePath));
	}
}
