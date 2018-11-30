package com.sequoias3.bucket;

import com.amazonaws.auth.AWSCredentials;
import com.amazonaws.auth.AWSStaticCredentialsProvider;
import com.amazonaws.auth.BasicAWSCredentials;
import com.amazonaws.client.builder.AwsClientBuilder;
import com.amazonaws.services.s3.AmazonS3;
import com.amazonaws.services.s3.AmazonS3ClientBuilder;
import com.amazonaws.services.s3.model.Bucket;
import com.sequoiadb.exception.BaseException;
import com.sequoias3.testcommon.CommLib;
import com.sequoias3.testcommon.S3TestBase;
import com.sequoias3.testcommon.S3ThreadBase;
import com.sequoias3.testcommon.s3utils.UserUtils;
import org.testng.Assert;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import java.util.List;


/**
 * test content: concurrent get bucket information
 * testlink-case: seqDB-15912
 * @author wangkexin
 * @Date 2018.10.16
 * @version 1.00
 */
public class CreateBucket15912 extends S3TestBase {
	private boolean runSuccess = false;
	private String clientRegion = "us-east-1";
	private String userName = "user15912";
	private String bucketName = "bucket15912";
	private String roleName = "normal";
	private final int defaultNums = 30;
	private AmazonS3 s3Client = null;
	private AWSCredentials credentials = null;
	private AwsClientBuilder.EndpointConfiguration endpointConfiguration = null;

	@BeforeClass
	private void setUp() throws Exception {
		String[] acessKeys = UserUtils.createUser(userName, roleName);
		credentials = new BasicAWSCredentials(acessKeys[0], acessKeys[1]);
		endpointConfiguration = new AwsClientBuilder.EndpointConfiguration(
				S3TestBase.s3ClientUrl, clientRegion);
		s3Client = AmazonS3ClientBuilder.standard().withEndpointConfiguration(endpointConfiguration)
				.withCredentials(new AWSStaticCredentialsProvider(credentials)).build();
		createBuckets(s3Client);
	}

	@Test
	private void testGetBuckets() throws Exception {
		GetBucketThread getBuckets = new GetBucketThread();
		getBuckets.start(100);
		Assert.assertTrue(getBuckets.isSuccess(), getBuckets.getErrorMsg());

		runSuccess = true;
	}

	@AfterClass
	private void tearDown() throws Exception {
		try {
			if (runSuccess) {
				CommLib.clearBuckets(s3Client);
				UserUtils.deleteUser(userName);
			}
		} catch (BaseException e) {
			Assert.fail("clean up failed:" + e.getMessage());
		} finally {
			if (s3Client != null) {
				s3Client.shutdown();
			}
		}
	}

	private class GetBucketThread extends S3ThreadBase {
		@Override
		public void exec() throws Exception {
			AmazonS3 s3Client = AmazonS3ClientBuilder.standard().withEndpointConfiguration(endpointConfiguration)
					.withCredentials(new AWSStaticCredentialsProvider(credentials)).build();
			try {
				s3Client.listBuckets();
				checkBucketResult(s3Client);
			} finally {
				if (s3Client != null) {
					s3Client.shutdown();
				}
			}
		}
	}

	private void createBuckets(AmazonS3 s3Client) {
		for (int i = 0; i < defaultNums; i++) {
			String subBucketName = bucketName + "." + i;
			s3Client.createBucket(subBucketName);
		}
	}

	private void checkBucketResult(AmazonS3 s3Client) {
		// check bucket nums
		List<Bucket> buckets = s3Client.listBuckets();
		Assert.assertEquals(buckets.size(), defaultNums);

		for (int i = 0; i < buckets.size(); i++) {
			Bucket bucket = buckets.get(i);
			String actBucketName = bucket.getName();
			String actOwner = bucket.getOwner().getDisplayName();
			Assert.assertEquals(actBucketName, bucketName + "." + i);
			Assert.assertEquals(actOwner, userName);
		}
	}
}
