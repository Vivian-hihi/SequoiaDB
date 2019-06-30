package com.sequoias3.testcommon.s3utils;

import java.io.UnsupportedEncodingException;
import java.util.ArrayList;
import java.util.List;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;
import org.json.XML;
import org.springframework.http.HttpMethod;
import org.springframework.http.ResponseEntity;
import org.springframework.web.client.HttpClientErrorException;
import org.springframework.web.client.HttpStatusCodeException;
import org.testng.Assert;

import com.amazonaws.services.s3.model.AmazonS3Exception;
import com.sequoias3.testcommon.S3TestBase;
import com.sequoias3.testcommon.TestRest;
import com.sequoias3.user.UserCommDefind;

public class S3PathUtils extends S3TestBase {
	private static String addr = "http://" + S3TestBase.s3HostName + ":" + S3TestBase.s3Port + "/s3/";

	/******************
	 * 桶管理
	 * 
	 * @throws UnsupportedEncodingException
	 ***********************/

	public static void createBucket(String bucketName) {
		TestRest rest = new TestRest(addr);
		try {
			rest.setApi(bucketName).setRequestMethod(HttpMethod.PUT)
					.setRequestHeaders(UserCommDefind.authorization,
							UserCommDefind.authValPre + UserUtils.accessKeyId + "/")
					.setResponseType(String.class).exec();
		} catch (HttpStatusCodeException e) {
			throw httpToAmazon(e);
		}
	}

	public static void headBucket(String bucketName) {
		TestRest rest = new TestRest(addr);
		try {
			rest.setApi(bucketName).setRequestMethod(HttpMethod.HEAD)
					.setRequestHeaders(UserCommDefind.authorization,
							UserCommDefind.authValPre + UserUtils.accessKeyId + "/")
					.setResponseType(String.class).exec();
		} catch (HttpClientErrorException e) {
			throw httpToAmazonHead(e);
		}
	}

	public static void deleteBucket(String bucketName) {
		TestRest rest = new TestRest(addr);
		try {
			rest.setApi(bucketName).setRequestMethod(HttpMethod.DELETE)
					.setRequestHeaders(UserCommDefind.authorization,
							UserCommDefind.authValPre + UserUtils.accessKeyId + "/")
					.setResponseType(String.class).exec();
		} catch (HttpStatusCodeException e) {
			throw httpToAmazon(e);
		}
	}

	public static List<String> listBuckets() {
		TestRest rest = new TestRest(addr);
		ResponseEntity<?> resp;
		JSONObject buckets = null;
		try {
			List<String> bucketNames = new ArrayList<>();
			resp = rest.setApi("")
					.setRequestHeaders(UserCommDefind.authorization,
							UserCommDefind.authValPre + UserUtils.accessKeyId + "/")
					.setRequestMethod(HttpMethod.GET).setResponseType(String.class).exec();
			String xmlBody = resp.getBody().toString();
			JSONObject bucketListJSON = XML.toJSONObject(xmlBody);
			try {
				buckets = bucketListJSON.getJSONObject("ListAllMyBucketsResult").getJSONObject("Buckets");
			} catch (JSONException e) {
				Assert.assertEquals(e.getMessage(), "JSONObject[\"Buckets\"] not found.");
			}
			if (buckets != null) {
				Object bucket = buckets.get("Bucket");
				if (bucket instanceof JSONArray) {
					JSONArray array = (JSONArray) bucket;
					for (int i = 0; i < array.length(); i++) {
						bucketNames.add(array.getJSONObject(i).getString("Name"));
					}
				} else {
					bucketNames.add(((JSONObject) bucket).getString("Name"));
				}
			}
			return bucketNames;
		} catch (HttpClientErrorException e) {
			throw httpToAmazon(e);
		}
	}

