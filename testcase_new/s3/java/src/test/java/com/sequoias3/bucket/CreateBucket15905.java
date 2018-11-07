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
 * @Description seqDB-15905:maximum number of buckets to be created *
 * @author wuyan
 * @Date 2018.09.30
 * @version 1.00
 */
public class CreateBucket15905 extends S3TestBase {
	private boolean runSuccess = false;
	private String bucketName = "bucket15905";
	private String userName = "user15905";
	private String roleName = "normal";
	private String clientRegion = "us-east-1";
	private final int defaultMaxNums = 100;
	private AmazonS3 s3Client = null;

	@BeforeClass
	private void setUp() throws Exception {		
		String[] acessKeys = RestClient.createUser(userName, roleName);
		AWSCredentials credentials = new BasicAWSCredentials(acessKeys[0], acessKeys[1]);
		AwsClientBuilder.EndpointConfiguration endpointConfiguration = new AwsClientBuilder.EndpointConfiguration(
				S3TestBase.s3ClientUrl, clientRegion);
		s3Client = AmazonS3ClientBuilder.standard().withEndpointConfiguration(endpointConfiguration)
				.withCredentials(new AWSStaticCredentialsProvider(credentials)).build();
		CommLib.clearBuckets(s3Client);
	}

	@Test
	public void testCreateBucket() throws Exception {
		createBucketbyMaxNums();

		// create buckets of the same name fail
		try {
			s3Client.createBucket(new CreateBucketRequest(bucketName));
			Assert.fail("create bucket maxnum is 100 by default!");
		} catch (AmazonS3Exception e) {
			Assert.assertEquals(e.getErrorCode(), "TooManyBuckets");
		}

		checkCreateBucketResult(s3Client);
		runSuccess = true;
	}

	@AfterClass
	private void tearDown() throws Exception {
		try {
			if (runSuccess) {
				CommLib.clearBuckets(s3Client);
				RestClient.deleteUser(userName);
			}
		} finally {
			if (s3Client != null) {
				s3Client.shutdown();
			}
		}
	}

	private void createBucketbyMaxNums() {
		for (int i = 0; i < defaultMaxNums; i++) {
			String subBucketName = bucketName + "." + i;
			s3Client.createBucket(subBucketName);
		}
	}

	private void checkCreateBucketResult(AmazonS3 s3Client) {
		// check bucket nums
		List<Bucket> buckets = s3Client.listBuckets();
		Assert.assertEquals(buckets.size(), defaultMaxNums);

		for (int i = 0; i < buckets.size(); i++) {
			Bucket bucket = buckets.get(i);
			String actBucketName = bucket.getName();
			String actOwner = bucket.getOwner().getDisplayName();
			Assert.assertEquals(actBucketName, bucketName + "." + i);
			Assert.assertEquals(actOwner, userName);
		}
	}

}
