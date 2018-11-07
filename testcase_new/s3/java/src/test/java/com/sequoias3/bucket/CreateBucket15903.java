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
import com.sequoias3.testcommon.RestClient;
import com.sequoias3.testcommon.S3TestBase;

/**
 * @Description seqDB-15903:different users create buckets of the same name *
 * @author wuyan
 * @Date 2018.09.30
 * @version 1.00
 */
public class CreateBucket15903 extends S3TestBase {
	private boolean runSuccess = false;
	private String clientRegion = "us-east-1";
	private String bucketName = "bucket15903";
	private String userName1 = "user15903_a";
	private String userName2 = "user15903_b";
	private String roleName = "normal";
	private AmazonS3 s3Client1 = null;
	private AmazonS3 s3Client2 = null;

	@BeforeClass
	private void setUp() throws Exception {		
		String[] acessKeys1 = RestClient.createUser(userName1, roleName);
		String[] acessKeys2 = RestClient.createUser(userName2, roleName);
		AWSCredentials credentials1 = new BasicAWSCredentials(acessKeys1[0], acessKeys1[1]);
		AWSCredentials credentials2 = new BasicAWSCredentials(acessKeys2[0], acessKeys2[1]);

		AwsClientBuilder.EndpointConfiguration endpointConfiguration = new AwsClientBuilder.EndpointConfiguration(
				S3TestBase.s3ClientUrl, clientRegion);
		s3Client1 = AmazonS3ClientBuilder.standard().withEndpointConfiguration(endpointConfiguration)
				.withCredentials(new AWSStaticCredentialsProvider(credentials1)).build();
		s3Client2 = AmazonS3ClientBuilder.standard().withEndpointConfiguration(endpointConfiguration)
				.withCredentials(new AWSStaticCredentialsProvider(credentials2)).build();		
	}

	@Test
	public void testCreateBucket() throws Exception {
		s3Client1.createBucket(new CreateBucketRequest(bucketName));

		// create buckets of the same name fail by other user
		try {
			s3Client2.createBucket(new CreateBucketRequest(bucketName));
			Assert.fail("create bucket with same name should fail!");
		} catch (AmazonS3Exception e) {
			Assert.assertEquals(e.getErrorCode(), "BucketAlreadyExists");
		}

		int bucketNum1 = 1;
		int bucketNum2 = 0;
		checkCreateBucketResult(s3Client1, bucketName, userName1, bucketNum1);
		checkCreateBucketResult(s3Client2, bucketName, userName2, bucketNum2);
		runSuccess = true;
	}

	@AfterClass
	private void tearDown() throws Exception {
		try {
			if (runSuccess) {
				s3Client1.deleteBucket(bucketName);
				RestClient.deleteUser(userName1);
				RestClient.deleteUser(userName2);
			}
		} finally {
			s3Client1.shutdown();
			s3Client2.shutdown();
		}
	}

	private void checkCreateBucketResult(AmazonS3 s3Client, String bucketName, String userName, int bucketNums) {
		// check bucket nums
		List<Bucket> buckets = s3Client.listBuckets();
		Assert.assertEquals(buckets.size(), bucketNums);

		// create bucket success, than check the bucket name and owner name
		if (buckets.size() != 0) {
			Bucket bucket = buckets.get(0);
			String actOwner = bucket.getOwner().getDisplayName();
			String actBucketName = bucket.getName();
			Assert.assertEquals(actBucketName, bucketName);
			Assert.assertEquals(actOwner, userName);
		}

	}

}
