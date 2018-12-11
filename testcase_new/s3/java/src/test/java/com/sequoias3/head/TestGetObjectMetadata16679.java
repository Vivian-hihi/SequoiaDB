package com.sequoias3.head;

import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.Calendar;
import java.util.Date;
import java.util.Locale;

import org.apache.http.client.methods.CloseableHttpResponse;
import org.apache.http.client.methods.HttpHead;
import org.apache.http.impl.client.CloseableHttpClient;
import org.testng.Assert;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import com.amazonaws.services.s3.AmazonS3;
import com.amazonaws.services.s3.model.PutObjectResult;
import com.sequoiadb.exception.BaseException;
import com.sequoias3.testcommon.CommLib;
import com.sequoias3.testcommon.RestClient;
import com.sequoias3.testcommon.S3TestBase;
import com.sequoias3.testcommon.s3utils.UserUtils;

/**
 * test content:  匹配if-match条件，不带versionId查询对象
 * testlink-case: seqDB-16679 
 * @author wangkexin
 * @Date 2018.12.07
 * @version 1.00
 */

public class TestGetObjectMetadata16679  extends S3TestBase{
	private boolean runSuccess = false;
	private String bucketName = "bucket16679";
	private String userName = "user16679";
	private String roleName = "normal";
	private String keyName = "key16679";
	private String content = "content16679";
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
		PutObjectResult resultV1 =  s3Client.putObject(bucketName, keyName, content+"v1");
		
		Date date = new Date();
		PutObjectResult resultV2 = s3Client.putObject(bucketName, keyName, content+"v2");
		Calendar expLastModified[] = getGMTDateRange(date);
		String expVersionId = resultV2.getVersionId();
		String etagV1 = resultV1.getETag();
		String etagV2 = resultV2.getETag();
		
		HttpHead request = new HttpHead(S3TestBase.s3ClientUrl + "/s3/"+bucketName+"/"+keyName);
	    request.setHeader("Authorization", "Credential="+accessKeys[0]);
	    request.setHeader("If-Match", etagV1);
	    client = RestClient.createHttpClient();
	    try{
	    	RestClient.sendRequest(client, request);
	    	Assert.fail("get object v1 with etag should be fail!");
	    } catch(Exception e){
	    	Assert.assertNotEquals(e.getMessage().indexOf("errcode=412"), -1);
	    }
	    
	    request.setHeader("If-Match", etagV2);
	    client = RestClient.createHttpClient();
    	CloseableHttpResponse resp = RestClient.sendRequest(client, request);
    	Assert.assertEquals(resp.getFirstHeader("ETag").getValue(), etagV2);
    	Assert.assertEquals(resp.getFirstHeader("x-amz-version-id").getValue(), expVersionId);
    	Assert.assertEquals(resp.getFirstHeader("Content-Length").getValue(), String.valueOf((content+"v2").length()));
    	
    	SimpleDateFormat sdf = new SimpleDateFormat("EEE, dd MMM yyyy HH:mm:ss z",Locale.US);
		Date actDate = sdf.parse(resp.getFirstHeader("Last-Modified").getValue());
		Calendar actCalendar = Calendar.getInstance();
		actCalendar.setTime(actDate);
		
		if(actCalendar.before(expLastModified[0]) || actCalendar.after(expLastModified[1])){
			Assert.fail("versions' lastModified is wrong , "
					+ "exp actual time is in :[" + expLastModified[0].getTime().toString()+ ", " + expLastModified[1].getTime().toString() + "] , "
							+ "but actual time is : " + actCalendar.getTime().toString());
		}
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
	
	private Calendar[] getGMTDateRange(Date date) throws ParseException{
		Calendar calRange[] = new Calendar[2];
		SimpleDateFormat sdf = new SimpleDateFormat("EEE, dd MMM yyyy HH:mm:ss z",Locale.US);
		String rfc1123 = sdf.format(date);
		
		Date formattedDate = sdf.parse(rfc1123);
		//设置误差为半小时内
		calRange[0] = Calendar.getInstance();
		calRange[0].setTime(formattedDate);
		calRange[0].set(Calendar.MINUTE, calRange[0].get(Calendar.MINUTE)- 15 );
		calRange[1] = Calendar.getInstance();
		calRange[1].setTime(formattedDate);
		calRange[1].set(Calendar.MINUTE, calRange[1].get(Calendar.MINUTE)+ 15 );
		return calRange;
	}
}
