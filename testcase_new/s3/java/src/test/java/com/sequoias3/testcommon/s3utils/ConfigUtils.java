package com.sequoias3.testcommon.s3utils;

import java.io.UnsupportedEncodingException;
import java.net.URLEncoder;
import java.nio.charset.StandardCharsets;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

import org.apache.http.client.methods.CloseableHttpResponse;
import org.apache.http.client.methods.HttpDelete;
import org.apache.http.client.methods.HttpGet;
import org.apache.http.client.methods.HttpHead;
import org.apache.http.client.methods.HttpPut;
import org.apache.http.entity.StringEntity;
import org.apache.http.impl.client.CloseableHttpClient;
import org.json.JSONArray;
import org.json.JSONException;
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
import com.amazonaws.services.s3.model.Bucket;
import com.amazonaws.services.s3.model.Owner;
import com.amazonaws.util.DateUtils;
import com.sequoias3.delimiter.DelimiterConfiguration;
import com.sequoias3.region.GetRegionResult;
import com.sequoias3.region.Region;
import com.sequoias3.testcommon.RestClient;
import com.sequoias3.testcommon.S3TestBase;
import com.sequoias3.testcommon.TestRest;
import com.sequoias3.user.UserCommDefind;

public class ConfigUtils extends S3TestBase {
	private static MediaType type = MediaType.parseMediaType("text/xml;charset=UTF-8");

	/******************
	 * 桶管理
	 * 
	 * @throws UnsupportedEncodingException
	 ***********************/

	public static void createBucket(String bucketName, String authorization) throws UnsupportedEncodingException {
		TestRest rest = new TestRest(type);
		try {
			rest.setApi(URLEncoder.encode(bucketName, "UTF-8")).setRequestMethod(HttpMethod.PUT)
					.setRequestHeaders(UserCommDefind.authorization, authorization).setResponseType(String.class)
					.exec();
		} catch (HttpStatusCodeException e) {
			throw httpToAmazon(e);
		}
	}

	public static void createBucket(String bucketName, String regionName, String authorization)
			throws UnsupportedEncodingException {
		TestRest rest = new TestRest(type);
		try {
			rest.setApi(URLEncoder.encode(bucketName, "UTF-8")).setRequestMethod(HttpMethod.PUT)
					.setRequestHeaders(UserCommDefind.authorization, authorization).setResponseType(String.class)
					.setRequestBody("<CreateBucketConfiguration><LocationConstraint>" + regionName
							+ "</LocationConstraint></CreateBucketConfiguration>")
					.exec();
		} catch (HttpStatusCodeException e) {
			throw httpToAmazon(e);
		}
	}

	public static void headBucket(String bucketName, String authorization) throws UnsupportedEncodingException {
		TestRest rest = new TestRest(type);
		try {
			rest.setApi(URLEncoder.encode(bucketName, "UTF-8")).setRequestMethod(HttpMethod.HEAD)
					.setRequestHeaders(UserCommDefind.authorization, authorization).setResponseType(String.class)
					.exec();
		} catch (HttpClientErrorException e) {
			throw httpToAmazonHead(e);
		}
	}

	public static void deleteBucket(String bucketName, String authorization) throws UnsupportedEncodingException {
		TestRest rest = new TestRest(type);
		try {
			rest.setApi(URLEncoder.encode(bucketName, "UTF-8")).setRequestMethod(HttpMethod.DELETE)
					.setRequestHeaders(UserCommDefind.authorization, authorization).setResponseType(String.class)
					.exec();
		} catch (HttpStatusCodeException e) {
			throw httpToAmazon(e);
		}
	}

