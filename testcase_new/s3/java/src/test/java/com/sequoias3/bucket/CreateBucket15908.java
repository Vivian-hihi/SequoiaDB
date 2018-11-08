package com.sequoias3.bucket;

import java.io.UnsupportedEncodingException;
import java.net.URLDecoder;
import java.net.URLEncoder;
import java.util.List;
import java.util.Random;

import org.apache.http.client.methods.HttpPut;
import org.apache.http.impl.client.CloseableHttpClient;
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
import com.amazonaws.services.s3.model.Bucket;
import com.amazonaws.services.s3.model.Owner;
import com.sequoiadb.exception.BaseException;
import com.sequoias3.testcommon.CommLib;
import com.sequoias3.testcommon.RestClient;
import com.sequoias3.testcommon.S3TestBase;

/**
 * test content: bucket name parameter verification  
 * testlink-case: seqDB-15908  
 * @author wangkexin
 * @Date 2018.09.28
 * @version 1.00
 */
public class CreateBucket15908 extends S3TestBase {
	private boolean runSuccess = false;
	private String clientRegion = "us-east-1";
	private String bucketName[] = new String[12];
	private String illegalBucketName[] = new String[4];
	private String userName = "user15908";
	private String roleName = "normal";
	private AmazonS3 s3Client = null;
	private String accessKey = null;
	private static CloseableHttpClient client;

	@BeforeClass
	private void setUp() throws Exception {
		String[] acessKeys = RestClient.createUser(userName, roleName);
		accessKey = acessKeys[0];
		AWSCredentials credentials = new BasicAWSCredentials(accessKey, acessKeys[1]);
		
		AwsClientBuilder.EndpointConfiguration endpointConfiguration = new AwsClientBuilder.EndpointConfiguration(
				S3TestBase.s3ClientUrl, clientRegion);
		s3Client = AmazonS3ClientBuilder.standard().withEndpointConfiguration(endpointConfiguration)
				.withCredentials(new AWSStaticCredentialsProvider(credentials)).build();
	}

	@Test
	private void testCreateBucket() throws Exception {
		bucketName[0] = "testbucket";
		bucketName[1] = URLEncoder.encode(getRandomString(3), "UTF-8");
		bucketName[2] = URLEncoder.encode(getRandomString(63), "UTF-8");
		bucketName[3] = URLEncoder.encode("~`!@#$%^\"&*()-=+;?<>", "UTF-8");
		bucketName[4] = URLEncoder.encode("Test_Bucket#001$A", "UTF-8");
		bucketName[5] = ".testbucket01";
		bucketName[6] = "test.bucket02";
		bucketName[7] = "testbucket03.";
		bucketName[8] = "test....bucket04";
		bucketName[9] = "test-bucket05";
		bucketName[10] = "-testbucket06";
		bucketName[11] = "testbucket07-";
		for (int i = 0; i < bucketName.length; i++) {
			try {
				createBucket(bucketName[i]);
			} catch (Exception e) {
				e.printStackTrace();
			}
		}
		checkBucketNameResult(12);

		illegalBucketName[0] = URLEncoder.encode(getRandomString(2), "UTF-8");
		illegalBucketName[1] = URLEncoder.encode(getRandomString(64), "UTF-8");
		illegalBucketName[2] = "users";
		illegalBucketName[3] = "USERS";

		for (int i = 0; i < illegalBucketName.length; i++) {
			try {
				createBucket(illegalBucketName[i]);
				Assert.fail("create bucket should fail!");
			} catch (Exception e) {
				String str = e.getMessage().replace("</Code>", "<Code>");
				String[] strs = str.split("<Code>");
				Assert.assertEquals(strs[1], "InvalidBucketName");
			}
			checkIllegalBucketNameResult(i);
		}
		runSuccess = true;
	}

	@AfterClass
	private void tearDown() throws Exception {
		try {
			if (runSuccess) {
				CommLib.clearBuckets(s3Client);
				RestClient.deleteUser(userName);
			}
		} catch (BaseException e) {
			Assert.fail("clean up failed:" + e.getMessage());
		} finally {
			if (s3Client != null) {
				s3Client.shutdown();
			}
		}
	}

	private void createBucket(String bucketName) throws Exception {
		HttpPut request = new HttpPut("http://192.168.10.82:8080/" + bucketName);
		request.setHeader("Authorization", "Credential=" + accessKey);
		client = RestClient.createHttpClient();
		RestClient.sendRequest(client, request);
	}

	private void checkBucketNameResult(int bucketNums) {
		// create one bucket,check the bucket name and owner name
		List<Bucket> buckets = s3Client.listBuckets();
		// check bucket number
		Assert.assertEquals(buckets.size(), bucketNums);

		boolean findBucketFlag = false;
		for (int i = 0; i < buckets.size(); i++) {
			String actBucketName = buckets.get(i).getName();
			// get the create bucket,then check the bucket name and owner
			try {
				if (actBucketName.equals(URLDecoder.decode(bucketName[i], "UTF-8").toLowerCase())) {
					Owner actOwner = buckets.get(i).getOwner();
					Assert.assertEquals(actOwner.getDisplayName(), userName);
					findBucketFlag = true;
					break;
				}
			} catch (UnsupportedEncodingException e) {
				e.printStackTrace();
			}
		}
		Assert.assertTrue(findBucketFlag, " the bucket must be exist!");
	}

	private void checkIllegalBucketNameResult(int i) {
		try {
			Assert.assertEquals(false, doesBucketExist(URLEncoder.encode(illegalBucketName[i], "UTF-8").toLowerCase()));
		} catch (Exception e) {
			e.printStackTrace();
		}
	}

	private String getRandomString(int length) {
		String str = "zxcvbnmlkjhgfdsaqwertyuiopQWERTYUIOPLKJHGFDSAZXCVBNM1234567890";
		Random random = new Random();
		StringBuffer sbuff = new StringBuffer();
		for (int i = 0; i < length; i++) {
			int number = random.nextInt(62);
			sbuff.append(str.charAt(number));
		}
		return sbuff.toString();
	}

	private boolean doesBucketExist(String bucketName) {
		List<Bucket> buckets = s3Client.listBuckets();
		boolean findBucketFlag = false;
		for (int i = 0; i < buckets.size(); i++) {
			String actBucketName = buckets.get(i).getName();
			if (actBucketName.equals(bucketName)) {
				findBucketFlag = true;
				break;
			}
		}
		return findBucketFlag;
	}
}
