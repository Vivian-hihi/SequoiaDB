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
import com.sequoias3.testcommon.s3utils.UserUtils;
import org.testng.Assert;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import java.util.List;

/**
 * test content: create multiple buckets to get bucket list information
 * testlink-case: seqDB-15909
 * @author wangkexin
 * @Date 2018.10.16
 * @version 1.00
 */
public class CreateBucket15909 extends S3TestBase {
	private boolean runSuccess = false;
	private String clientRegion = "us-east-1";
	private String bucketName = "bucket15909";
	private String userName = "user15909";
	private String roleName = "normal";
	private final int defaultNums = 100;
	private AmazonS3 s3Client = null;

	@BeforeClass
	private void setUp() throws Exception {
		String[] acessKeys = UserUtils.createUser(userName, roleName);
		AWSCredentials credentials = new BasicAWSCredentials(acessKeys[0], acessKeys[1]);
		
		AwsClientBuilder.EndpointConfiguration endpointConfiguration = new AwsClientBuilder.EndpointConfiguration(
				S3TestBase.s3ClientUrl, clientRegion);
		s3Client = AmazonS3ClientBuilder.standard().withEndpointConfiguration(endpointConfiguration)
				.withCredentials(new AWSStaticCredentialsProvider(credentials)).build();
	}

	@Test
	private void testCreateBucket() throws Exception {
		// create buckets
		createBucketbyNums();
		checkCreateBucketResult(s3Client);
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

	private void createBucketbyNums() {
		for (int i = 0; i < defaultNums; i++) {
			String subBucketName = bucketName + "." + i;
			s3Client.createBucket(subBucketName);
		}
	}

	private void checkCreateBucketResult(AmazonS3 s3Client) {
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
