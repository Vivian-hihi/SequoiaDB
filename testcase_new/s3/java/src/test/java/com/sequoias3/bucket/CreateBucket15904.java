package com.sequoias3.bucket;

import java.util.List;

import org.testng.Assert;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import com.amazonaws.auth.AWSCredentials;
import com.amazonaws.auth.AWSStaticCredentialsProvider;
import com.amazonaws.auth.BasicAWSCredentials;
import com.amazonaws.client.builder.AwsClientBuilder;
import com.amazonaws.services.s3.AmazonS3;
import com.amazonaws.services.s3.AmazonS3ClientBuilder;
import com.amazonaws.services.s3.model.AmazonS3Exception;
import com.amazonaws.services.s3.model.Bucket;
import com.amazonaws.services.s3.model.CreateBucketRequest;
import com.sequoias3.testcommon.CommLib;
import com.sequoias3.testcommon.RestClient;
import com.sequoias3.testcommon.S3TestBase;

/**
 * @Description seqDB-15904:create buckets of the same name * 
 * @author wuyan
 * @Date 2018.09.30
 * @version 1.00
 */
public class CreateBucket15904 extends S3TestBase {
	private boolean runSuccess = false;
	private String bucketName = "bucket15904";
	private String userName = "bucket15904";
	private String clientRegion = "us-east-1";
	private String roleName = "normal";
	private AmazonS3 s3Client = null;

	@BeforeClass
	private void setUp() throws Exception {	
		String[] acessKeys = RestClient.createUser(userName, roleName);
		AWSCredentials credentials = new BasicAWSCredentials(acessKeys[0], acessKeys[1]);
		AwsClientBuilder.EndpointConfiguration endpointConfiguration = new AwsClientBuilder.EndpointConfiguration(
				S3TestBase.s3ClientUrl, clientRegion);
		s3Client = AmazonS3ClientBuilder.standard().withEndpointConfiguration(endpointConfiguration)
				.withCredentials(new AWSStaticCredentialsProvider(credentials)).build();
		CommLib.clearBucket(s3Client, bucketName);
	}

	@Test
	public void testCreateBucket() throws Exception {
		s3Client.createBucket(new CreateBucketRequest(bucketName));

		// create buckets of the same name fail
		try {
			s3Client.createBucket(new CreateBucketRequest(bucketName));
			Assert.fail("create bucket with same name should fail!");
		} catch (AmazonS3Exception e) {
			Assert.assertEquals(e.getErrorCode(), "BucketAlreadyOwnedByYou");
		}

		int bucketNum = 1;
		checkCreateBucketResult(s3Client, bucketName, bucketNum);
		runSuccess = true;
	}

	@AfterClass
	private void tearDown() throws Exception {
		try {
			if (runSuccess) {
				CommLib.clearBucket(s3Client, bucketName);
				RestClient.deleteUser(userName);
			}
		} finally {
			if (s3Client != null) {
				s3Client.shutdown();
			}
		}
	}

	private void checkCreateBucketResult(AmazonS3 s3Client, String bucketName, int bucketNums) {
		// check bucket nums
		List<Bucket> buckets = s3Client.listBuckets();
		Assert.assertEquals(buckets.size(), bucketNums);
		Bucket bucket = buckets.get(0);
		String actOwner = bucket.getOwner().getDisplayName();
		String actBucketName = bucket.getName();
		Assert.assertEquals(actBucketName, bucketName);
		Assert.assertEquals(actOwner, userName);
	}

}
