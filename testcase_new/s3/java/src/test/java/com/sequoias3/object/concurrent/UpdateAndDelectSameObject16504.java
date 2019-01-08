package com.sequoias3.object.concurrent;

import org.testng.Assert;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import com.amazonaws.services.s3.AmazonS3;
import com.amazonaws.services.s3.model.ObjectMetadata;
import com.sequoiadb.exception.BaseException;
import com.sequoias3.testcommon.CommLib;
import com.sequoias3.testcommon.S3TestBase;
import com.sequoias3.testcommon.S3ThreadBase;
import com.sequoias3.testcommon.TestTools;
import com.sequoias3.testcommon.s3utils.UserUtils;

/**
 * test content: 开启版本控制，并发更新和删除相同对象 
 * testlink-case: seqDB-16504
 * @author wangkexin
 * @Date 2019.01.04
 * @version 1.00
 */
public class UpdateAndDelectSameObject16504 extends S3TestBase {
	private boolean runSuccess = false;	
	private String userName = "user16504";
	private String bucketName = "bucket16504";
	private String keyName = "key16504";
	private String roleName = "normal";
	private String oldContent = "test16504";
	private String newContent = "newtest16504";
	private String[] acessKeys = null;
	private AmazonS3 s3Client = null;
	
	@BeforeClass
	private void setUp() throws Exception {	
		CommLib.clearUser(userName);
		acessKeys = UserUtils.createUser(userName, roleName);
		s3Client = CommLib.buildS3Client(acessKeys[0], acessKeys[1]);	
		s3Client.createBucket(bucketName);
		CommLib.setBucketVersioning(s3Client, bucketName, "Enabled");
		s3Client.putObject(bucketName, keyName, oldContent);
	}
	
	@Test
	public void testUpdateAndDeleteObject() throws Exception {
		UpdateObjectThread updateObject = new UpdateObjectThread(keyName);
		DeleteObjectThread deleteObject = new DeleteObjectThread(keyName);
		updateObject.start();
		deleteObject.start();
		
		Assert.assertTrue( updateObject.isSuccess(), updateObject.getErrorMsg());
		Assert.assertTrue( deleteObject.isSuccess(), deleteObject.getErrorMsg());
		
		if (s3Client.doesObjectExist(bucketName, keyName)) {
			ObjectMetadata object = s3Client.getObjectMetadata(bucketName, keyName);
			Assert.assertEquals(object.getETag(), TestTools.getMD5(newContent.getBytes()));
		}
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
	
	private class DeleteObjectThread extends S3ThreadBase{
		String keyName;	
		public DeleteObjectThread ( String keyName ){
			this.keyName = keyName;	
		}
		@Override
		public void exec() throws Exception {
			AmazonS3 s3Client = CommLib.buildS3Client(acessKeys[0], acessKeys[1]);	
			try{
				Thread.sleep(30);
				s3Client.deleteObject(bucketName, keyName);
			}finally{
				if (s3Client != null) {
					s3Client.shutdown();
				}
			}			
		}		
	}	
}