	public static String getBucketLocation(String bucketName) {
		TestRest rest = new TestRest(addr);
		ResponseEntity<?> resp;
		try {
			resp = rest.setApi(bucketName + "/?location")
					.setRequestHeaders(UserCommDefind.authorization,
							UserCommDefind.authValPre + UserUtils.accessKeyId + "/")
					.setRequestMethod(HttpMethod.GET).setResponseType(String.class).exec();
			String xmlBody = resp.getBody().toString();
			JSONObject bucketListJSON = XML.toJSONObject(xmlBody);
			return bucketListJSON.getString("LocationConstraint");
		} catch (HttpClientErrorException e) {
			throw httpToAmazon(e);
		}
	}

	public static void setBucketVersioning(String bucketName, String versioning) {
		TestRest rest = new TestRest(addr);
		try {
			rest.setApi(bucketName + "/?versioning").setRequestMethod(HttpMethod.PUT)
					.setRequestHeaders(UserCommDefind.authorization,
							UserCommDefind.authValPre + UserUtils.accessKeyId + "/")
					.setRequestBody(
							"<VersioningConfiguration><Status>" + versioning + "</Status></VersioningConfiguration>")
					.setResponseType(String.class).exec();
		} catch (HttpStatusCodeException e) {
			throw httpToAmazon(e);
		}
	}

	public static String getBucketVersioning(String bucketName) {
		TestRest rest = new TestRest(addr);
		String status = "";
		try {
			ResponseEntity<?> resp = rest.setApi(bucketName + "/?versioning")
					.setRequestHeaders(UserCommDefind.authorization,
							UserCommDefind.authValPre + UserUtils.accessKeyId + "/")
					.setRequestMethod(HttpMethod.GET).setResponseType(String.class).exec();

			String body = resp.getBody().toString();
			JSONObject bucketVersioningJSON = XML.toJSONObject(body);
			status = bucketVersioningJSON.getJSONObject("VersioningConfiguration").getString("Status");
		} catch (HttpStatusCodeException e) {
			throw httpToAmazon(e);
		}

		return status;
	}

	/******************
	 * 对象管理
	 * 
	 * @throws Exception
	 ***********************/

	public static void putObject(String bucketName, String objectName, String content) {
		TestRest rest = new TestRest(addr);
		try {
			rest.setApi(bucketName + "/" + objectName).setRequestMethod(HttpMethod.PUT)
					.setRequestHeaders(UserCommDefind.authorization,
							UserCommDefind.authValPre + UserUtils.accessKeyId + "/")
					.setRequestBody(content).setResponseType(String.class).exec();
		} catch (HttpStatusCodeException e) {
			throw httpToAmazon(e);
		}
	}

	public static String getObject(String bucketName, String objectName) {
		ResponseEntity<?> resp;
		TestRest rest = new TestRest(addr);
		try {
			resp = rest.setApi(bucketName + "/" + objectName).setRequestMethod(HttpMethod.GET)
					.setRequestHeaders(UserCommDefind.authorization,
							UserCommDefind.authValPre + UserUtils.accessKeyId + "/")
					.setResponseType(String.class).exec();
			return resp.getHeaders().getETag();
		} catch (HttpStatusCodeException e) {
			throw httpToAmazon(e);
		}
	}

	public static void headObject(String bucketName, String objectName) {
		TestRest rest = new TestRest(addr);
		try {
			rest.setApi(bucketName + "/" + objectName).setRequestMethod(HttpMethod.HEAD)
					.setRequestHeaders(UserCommDefind.authorization,
							UserCommDefind.authValPre + UserUtils.accessKeyId + "/")
					.setResponseType(String.class).exec();
		} catch (HttpClientErrorException e) {
			throw httpToAmazonHead(e);
		}
	}

	public static void deleteObjet(String bucketName, String objectName) {
		TestRest rest = new TestRest(addr);
		try {
			rest.setApi(bucketName + "/" + objectName).setRequestMethod(HttpMethod.DELETE)
					.setRequestHeaders(UserCommDefind.authorization,
							UserCommDefind.authValPre + UserUtils.accessKeyId + "/")
					.setResponseType(String.class).exec();
		} catch (HttpClientErrorException e) {
			throw httpToAmazonHead(e);
		}
	}

