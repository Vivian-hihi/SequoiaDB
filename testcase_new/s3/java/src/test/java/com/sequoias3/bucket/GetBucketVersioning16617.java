package com.sequoias3.bucket;

import com.amazonaws.services.s3.AmazonS3;
import com.sequoias3.testcommon.CommLib;
import com.sequoias3.testcommon.S3TestBase;
import com.sequoias3.testcommon.S3ThreadBase;
import com.sequoias3.testcommon.s3utils.UserUtils;
import org.testng.Assert;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import java.util.ArrayList;
import java.util.List;

/**
 * test content: 并发查询桶版本状态控制信息
 * testlink-case: seqDB-16617
 * @author wangkexin
 * @Date 2018.11.19
 * @version 1.00
 */
public class GetBucketVersioning16617 extends S3TestBase {
	private boolean runSuccess = false;
	private String userName = "user16617";
	private String bucketName = "bucket16617";
	private String roleName = "normal";
	private AmazonS3 s3Client = null;
	private String[] acessKeys = null;
	private final int defaultNums = 100;

	@BeforeClass
	private void setUp() throws Exception {
		acessKeys = UserUtils.createUser(userName, roleName);
		s3Client = CommLib.buildS3Client(acessKeys[0], acessKeys[1]);	
		s3Client.createBucket(bucketName);
		CommLib.setBucketVersioning(s3Client, bucketName, "Enabled");
	}

	@Test
	private void testGetBucketVersioning() throws Exception {
		List<GetBucketVersioningThread> getBucketVersionings = new ArrayList<>(20);
		for( int i = 0; i < defaultNums ; i++){
			getBucketVersionings.add( new GetBucketVersioningThread());			
		}
		for( GetBucketVersioningThread getBucketVersioning : getBucketVersionings ){
			getBucketVersioning.start();
		}	
		
		for( GetBucketVersioningThread getBucketVersioning : getBucketVersionings ){
			Assert.assertTrue( getBucketVersioning.isSuccess(), getBucketVersioning.getErrorMsg());
		}

		runSuccess = true;
	}

	@AfterClass
	private void tearDown() throws Exception {
		try {
			if (runSuccess) {
				CommLib.clearBuckets(s3Client);
				UserUtils.deleteUser(userName);
			}
		} finally {
			if (s3Client != null) {
				s3Client.shutdown();
			}
		}
	}

	private class GetBucketVersioningThread extends S3ThreadBase {
		@Override
		public void exec() throws Exception {
			AmazonS3 s3Client = CommLib.buildS3Client(acessKeys[0], acessKeys[1]);	
			try {
				Assert.assertEquals(s3Client.getBucketVersioningConfiguration(bucketName).getStatus(), "Enabled");
			} finally {
				if (s3Client != null) {
					s3Client.shutdown();
				}
			}
		}
	}
}
