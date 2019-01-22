package com.sequoias3.testcommon.s3utils;

import com.amazonaws.services.s3.model.AmazonS3Exception;
import com.amazonaws.services.s3.model.Bucket;
import com.amazonaws.util.DateUtils;
import com.sequoiadb.base.CollectionSpace;
import com.sequoiadb.base.Sequoiadb;
import com.sequoias3.region.GetRegionResult;
import com.sequoias3.region.Region;
import com.sequoias3.testcommon.S3TestBase;
import com.sequoias3.testcommon.TestRest;
import com.sequoias3.user.UserCommDefind;
import org.json.JSONArray;
import org.json.JSONObject;
import org.json.XML;
import org.springframework.http.HttpMethod;
import org.springframework.http.MediaType;
import org.springframework.http.ResponseEntity;
import org.springframework.web.client.HttpClientErrorException;

import java.util.ArrayList;
import java.util.List;

public class RegionUtils extends S3TestBase {
	private static MediaType type = MediaType.parseMediaType("text/xml;charset=UTF-8");

	public static void putRegion(Region region) throws Exception {
		TestRest rest = new TestRest(type);
		ResponseEntity<?> resp;
		try {
			resp = rest.setApi("/region/" + region.getName())
					.setRequestHeaders(UserCommDefind.authorization, "ABCDEFGHIJKLMNOPQRST").setRequestBody(region)
					.setRequestMethod(HttpMethod.PUT).setResponseType(String.class).exec();
			int status = resp.getStatusCodeValue();
			if (status != 200) {
				System.out.println("put region failed,region = " + region.toString());
			}
		} catch (HttpClientErrorException e) {
			throw httpToAmazon(e);
		}
	}

	public static boolean deleteRegion(String regionName) throws Exception {
		TestRest rest = new TestRest();
		ResponseEntity<?> resp;
		boolean isDelete = false;
		try {
			resp = rest.setApi("/region/" + regionName)
					.setRequestHeaders(UserCommDefind.authorization, "ABCDEFGHIJKLMNOPQRST")
					.setRequestMethod(HttpMethod.DELETE).setResponseType(String.class).exec();
			if (resp.getStatusCodeValue() == 204) {
				isDelete = true;
			}
		} catch (HttpClientErrorException e) {
			throw httpToAmazon(e);
		}
		return isDelete;
	}

	public static GetRegionResult getRegion(String regionName) throws Exception {
		TestRest rest = new TestRest();
		ResponseEntity<?> resp;
		GetRegionResult result;
		try {
			resp = rest.setApi("/region/" + regionName)
					.setRequestHeaders(UserCommDefind.authorization, "ABCDEFGHIJKLMNOPQRST")
					.setRequestMethod(HttpMethod.GET).setResponseType(String.class).exec();
			String xmlBody = resp.getBody().toString();
			result = stringToObject(xmlBody);
		} catch (HttpClientErrorException e) {
			throw httpToAmazon(e);
		}
		return result;
	}

	public static boolean headRegion(String regionName) throws Exception {
		TestRest rest = new TestRest();
		ResponseEntity<?> resp;
		boolean doesExist = false;
		try {
			resp = rest.setApi("/region/" + regionName)
					.setRequestHeaders(UserCommDefind.authorization, "ABCDEFGHIJKLMNOPQRST")
					.setRequestMethod(HttpMethod.HEAD).setResponseType(String.class).exec();
			if (resp.getStatusCodeValue() == 200) {
				doesExist = true;
			}
		} catch (HttpClientErrorException e) {
			if( e.getStatusCode().value() != 404 ){
				throw httpToAmazon(e);
			}			
		}
		return doesExist;
	}

	public static List<String> listRegions() throws Exception {
		TestRest rest = new TestRest();
		ResponseEntity<?> resp;
		List<String> listResult;
		try {
			resp = rest.setApi("/region").setRequestHeaders(UserCommDefind.authorization, "ABCDEFGHIJKLMNOPQRST")
					.setRequestMethod(HttpMethod.GET).setResponseType(String.class).exec();
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
			JSONArray jsonArray = jsonObject.getJSONArray("Bucket");
			for (int i = 0; i < jsonArray.length(); i++) {
				Bucket bucket = new Bucket();
				JSONObject subjsonObject = jsonArray.getJSONObject(i);
				bucket.setName(subjsonObject.getString("Name"));
				bucket.setCreationDate(DateUtils.parseISO8601Date(subjsonObject.getString("CreationDate")));
				buckets.add(bucket);
			}
		}
		result.setBuckets(buckets);
		return result;
	}

	private static AmazonS3Exception httpToAmazon(HttpClientErrorException e) {
		AmazonS3Exception amazonS3Exception = new AmazonS3Exception(e.getMessage());
		amazonS3Exception.setStatusCode(e.getStatusCode().value());
		JSONObject jsonBody = XML.toJSONObject(e.getResponseBodyAsString());		
		JSONObject subjsonBody = jsonBody.getJSONObject("Error");
		amazonS3Exception.setErrorCode(subjsonBody.getString("Code"));
		amazonS3Exception.setErrorMessage(subjsonBody.getString("Message"));
		return amazonS3Exception;
	}

	public static void createCSAndCL(Sequoiadb sdb, String csName, String[] clNames) {
		if (sdb.isCollectionSpaceExist(csName)) {
			sdb.dropCollectionSpace(csName);
		}
		CollectionSpace cs = sdb.createCollectionSpace(csName);
		for (int i = 0; i < clNames.length; i++) {
			cs.createCollection(clNames[i]);
		}
	}
}
