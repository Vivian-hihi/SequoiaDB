package com.sequoias3.region;

import java.util.ArrayList;
import java.util.List;

import org.json.JSONArray;
import org.json.JSONObject;
import org.json.XML;
import org.springframework.http.HttpMethod;
import org.springframework.http.ResponseEntity;
import org.springframework.web.client.HttpClientErrorException;
import org.testng.Assert;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import com.amazonaws.services.s3.model.AmazonS3Exception;
import com.sequoiadb.base.Sequoiadb;
import com.sequoias3.testcommon.S3TestBase;
import com.sequoias3.testcommon.TestRest;
import com.sequoias3.testcommon.s3utils.RegionUtils;
import com.sequoias3.user.UserCommDefind;

/**
 * test content: GetRegion接口参数校验
 * testlink-case: seqDB-17354
 * @author wangkexin
 * @Date 2019.01.24
 * @version 1.00
 */

public class TestGetRegion17354 extends S3TestBase{
	private String regionName = "Beijing17354";
	private String metaCSName = "metaCS17354";
	private String dataCSName = "dataCS17354";
	private String[] metaClNames = {"metaCL17354","metaHistoryCL17354"};
	private String[] dataClName = {"dataCL17354"};
	private static Sequoiadb sdb = null;
	private boolean runSuccess = false;

	@BeforeClass
	private void setUp() throws Exception {
		sdb = new Sequoiadb(S3TestBase.coordUrl, "", "");
		RegionUtils.createCSAndCL(metaCSName, metaClNames);
		RegionUtils.createCSAndCL(dataCSName, dataClName);
		
		if(RegionUtils.headRegion(regionName)){
			RegionUtils.deleteRegion(regionName);
		}
		Region region = new Region();
        region.withName(regionName)
        .withMetaLocation(metaCSName + "." + metaClNames[0])
	    .withMetaHisLocation(metaCSName + "." + metaClNames[1])
	    .withDataLocation(dataCSName + "." + dataClName[0]);
        RegionUtils.putRegion(region);
	}
	
	@Test
	public void testCreateRegion() throws Exception {
        //合法值
        GetRegionResult result = RegionUtils.getRegion(regionName);
        Region region = result.getRegion();
        //TODO:1、用例中没有特别要求，region名没有必要大写，这里就不需要再转换大小写
        Assert.assertEquals(region.getName(), regionName.toLowerCase());
    	Assert.assertEquals(region.getMetaLocation(), metaCSName + "." + metaClNames[0]);
		Assert.assertEquals(region.getMetaHisLocation(), metaCSName + "." + metaClNames[1]);
		Assert.assertEquals(region.getDataLocation(), dataCSName + "." + dataClName[0]);
    	
    	//非法值
		List<String> regionList = getRegion("");
		Assert.assertTrue(regionList.contains(regionName.toLowerCase()));
    	
		List<String> regionList2 = getRegion(new String());
		Assert.assertTrue(regionList2.contains(regionName.toLowerCase()));
		
		runSuccess = true;
	}
	
	@AfterClass
	private void tearDown() throws Exception {
		if (runSuccess) {
			RegionUtils.deleteRegion(regionName);
			sdb.dropCollectionSpace(dataCSName);
			sdb.dropCollectionSpace(metaCSName);
			sdb.close();
		}
	}
	
	private static List<String> getRegion(String regionName) throws Exception {
		TestRest rest = new TestRest();
		ResponseEntity<?> resp;
		List<String> listResult;
		try {
			resp = rest.setApi("/region" + regionName)
					.setRequestHeaders(UserCommDefind.authorization, S3TestBase.s3AccessKeyId)
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
	
	private static AmazonS3Exception httpToAmazon(HttpClientErrorException e) {
		AmazonS3Exception amazonS3Exception = new AmazonS3Exception(e.getMessage());
		amazonS3Exception.setStatusCode(e.getStatusCode().value());
		JSONObject jsonBody = XML.toJSONObject(e.getResponseBodyAsString());		
		JSONObject subjsonBody = jsonBody.getJSONObject("Error");
		amazonS3Exception.setErrorCode(subjsonBody.getString("Code"));
		amazonS3Exception.setErrorMessage(subjsonBody.getString("Message"));
		return amazonS3Exception;
	}
}
