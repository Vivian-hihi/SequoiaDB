package com.sequoias3.object.concurrent;

import java.io.IOException;

import org.testng.Assert;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import com.amazonaws.services.s3.AmazonS3;
import com.amazonaws.services.s3.model.ListObjectsV2Result;
import com.sequoiadb.exception.BaseException;
import com.sequoias3.testcommon.CommLib;
import com.sequoias3.testcommon.S3TestBase;
import com.sequoias3.testcommon.S3ThreadBase;
import com.sequoias3.testcommon.TestTools;
import com.sequoias3.testcommon.s3utils.UserUtils;

/**
 * test content: 并发更新和获取对象 
 * testlink-case: seqDB-16493
 * @author wangkexin
 * @Date 2019.01.04
 * @version 1.00
 */
public class UpdateAndGetObject16493 extends S3TestBase {
	private boolean runSuccess = false;	
	private String userName = "user16493";
	private String bucketName = "bucket16493";
	private String keyName = "key16493";
	private String roleName = "normal";
	private String oldContent = "testContentold16493";
	private String newContent = "testContentnew16493";
	private String[] acessKeys = null;
	private AmazonS3 s3Client = null;
	
	@BeforeClass
	private void setUp() throws Exception {	
		CommLib.clearUser(userName);
		acessKeys = UserUtils.createUser(userName, roleName);
		s3Client = CommLib.buildS3Client(acessKeys[0], acessKeys[1]);	
		s3Client.createBucket(bucketName);
		s3Client.putObject(bucketName, keyName, oldContent);
	}
	
	@Test
	public void testUpdateAndGetBucket() throws Exception {
		UpdateObjectThread updateObject = new UpdateObjectThread(keyName);
		GetObjectThread getObject = new GetObjectThread();
		updateObject.start(50);
		getObject.start();
		
		Assert.assertTrue( updateObject.isSuccess(), updateObject.getErrorMsg());
		Assert.assertTrue( getObject.isSuccess(), updateObject.getErrorMsg());
		checkUpdateObjectResult();
		
		runSuccess = true;			
	}
	
	@AfterClass
	private void tearDown() throws Exception {
		try {
			if (runSuccess) {
				UserUtils.deleteUser(userName);
			}
		} catch (BaseException e) {
			Assert.fail("clean up failed:" + e.getMessage());
		} finally {
			if( s3Client != null ){
				s3Client.shutdown();
			}
		}
	}
	
	private class UpdateObjectThread extends S3ThreadBase{
		String keyName;	
		public UpdateObjectThread ( String keyName ){
			this.keyName = keyName;	
		}
		@Override
		public void exec() throws Exception {
			AmazonS3 s3Client = CommLib.buildS3Client(acessKeys[0], acessKeys[1]);	
			try{
				s3Client.putObject(bucketName, keyName, newContent);
			}finally{
				if (s3Client != null) {
					s3Client.shutdown();
				}
			}			
		}		
	}	
	
	private class GetObjectThread extends S3ThreadBase{
		@Override
		public void exec() throws Exception {
			AmazonS3 s3Client = CommLib.buildS3Client(acessKeys[0], acessKeys[1]);	
			try{
				s3Client.getObject(bucketName, keyName);
			}finally{
				if (s3Client != null) {
					s3Client.shutdown();
				}
			}			
		}		
	}
	
	private void checkUpdateObjectResult() throws IOException {
		ListObjectsV2Result listObjectsV2Result = s3Client.listObjectsV2(bucketName);
		Assert.assertEquals(listObjectsV2Result.getObjectSummaries().size(), 1);
		Assert.assertEquals(listObjectsV2Result.getObjectSummaries().get(0).getBucketName(),bucketName, "bucketName is wrong!");
		Assert.assertEquals(listObjectsV2Result.getObjectSummaries().get(0).getKey(),keyName, "keyName is wrong!");
		Assert.assertEquals(listObjectsV2Result.getObjectSummaries().get(0).getETag(),TestTools.getMD5(newContent.getBytes()), "content is wrong!");
	}
}