	public static List<String> listBuckets(String authorization) {
		TestRest rest = new TestRest();
		ResponseEntity<?> resp;
		JSONObject buckets = null;
		try {
			List<String> bucketNames = new ArrayList<>();
			resp = rest.setApi("").setRequestHeaders(UserCommDefind.authorization, authorization)
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

	public static String getBucketLocation(String bucketName, String authorization) {
		TestRest rest = new TestRest();
		ResponseEntity<?> resp;
		try {
			resp = rest.setApi(bucketName + "/?location").setRequestHeaders(UserCommDefind.authorization, authorization)
					.setRequestMethod(HttpMethod.GET).setResponseType(String.class).exec();
			String xmlBody = resp.getBody().toString();
			JSONObject bucketListJSON = XML.toJSONObject(xmlBody);
			return bucketListJSON.getString("LocationConstraint");
		} catch (HttpClientErrorException e) {
			throw httpToAmazon(e);
		}
	}

	public static void setBucketVersioning(String authorization, String bucketName, String versioning) {
		TestRest rest = new TestRest(type);
		try {
			rest.setApi(bucketName + "/?versioning").setRequestMethod(HttpMethod.PUT)
					.setRequestHeaders(UserCommDefind.authorization, authorization)
					.setRequestBody(
							"<VersioningConfiguration><Status>" + versioning + "</Status></VersioningConfiguration>")
					.setResponseType(String.class).exec();
		} catch (HttpStatusCodeException e) {
			throw httpToAmazon(e);
		}
	}

	public static String getBucketVersioning(String authorization, String bucketName) {
		TestRest rest = new TestRest(type);
		String status = "";
		try {
			ResponseEntity<?> resp = rest.setApi(bucketName + "/?versioning")
					.setRequestHeaders(UserCommDefind.authorization, authorization).setRequestMethod(HttpMethod.GET)
					.setResponseType(String.class).exec();

			String body = resp.getBody().toString();
			JSONObject bucketVersioningJSON = XML.toJSONObject(body);
			status = bucketVersioningJSON.getJSONObject("VersioningConfiguration").getString("Status");
		} catch (HttpStatusCodeException e) {
			throw httpToAmazon(e);
		}

		return status;
	}

	public static void putBucketDelimiter(String bucketName, String delimiter, String authorization) {
		DelimiterConfiguration delimiterConfig = new DelimiterConfiguration();
		delimiterConfig.setDelimiter(delimiter);
		TestRest rest = new TestRest(type);
		try {
			rest.setApi(bucketName + "/?delimiter-config").setRequestMethod(HttpMethod.PUT)
					.setRequestHeaders(UserCommDefind.authorization, authorization).setRequestBody(delimiterConfig)
					.setResponseType(String.class).exec();
		} catch (HttpStatusCodeException e) {
			throw httpToAmazon(e);
		}
	}

	public static DelimiterConfiguration getDelimiter(String bucketName, String authorization) {
		TestRest rest = new TestRest(type);
		ResponseEntity<?> resp;
		DelimiterConfiguration result;
		try {
			resp = rest.setApi(bucketName + "/?delimiter-config")
					.setRequestHeaders(UserCommDefind.authorization, authorization).setRequestMethod(HttpMethod.GET)
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

	public static void checkCurrentDelimiteInfo(String bucketName, String delimiter, String authorization) {
		DelimiterConfiguration delimiterResult = getDelimiter(bucketName, authorization);
		String curDelimiter = delimiterResult.getDelimiter();
		String curStatus = delimiterResult.getStatus();
		Assert.assertEquals(curDelimiter, delimiter);
		Assert.assertEquals(curStatus, "Normal");
	}

	/******************
	 * 对象管理
	 * 
	 * @throws Exception
	 ***********************/

	public static void putObject(String bucketName, String objectName, String content, String authorization)
			throws Exception {
		HttpPut request = new HttpPut(S3TestBase.s3ClientUrl + "/" + URLEncoder.encode(bucketName, "UTF-8") + "/"
				+ URLEncoder.encode(objectName, "UTF-8"));
		// RequestHeaders:
		request.setHeader("Authorization", authorization);

		// Requeatbody:
		StringEntity testString = new StringEntity(content, StandardCharsets.UTF_8);
		request.setEntity(testString);
		CloseableHttpClient client = RestClient.createHttpClient();
		RestClient.sendRequest(client, request);
	}

	public static String getObject(String bucketName, String objectName, String authorization) throws Exception {
		String etag = "";
		HttpGet request = new HttpGet(S3TestBase.s3ClientUrl + "/" + URLEncoder.encode(bucketName, "UTF-8") + "/"
				+ URLEncoder.encode(objectName, "UTF-8"));
		request.setHeader("Authorization", authorization);
		CloseableHttpClient client = RestClient.createHttpClient();
		CloseableHttpResponse response = RestClient.sendRequest(client, request);
		etag = response.getFirstHeader("ETag").getValue().replace("\"", "");

		return etag;
	}

	public static void headObject(String bucketName, String objectName, String authorization) throws Exception {
		HttpHead request = new HttpHead(S3TestBase.s3ClientUrl + "/" + URLEncoder.encode(bucketName, "UTF-8") + "/"
				+ URLEncoder.encode(objectName, "UTF-8"));
		request.setHeader("Authorization", authorization);
		CloseableHttpClient client = RestClient.createHttpClient();
		RestClient.sendRequest(client, request);
	}

	public static void deleteObjet(String bucketName, String objectName, String authorization) throws Exception {
		HttpDelete request = new HttpDelete(S3TestBase.s3ClientUrl + "/" + URLEncoder.encode(bucketName, "UTF-8") + "/"
				+ URLEncoder.encode(objectName, "UTF-8"));
		request.setHeader("Authorization", authorization);
		CloseableHttpClient client = RestClient.createHttpClient();
		RestClient.sendRequest(client, request);
	}

	public static void deleteVersion(String bucketName, String objectName, int versionId, String authorization)
			throws Exception {
		HttpDelete request = new HttpDelete(S3TestBase.s3ClientUrl + "/" + URLEncoder.encode(bucketName, "UTF-8") + "/"
				+ URLEncoder.encode(objectName, "UTF-8") + "?versionId=" + versionId);
		request.setHeader("Authorization", authorization);
		CloseableHttpClient client = RestClient.createHttpClient();
		RestClient.sendRequest(client, request);
	}

	public static List<String> listOvjectV2(String bucketName, String authorization) {
		TestRest rest = new TestRest();
		ResponseEntity<?> resp;
		JSONArray contents = new JSONArray();
		List<String> keyNames = new ArrayList<>();

		try {
			resp = rest.setApi(bucketName + "/?list-type=2")
					.setRequestHeaders(UserCommDefind.authorization, authorization).setRequestMethod(HttpMethod.GET)
					.setResponseType(String.class).exec();
			String xmlBody = resp.getBody().toString();
			JSONObject resultJson = XML.toJSONObject(xmlBody);
			JSONObject listBucketResultJson = resultJson.getJSONObject("ListBucketResult");
			try {
				contents = listBucketResultJson.getJSONArray("Contents");
			} catch (JSONException e) {
				Assert.assertEquals(e.getMessage(), "JSONObject[\"Contents\"] not found.");
			}

			for (int i = 0; i < contents.length(); i++) {
				keyNames.add(contents.getJSONObject(i).getString("Key"));
			}
		} catch (HttpStatusCodeException e) {
			throw httpToAmazon(e);
		}

		return keyNames;
	}

	public static JSONObject listObjectsWithDelimiter(String bucketName, String delimiter, String authorization) {
		TestRest rest = new TestRest();
		ResponseEntity<?> resp;
		JSONObject listBucketResultJson = null;

		try {
			resp = rest.setApi(bucketName + "/?list-type=2&delimiter=" + delimiter)
					.setRequestHeaders(UserCommDefind.authorization, authorization).setRequestMethod(HttpMethod.GET)
					.setResponseType(String.class).exec();
			String xmlBody = resp.getBody().toString();
			JSONObject resultJson = XML.toJSONObject(xmlBody);
			listBucketResultJson = resultJson.getJSONObject("ListBucketResult");

		} catch (HttpStatusCodeException e) {
			throw httpToAmazon(e);
		}
		return listBucketResultJson;
	}

	public static void checkListObjV2WithDelimiter(JSONObject ListBucketResultObj, List<String> expCommonPrefixes,
			List<String> expContents) {

		List<String> actCommprefixes = new ArrayList<String>();
		List<String> actContents = new ArrayList<String>();
		JSONArray commonPrefixes = new JSONArray();
		JSONArray contents = new JSONArray();

		try {
			commonPrefixes = ListBucketResultObj.getJSONArray("CommonPrefixes");
		} catch (JSONException e) {
			Assert.assertEquals(e.getMessage(), "JSONObject[\"CommonPrefixes\"] not found.");
		}
		for (int i = 0; i < commonPrefixes.length(); i++) {
			actCommprefixes.add(commonPrefixes.getJSONObject(i).getString("Prefix"));
		}

		try {
			contents = ListBucketResultObj.getJSONArray("Contents");
		} catch (JSONException e) {
			Assert.assertEquals(e.getMessage(), "JSONObject[\"Contents\"] not found.");
		}
		for (int i = 0; i < contents.length(); i++) {
			actContents.add(contents.getJSONObject(i).getString("Key"));
		}
		Collections.sort(expCommonPrefixes);
		Collections.sort(actCommprefixes);
		Assert.assertEquals(actCommprefixes, expCommonPrefixes,
				"actPrefixes:" + actCommprefixes.toString() + "\n expPrefixes:" + expCommonPrefixes.toString());

		Collections.sort(expContents);
		Collections.sort(actContents);
		Assert.assertEquals(actContents, expContents,
				"actcontents: " + actContents.toString() + ", expcontents: " + expContents.toString());

	}

	public static JSONArray listVersions(String authorization, String bucketName) {
		TestRest rest = new TestRest();
		ResponseEntity<?> resp = rest.setApi(bucketName + "/?versions")
				.setRequestHeaders(UserCommDefind.authorization, authorization).setRequestMethod(HttpMethod.GET)
				.setResponseType(String.class).exec();

		String body = resp.getBody().toString();
		JSONObject objectVersioningJSON = XML.toJSONObject(body);
		JSONObject listVersionsResult = objectVersioningJSON.getJSONObject("ListVersionsResult");
		JSONArray version = listVersionsResult.getJSONArray("Version");
		return version;
	}

	/****************** 用户管理 ***********************/
	public static String[] createUser(String name, String role, String authorization) {
		TestRest rest = new TestRest(type);
		try {
			ResponseEntity<?> resp = rest.setApi("/users/?Action=CreateUser&UserName=" + name + "&Role=" + role)
					.setRequestMethod(HttpMethod.POST).setRequestHeaders(UserCommDefind.authorization, authorization)
					.setResponseType(String.class).exec();
			String xmlBody = resp.getBody().toString();
			JSONObject resultJson = XML.toJSONObject(xmlBody);
			JSONObject AccessKeys = resultJson.getJSONObject("AccessKeys");
			String accessKeyID = AccessKeys.getString(UserCommDefind.accessKeyID);
			String secretAccessKey = AccessKeys.getString(UserCommDefind.secretAccessKey);
			return new String[] { accessKeyID, secretAccessKey };
		} catch (HttpStatusCodeException e) {
			throw httpToAmazon(e);
		}
	}

	public static String[] updateUser(String name, String authorization) {
		TestRest rest = new TestRest(type);
		try {
			ResponseEntity<?> resp = rest.setApi("/users/?Action=CreateAccessKey&UserName=" + name)
					.setRequestMethod(HttpMethod.POST).setRequestHeaders(UserCommDefind.authorization, authorization)
					.setResponseType(String.class).exec();
			String xmlBody = resp.getBody().toString();
			JSONObject resultJson = XML.toJSONObject(xmlBody);
			JSONObject AccessKeys = resultJson.getJSONObject("AccessKeys");
			String accessKeyID = AccessKeys.getString(UserCommDefind.accessKeyID);
			String secretAccessKey = AccessKeys.getString(UserCommDefind.secretAccessKey);
			return new String[] { accessKeyID, secretAccessKey };
		} catch (HttpStatusCodeException e) {
			throw httpToAmazon(e);
		}
	}

	public static void deleteUser(String name, String authorization, boolean force) {
		TestRest rest = new TestRest(type);
		try {
			rest.setApi("/users/?Action=DeleteUser&UserName=" + name + "&Force=" + force)
					.setRequestMethod(HttpMethod.POST).setRequestHeaders(UserCommDefind.authorization, authorization)
					.setResponseType(String.class).exec();
		} catch (HttpStatusCodeException e) {
			throw httpToAmazon(e);
		}
	}

	public static String getUser(String name, String authorization) throws HttpClientErrorException {
		TestRest rest = new TestRest(type);
		String accessKeyId = "";
		try {
			ResponseEntity<?> resp = rest.setApi("/users/?Action=GetAccessKey&UserName=" + name)
					.setRequestMethod(HttpMethod.POST).setRequestHeaders(UserCommDefind.authorization, authorization)
					.setResponseType(String.class).exec();
			String xmlBody = resp.getBody().toString();
			JSONObject resultJson = XML.toJSONObject(xmlBody);
			JSONObject AccessKeys = resultJson.getJSONObject("AccessKeys");
			accessKeyId = AccessKeys.getString("AccessKeyID");
		} catch (HttpStatusCodeException e) {
			throw httpToAmazon(e);
		}
		return accessKeyId;
	}

	/****************** 区域管理 ***********************/
	public static void putRegion(Region region, String authorization) throws Exception {
		TestRest rest = new TestRest(type);
		try {
			rest.setApi("/region/?Action=CreateRegion&RegionName=" + region.getName())
					.setRequestHeaders(UserCommDefind.authorization, authorization).setRequestBody(region)
					.setRequestMethod(HttpMethod.POST).setResponseType(String.class).exec();
		} catch (HttpClientErrorException e) {
			throw httpToAmazon(e);
		}
	}

	public static boolean deleteRegion(String regionName, String authorization) throws Exception {
		TestRest rest = new TestRest();
		ResponseEntity<?> resp;
		boolean isDelete = false;
		try {
			resp = rest.setApi("/region/?Action=DeleteRegion&RegionName=" + regionName)
					.setRequestHeaders(UserCommDefind.authorization, authorization).setRequestMethod(HttpMethod.POST)
					.setResponseType(String.class).exec();
			if (resp.getStatusCodeValue() == 204) {
				isDelete = true;
			}
		} catch (HttpClientErrorException e) {
			throw httpToAmazon(e);
		}
		return isDelete;
	}

	public static GetRegionResult getRegion(String regionName, String authorization) throws Exception {
		TestRest rest = new TestRest();
		ResponseEntity<?> resp;
		GetRegionResult result;
		try {
			resp = rest.setApi("/region/?Action=GetRegion&RegionName=" + regionName)
					.setRequestHeaders(UserCommDefind.authorization, authorization).setRequestMethod(HttpMethod.POST)
					.setResponseType(String.class).exec();
			String xmlBody = resp.getBody().toString();
			result = stringToObject(xmlBody);
		} catch (HttpClientErrorException e) {
			throw httpToAmazon(e);
		}
		return result;
	}

	public static boolean headRegion(String regionName, String authorization) throws Exception {
		TestRest rest = new TestRest();
		ResponseEntity<?> resp;
		boolean doesExist = false;
		try {
			resp = rest.setApi("/region/?Action=HeadRegion&RegionName=" + regionName)
					.setRequestHeaders(UserCommDefind.authorization, authorization).setRequestMethod(HttpMethod.POST)
					.setResponseType(String.class).exec();
			if (resp.getStatusCodeValue() == 200) {
				doesExist = true;
			}
		} catch (HttpClientErrorException e) {
			if (e.getStatusCode().value() != 404) {
				throw httpToAmazonHead(e);
			}
		}
		return doesExist;
	}

	public static List<String> listRegions(String authorization) throws Exception {
		TestRest rest = new TestRest();
		ResponseEntity<?> resp;
		List<String> listResult;
		try {
			resp = rest.setApi("/region/?Action=ListRegions")
					.setRequestHeaders(UserCommDefind.authorization, authorization).setRequestMethod(HttpMethod.POST)
					.setResponseType(String.class).exec();
			String xmlBody = resp.getBody().toString();
			JSONObject jsonBody = XML.toJSONObject(xmlBody);
			JSONObject regions = jsonBody.getJSONObject("ListAllRegionsResult");
			Object object = regions.get("Region");
			listResult = new ArrayList<>();
			if (object instanceof JSONArray) {
				JSONArray array = (JSONArray) object;
				for (int i = 0; i < array.length(); i++) {
					listResult.add(array.getString(i));
				}
			} else {
				listResult.add(object.toString());
			}
		} catch (HttpClientErrorException e) {
			throw httpToAmazon(e);
		}
		return listResult;
	}

	private static GetRegionResult stringToObject(String xmlBody) {
		JSONObject jsonBody = XML.toJSONObject(xmlBody);
		JSONObject subjsonBody = jsonBody.getJSONObject("RegionConfiguration");
		Region region = new Region();
		region.withName(subjsonBody.getString("Name"));
		region.withDataCSShardingType(subjsonBody.getString("DataCSShardingType"));
		region.withDataCLShardingType(subjsonBody.getString("DataCLShardingType"));
		region.withDataDomain(subjsonBody.getString("DataDomain"));
		region.withMetaDomain(subjsonBody.getString("MetaDomain"));
		region.withDataLocation(subjsonBody.getString("DataLocation"));
		region.withMetaLocation(subjsonBody.getString("MetaLocation"));
		region.withMetaHisLocation(subjsonBody.getString("MetaHisLocation"));
		GetRegionResult result = new GetRegionResult(region);
		List<Bucket> buckets = new ArrayList<>();
		Object objects = subjsonBody.get("Buckets");
		if (objects instanceof JSONObject) {
			JSONObject jsonObject = (JSONObject) objects;
			Object jsonObjectBucket = jsonObject.get("Bucket");
			if (jsonObjectBucket instanceof JSONArray) {
				JSONArray jsonArray = (JSONArray) jsonObjectBucket;
				for (int i = 0; i < jsonArray.length(); i++) {
					Bucket bucket = new Bucket();
					JSONObject subjsonObject = jsonArray.getJSONObject(i);
					bucket.setName(subjsonObject.getString("Name"));
					bucket.setCreationDate(DateUtils.parseISO8601Date(subjsonObject.getString("CreationDate")));
					buckets.add(bucket);
				}
			} else {
				JSONObject json = (JSONObject) jsonObjectBucket;
				Bucket bucket = new Bucket();
				bucket.setName(json.getString("Name"));
				bucket.setCreationDate(DateUtils.parseISO8601Date(json.getString("CreationDate")));
				buckets.add(bucket);
			}
		}
		result.setBuckets(buckets);
		return result;
	}

	/****************** 公共方法工具类 ***********************/

	public static void checkCreateBucketResult(AmazonS3 s3Client, String bucketName, String userName) {
		// create one bucket,check the bucket name and owner name
		List<Bucket> buckets = s3Client.listBuckets();
		boolean findBucketFlag = false;
		for (int i = 0; i < buckets.size(); i++) {
			String actBucketName = buckets.get(i).getName();
			// get the create bucket,then check the bucket name and owner
			if (actBucketName.equals(bucketName)) {
				Owner actOwner = buckets.get(i).getOwner();
				Assert.assertEquals(actOwner.getDisplayName(), userName);
				findBucketFlag = true;
				break;
			}
		}
		Assert.assertTrue(findBucketFlag, " the bucket must be exist!");
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

	private static AmazonS3Exception httpToAmazonHead(HttpClientErrorException e) {
		AmazonS3Exception amazonS3Exception = new AmazonS3Exception(e.getMessage());
		amazonS3Exception.setStatusCode(e.getStatusCode().value());
		return amazonS3Exception;
	}
}