	public static void deleteVersion(String bucketName, String objectName, int versionId) {
		TestRest rest = new TestRest(addr);
		try {
			rest.setApi(bucketName + "/" + objectName + "?versionId=" + versionId).setRequestMethod(HttpMethod.DELETE)
					.setRequestHeaders(UserCommDefind.authorization,
							UserCommDefind.authValPre + UserUtils.accessKeyId + "/")
					.setResponseType(String.class).exec();
		} catch (HttpClientErrorException e) {
			throw httpToAmazonHead(e);
		}
	}

	public static List<String> listOvjectV2(String bucketName) {
		TestRest rest = new TestRest(addr);
		ResponseEntity<?> resp;
		Object contents = null;
		List<String> keyNames = new ArrayList<>();

		try {
			resp = rest.setApi(bucketName + "/?list-type=2")
					.setRequestHeaders(UserCommDefind.authorization,
							UserCommDefind.authValPre + UserUtils.accessKeyId + "/")
					.setRequestMethod(HttpMethod.GET).setResponseType(String.class).exec();
			String xmlBody = resp.getBody().toString();
			JSONObject resultJson = XML.toJSONObject(xmlBody);
			JSONObject listBucketResultJson = resultJson.getJSONObject("ListBucketResult");
			try {
				contents = listBucketResultJson.get("Contents");
			} catch (JSONException e) {
				Assert.assertEquals(e.getMessage(), "JSONObject[\"Contents\"] not found.");
			}
			if (contents != null) {
				if (contents instanceof JSONArray) {
					JSONArray array = (JSONArray) contents;
					for (int i = 0; i < array.length(); i++) {
						keyNames.add(array.getJSONObject(i).getString("Key"));
					}
				} else {
					keyNames.add(((JSONObject) contents).getString("Key"));
				}
			}
		} catch (HttpStatusCodeException e) {
			throw httpToAmazon(e);
		}
		return keyNames;
	}

	public static JSONArray listVersions(String bucketName) {
		TestRest rest = new TestRest(addr);
		Object version = new JSONArray();
		try {
			ResponseEntity<?> resp = rest.setApi(bucketName + "/?versions")
					.setRequestHeaders(UserCommDefind.authorization,
							UserCommDefind.authValPre + UserUtils.accessKeyId + "/")
					.setRequestMethod(HttpMethod.GET).setResponseType(String.class).exec();

			String body = resp.getBody().toString();
			JSONObject objectVersioningJSON = XML.toJSONObject(body);
			JSONObject listVersionsResult = objectVersioningJSON.getJSONObject("ListVersionsResult");

			try {
				version = listVersionsResult.get("Version");
			} catch (JSONException e) {
				Assert.assertEquals(e.getMessage(), "JSONObject[\"Version\"] not found.");
			}
			if (version instanceof JSONArray) {
				return (JSONArray) version;
			} else {
				JSONArray array = new JSONArray();
				return array.put(version);
			}
		} catch (HttpStatusCodeException e) {
			throw httpToAmazon(e);
		}
	}

	/****************** 公共方法类 ********************/

	public static AmazonS3Exception httpToAmazon(HttpStatusCodeException e) {
		AmazonS3Exception amazonS3Exception = new AmazonS3Exception(e.getMessage());
		amazonS3Exception.setStatusCode(e.getStatusCode().value());
		JSONObject jsonBody = XML.toJSONObject(e.getResponseBodyAsString());
		JSONObject subjsonBody = jsonBody.getJSONObject("Error");
		amazonS3Exception.setErrorCode(subjsonBody.getString("Code"));
		amazonS3Exception.setErrorMessage(subjsonBody.getString("Message"));
		return amazonS3Exception;
	}

	private static AmazonS3Exception httpToAmazonHead(HttpClientErrorException e) {
		AmazonS3Exception amazonS3Exception = new AmazonS3Exception(e.getMessage());
		amazonS3Exception.setStatusCode(e.getStatusCode().value());
		return amazonS3Exception;
	}
}
