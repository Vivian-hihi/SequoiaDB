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
 * test content: no bucket and get bucket list information testlink-case:
 * seqDB-15910
 * 
 * @author wangkexin
 * @Date 2018.10.16
 * @version 1.00
 */
public class CreateBucket15910 extends S3TestBase {
	private boolean runSuccess = false;
	private String clientRegion = "us-east-1";
	private String userName = "user15910";
	private String roleName = "normal";
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
		// get bucket information fail
		List<Bucket> buckets = s3Client.listBuckets();
		Assert.assertEquals(buckets.size(), 0, "get bucket information should fail!");
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
}
