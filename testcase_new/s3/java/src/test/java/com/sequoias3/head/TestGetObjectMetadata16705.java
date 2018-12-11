package com.sequoias3.head;

import java.text.SimpleDateFormat;
import java.util.Calendar;
import java.util.Date;
import java.util.GregorianCalendar;
import java.util.Locale;
import java.util.TimeZone;

import org.apache.http.client.methods.CloseableHttpResponse;
import org.apache.http.client.methods.HttpHead;
import org.apache.http.impl.client.CloseableHttpClient;
import org.testng.Assert;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import com.amazonaws.services.s3.AmazonS3;
import com.amazonaws.services.s3.model.GetObjectMetadataRequest;
import com.amazonaws.services.s3.model.ObjectMetadata;
import com.amazonaws.services.s3.model.PutObjectResult;
import com.sequoiadb.exception.BaseException;
import com.sequoias3.testcommon.CommLib;
import com.sequoias3.testcommon.RestClient;
import com.sequoias3.testcommon.S3TestBase;
import com.sequoias3.testcommon.s3utils.UserUtils;

/**
 * test content:  指定ifNoneMatch和ifModifiedSince条件查询对象，不匹配ifModifiedSince
 * testlink-case: seqDB-16705 
 * @author wangkexin
 * @Date 2018.12.11
 * @version 1.00
 */

public class TestGetObjectMetadata16705  extends S3TestBase{
	private boolean runSuccess = false;
	private String bucketName = "bucket16705";
	private String userName = "user16705";
	private String roleName = "normal";
	private String keyName = "key16705";
	private String content = "content16705";
	private static CloseableHttpClient client;
	private String[] accessKeys = null;
	private AmazonS3 s3Client = null;

	@BeforeClass
	private void setUp() throws Exception {
		CommLib.clearUser(userName);
		accessKeys = UserUtils.createUser(userName, roleName);
		s3Client = CommLib.buildS3Client(accessKeys[0], accessKeys[1]);
	}

	@Test
	private void testGetObjectMetadata() throws Exception {
		s3Client.createBucket(bucketName);
		CommLib.setBucketVersioning(s3Client, bucketName, "Enabled");

		s3Client.putObject(bucketName, keyName, content+"v1");
		PutObjectResult result =  s3Client.putObject(bucketName, keyName, content+"v2");
		String historyEtag = result.getETag();
		
		PutObjectResult currResult =s3Client.putObject(bucketName, keyName, content+"v3");
		String currEtag = currResult.getETag();
		String currVersion = currResult.getVersionId();
		
		ObjectMetadata metadata  = s3Client.getObjectMetadata(new GetObjectMetadataRequest(bucketName, keyName, currVersion));
		Date actDate = metadata.getLastModified();
		
		//匹配If-None-Match，不匹配If-Modified-Since
		HttpHead request = new HttpHead(S3TestBase.s3ClientUrl + "/s3/"+bucketName+"/"+keyName);
	    request.setHeader("Authorization", "Credential="+accessKeys[0]);
	    request.setHeader("If-None-Match", historyEtag);
	    request.setHeader("If-Modified-Since", getModifiedGMTDate(actDate, 1));
	    
	    client = RestClient.createHttpClient();
	    CloseableHttpResponse resp = RestClient.sendRequest(client, request);
    	Assert.assertEquals(resp.getFirstHeader("ETag").getValue(), currEtag);
    	Assert.assertEquals(resp.getFirstHeader("x-amz-version-id").getValue(), currVersion);
    	runSuccess = true;
	}

	@AfterClass
	private void tearDown() throws Exception {
		try {
			if (runSuccess) {
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
	
	private String getModifiedGMTDate(Date date , int amount){
		Calendar calendar = new GregorianCalendar();
		calendar.setTime(date);
		//把日期往后增加，正数往后推，负数往前推
		calendar.add(Calendar.DATE, amount);
		date = calendar.getTime();
		return getGMTDate(date);
	}
	
	private String getGMTDate(Date date){
		SimpleDateFormat sdf = new SimpleDateFormat("EEE, dd MMM yyyy HH:mm:ss z",Locale.US);
		sdf.setTimeZone(TimeZone.getTimeZone("GMT"));
		String rfc1123 = sdf.format(date);
		return rfc1123;
	}
}
