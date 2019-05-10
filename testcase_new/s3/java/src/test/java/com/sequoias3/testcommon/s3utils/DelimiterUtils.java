package com.sequoias3.testcommon.s3utils;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.Random;

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

/**
 * @Description delimiter function common class
 * @author wuyan
 * @Date 2019.04.12
 * @version 1.00
 */
public class DelimiterUtils extends S3TestBase {
	private static MediaType type = MediaType.parseMediaType("text/xml;charset=UTF-8");

	/**
	 * set delimiter of bucket *
	 * 
	 * @author wuyan
	 * @param bucketName
	 * @param delimiter
	 */
	public static void putBucketDelimiter(String bucketName, String delimiter) throws HttpClientErrorException {
		putBucketDelimiter(bucketName, delimiter, S3TestBase.s3AccessKeyId);
	}

	/**
	 * set delimiter of bucket *
	 * 
	 * @author wuyan
	 * @param bucketName
	 * @param delimiter
	 * @param accessKeyId
	 *            the oprater user accessKeyId,default user is administrator
	 */
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

	/**
	 * get delimiter info of bucket *
	 * 
	 * @author wuyan
	 * @param delimiter
	 * @param accessKeyId
	 *            the oprater user accessKeyId,default user is administrator
	 * @return the delimiter result info of the getDelimiter operation
	 */
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

	/**
	 * get delimiter info ,than check the correctness of the returned result
	 * information * *
	 * 
	 * @author wuyan
	 * @param bucketName
	 * @param delimiter
	 *            expected delimiter
	 * @param accessKeyId
	 *            the oprater user accessKeyId,default user is administrator
	 */
	public static void checkCurrentDelimiteInfo(String bucketName, String delimiter, String accessKeyId)
			throws Exception {
		DelimiterConfiguration delimiterResult = DelimiterUtils.getDelimiter(bucketName, accessKeyId);
		String curDelimiter = delimiterResult.getDelimiter();
		String curStatus = delimiterResult.getStatus();
		Assert.assertEquals(curDelimiter, delimiter);
		Assert.assertEquals(curStatus, "Normal");
	}

	/**
	 * list objects with delimiter,than check the correctness of the returned
	 * result *
	 * 
	 * @author wuyan
	 * @param s3Client
	 * @param bucketName
	 * @param delimiter
	 *            specify the delimiter to list
	 * @param expKeyList
	 *            generated directory list,expected to match commonPrefixes
	 * @param matchContentsList
	 *            the keys expected to not match delimter
	 */
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

	/**
	 * update delimiter repeatedly
	 * 
	 * @author wuyan
	 * @param bucketName
	 * @param delimiter
	 *            specify the delimiter
	 * @param accessKeyId
	 *            the oprater user accessKeyId,default user is administrator
	 * @return the errorCode of update delimiter,if update delimiter
	 *         success,return 0;if update delimiter fail,return the actual error
	 *         code :409
	 */
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

	/**
	 * update delimiter repeatedly,until the update succeeds within the
	 * specified time
	 * 
	 * @author wuyan
	 * @param bucketName
	 * @param delimiter
	 *            specify the delimiter
	 * @param accessKeyId
	 *            the oprater user accessKeyId,default user is administrator
	 */
	public static void updateDelimiterSuccessAgain(String bucketName, String delimiter, String accessKeyId) {
		// cleanup task execution cycle is 60s.wait for 30s each time,
		// max wait time is 30 min.
		int eachSleepTime = 30000;
		int maxSleepTime = 1800000;
		int alreadySleepTime = 0;
		int errCode = 0;
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

	/**
	 * ersolve error return type, change httpStatusCodeException to
	 * AmazonS3Exception
	 * 
	 * @author wuyan
	 * @param e
	 * @return ersolve the error return type is amazonS3Exception
	 */
	public static AmazonS3Exception httpToAmazon(HttpStatusCodeException e) {
		AmazonS3Exception amazonS3Exception = new AmazonS3Exception(e.getMessage());
		amazonS3Exception.setStatusCode(e.getStatusCode().value());
		JSONObject jsonBody = XML.toJSONObject(e.getResponseBodyAsString());
		JSONObject subjsonBody = jsonBody.getJSONObject("Error");
		amazonS3Exception.setErrorCode(subjsonBody.getString("Code"));
		amazonS3Exception.setErrorMessage(subjsonBody.getString("Message"));
		return amazonS3Exception;
	}

	// TODO:这里生成随机key名的用意是啥，如果随机生成是否需要带上用例ID区分key名
	public static String[] getRandomKeyListWithDelimiter(String delimiter1, String delimiter2, int keyNum) {
		String[] keyNameList = new String[keyNum];
		String str = "zxcvbnmlkjhgfdsaqwertyuiopQWERTYUIOPLKJHGFDSAZXCVBNM1234567890";
		Random random = new Random();
		int keyLength = 0;
		for (int i = 0; i < keyNum; i++) {
			StringBuffer sb = new StringBuffer();
			// 设置key未添加delimiter时的长度为1-10
			keyLength = random.nextInt(10) + 1;
			// 生成keyLength长度的随机字符串
			for (int j = 0; j < keyLength; j++) {
				int number = random.nextInt(62);
				sb.append(str.charAt(number));
			}

			// 在随机生成的字符串的任意位置插入delimiter1和delimiter2
			if (delimiter1 != null) {
				insertDelimiter(sb, keyLength, delimiter1);
			}
			if (delimiter2 != null) {
				insertDelimiter(sb, keyLength, delimiter2);
			}
			keyNameList[i] = sb.toString();
		}
		return keyNameList;
	}

	// TODO:这个方法没有看出来insertDelimiter的意思
	public static void insertDelimiter(StringBuffer sb, int keyLength, String delimiter) {
		Random random = new Random();
		// 随机插入delimiter次数：1-3次
		int insertNum = random.nextInt(3) + 1;
		for (int i = 0; i < insertNum; i++) {
			int offset = random.nextInt(keyLength + 1);
			sb.insert(offset, delimiter);
		}
	}
}
