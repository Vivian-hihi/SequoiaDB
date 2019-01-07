package com.sequoias3.object;

import java.io.IOException;

import org.testng.Assert;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import com.amazonaws.services.s3.AmazonS3;
import com.amazonaws.services.s3.model.AmazonS3Exception;
import com.amazonaws.services.s3.model.ObjectMetadata;
import com.amazonaws.services.s3.model.S3Object;
import com.sequoiadb.exception.BaseException;
import com.sequoias3.testcommon.CommLib;
import com.sequoias3.testcommon.S3TestBase;
import com.sequoias3.testcommon.S3ThreadBase;
import com.sequoias3.testcommon.TestTools;
import com.sequoias3.testcommon.s3utils.UserUtils;

/**
 * test content: 开启版本控制，并发删除和获取对象（不指定版本） 
 * testlink-case: seqDB-16505
 * @author wangkexin
 * @Date 2019.01.04
 * @version 1.00
 */
public class DeleteAndGetSameObject16505 extends S3TestBase {
	private boolean runSuccess = false;	
	private String userName = "user16505";
	private String bucketName = "bucket16505";
	private String keyName = "key16505";
	private String roleName = "normal";
	private String content = "testContent16505";
	private String[] acessKeys = null;
	private AmazonS3 s3Client = null;
	
	@BeforeClass
	private void setUp() throws Exception {	
		CommLib.clearUser(userName);
		acessKeys = UserUtils.createUser(userName, roleName);
		s3Client = CommLib.buildS3Client(acessKeys[0], acessKeys[1]);	
		s3Client.createBucket(bucketName);
		CommLib.setBucketVersioning(s3Client, bucketName, "Enabled");
		s3Client.putObject(bucketName, keyName, content+"v1");
		s3Client.putObject(bucketName, keyName, content+"v2");
	}
	
	@Test
	public void testDeleteAndGetObject() throws Exception {
		GetObjectThread getObject = new GetObjectThread();
		DeleteObjectThread deleteObject = new DeleteObjectThread();
		getObject.start();
		deleteObject.start();
		
		Assert.assertTrue( deleteObject.isSuccess(), deleteObject.getErrorMsg());
		Assert.assertTrue( getObject.isSuccess(), getObject.getErrorMsg());
		Assert.assertFalse(s3Client.doesObjectExist(bucketName, keyName));
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
	
	private class DeleteObjectThread extends S3ThreadBase{
		@Override
		public void exec() throws Exception {
			AmazonS3 s3Client = CommLib.buildS3Client(acessKeys[0], acessKeys[1]);	
			try{
				s3Client.deleteObject(bucketName, keyName);
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
				S3Object object = s3Client.getObject(bucketName, keyName);
				ObjectMetadata metadata = object.getObjectMetadata();
				checkGetObjectResult(metadata);
			}catch(AmazonS3Exception e){
				Assert.assertEquals(e.getErrorCode(), "NoSuchKey");
			}finally{
				if (s3Client != null) {
					s3Client.shutdown();
				}
			}			
		}		
	}	
	
	private void checkGetObjectResult(ObjectMetadata metadata) throws IOException {
		Assert.assertEquals(metadata.getETag(), TestTools.getMD5((content+"v2").getBytes()),"md5 is wrong!");
		Assert.assertEquals(metadata.getVersionId(), "1");
	}
}
