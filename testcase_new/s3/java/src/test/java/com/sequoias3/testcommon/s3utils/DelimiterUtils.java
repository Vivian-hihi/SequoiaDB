package com.sequoias3.testcommon.s3utils;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

import org.json.JSONObject;
import org.json.XML;
import org.springframework.http.HttpMethod;
import org.springframework.http.MediaType;
import org.springframework.http.ResponseEntity;
import org.springframework.web.client.HttpClientErrorException;
import org.springframework.web.client.HttpStatusCodeException;
import org.testng.Assert;

import com.amazonaws.services.s3.AmazonS3;
import com.amazonaws.services.s3.model.AmazonS3Exception;
import com.amazonaws.services.s3.model.ListObjectsV2Request;
import com.amazonaws.services.s3.model.ListObjectsV2Result;
import com.amazonaws.services.s3.model.S3ObjectSummary;
import com.sequoias3.delimiter.DelimiterConfiguration;
import com.sequoias3.testcommon.S3TestBase;
import com.sequoias3.testcommon.TestRest;
import com.sequoias3.user.UserCommDefind;

public class DelimiterUtils extends S3TestBase {
	private static MediaType type = MediaType.parseMediaType("text/xml;charset=UTF-8");

	public static void putBucketDelimiter(String bucketName, String delimiter) throws HttpClientErrorException {
		putBucketDelimiter(bucketName, delimiter, S3TestBase.s3AccessKeyId);
	}

	public static void putBucketDelimiter(String bucketName, String delimiter, String accessKeyId)
			throws HttpClientErrorException {
		DelimiterConfiguration delimiterConfig = new DelimiterConfiguration();
		delimiterConfig.setDelimiter(delimiter);
		TestRest rest = new TestRest(type);
		try {
			ResponseEntity<?> response = rest.setApi("/s3/" + bucketName + "/?delimiter-config")
					.setRequestMethod(HttpMethod.PUT).setRequestHeaders(UserCommDefind.authorization, accessKeyId)
					.setRequestBody(delimiterConfig).setResponseType(String.class).exec();
			int status = response.getStatusCodeValue();
			if (status != 200) {
				System.out.println("put delimiter failed,delimiter = " + delimiterConfig.toString());
			}
		} catch (HttpStatusCodeException e) {
			throw httpToAmazon(e);
		}
	}

	public static DelimiterConfiguration getDelimiter(String bucketName) throws Exception {
		return getDelimiter(bucketName, S3TestBase.s3AccessKeyId);
	}

	public static DelimiterConfiguration getDelimiter(String bucketName, String accessKeyId) throws Exception {
		TestRest rest = new TestRest();
		ResponseEntity<?> resp;
		DelimiterConfiguration result;
		try {
			resp = rest.setApi("/s3/" + bucketName + "/?delimiter-config")
					.setRequestHeaders(UserCommDefind.authorization, accessKeyId).setRequestMethod(HttpMethod.GET)
					.setResponseType(String.class).exec();
			String xmlBody = resp.getBody().toString();
			JSONObject jsonBody = XML.toJSONObject(xmlBody);
			JSONObject subjsonBody = jsonBody.getJSONObject("DelimiterConfiguration");
			String delimiter = subjsonBody.getString("Delimiter");
			String status = subjsonBody.getString("Status");
			result = new DelimiterConfiguration(delimiter, status);

		} catch (HttpStatusCodeException e) {
			throw httpToAmazon(e);
		}
		return result;
	}

	public static void checkCurrentDelimiteInfo(String bucketName, String delimiter) throws Exception {
		checkCurrentDelimiteInfo(bucketName, delimiter, S3TestBase.s3AccessKeyId);
	}

	public static void checkCurrentDelimiteInfo(String bucketName, String delimiter, String accessKeyId)
			throws Exception {
		DelimiterConfiguration delimiterResult = DelimiterUtils.getDelimiter(bucketName, accessKeyId);
		String curDelimiter = delimiterResult.getDelimiter();
		String curStatus = delimiterResult.getStatus();
		Assert.assertEquals(curDelimiter, delimiter);
		Assert.assertEquals(curStatus, "Normal");
	}

	public static void listObjectsWithDelimiter(AmazonS3 s3Client, String bucketName, String delimiter,
			List<String> expKeyList, List<String> matchContentsList) {
		ListObjectsV2Request request = new ListObjectsV2Request().withBucketName(bucketName).withEncodingType("url");
		request.withDelimiter(delimiter);
		ListObjectsV2Result result = s3Client.listObjectsV2(request);
		List<String> commonPrefixes = result.getCommonPrefixes();
		Collections.sort(expKeyList);
		Collections.sort(commonPrefixes);
		Assert.assertEquals(commonPrefixes, expKeyList,
				"actPrefixes:" + commonPrefixes.toString() + "\n ecpPrefixes:" + expKeyList.toString());
		// objects do not match delimiter are displayed in contents,num is 10
		List<String> actContentsList = new ArrayList<>();
		List<S3ObjectSummary> objects = result.getObjectSummaries();
		for (S3ObjectSummary os : objects) {
			String key = os.getKey();
			actContentsList.add(key);
		}

		// check the keyName
		Collections.sort(actContentsList);
		Collections.sort(matchContentsList);
		Assert.assertEquals(actContentsList, matchContentsList, "actcontent:" + actContentsList.toString());
	}

	public static int updateDelimiterAgain(String bucketName, String delimiter, String accessKeyId) {
		int errCode = 0;
		try {
			DelimiterUtils.putBucketDelimiter(bucketName, delimiter, accessKeyId);
		} catch (AmazonS3Exception e) {
			errCode = e.getStatusCode();
			if (errCode != 409 && !e.getErrorCode().contains("DelimiterNotStable")) {
				Assert.fail("update delimiter fail! e=" + e.getStatusCode() + e.getErrorCode());
			}
		}
		return errCode;
	}

	public static void updateDelimiterSuccessAgain(String bucketName, String delimiter) {
		updateDelimiterSuccessAgain(bucketName, delimiter, S3TestBase.s3AccessKeyId);
	}

	public static void updateDelimiterSuccessAgain(String bucketName, String delimiter, String accessKeyId) {
		// cleanup task execution cycle is 60s.wait for 30s each time,
		// max wait time is 30 min.
		int eachSleepTime = 30000;
		int maxSleepTime = 1800000;
		int alreadySleepTime = 0;
		int errCode = updateDelimiterAgain(bucketName, delimiter, accessKeyId);
		do {
			errCode = updateDelimiterAgain(bucketName, delimiter, accessKeyId);
			try {
				Thread.sleep(eachSleepTime);
			} catch (InterruptedException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
			alreadySleepTime += eachSleepTime;
			if (alreadySleepTime > maxSleepTime)
				Assert.fail("update delimiter fail exceeds maximum waiting time:" + alreadySleepTime);
		} while (errCode == 409);
	}

	public static AmazonS3Exception httpToAmazon(HttpStatusCodeException e) {
		AmazonS3Exception amazonS3Exception = new AmazonS3Exception(e.getMessage());
		amazonS3Exception.setStatusCode(e.getStatusCode().value());
		JSONObject jsonBody = XML.toJSONObject(e.getResponseBodyAsString());
		JSONObject subjsonBody = jsonBody.getJSONObject("Error");
		amazonS3Exception.setErrorCode(subjsonBody.getString("Code"));
		amazonS3Exception.setErrorMessage(subjsonBody.getString("Message"));
		return amazonS3Exception;
	}
}
