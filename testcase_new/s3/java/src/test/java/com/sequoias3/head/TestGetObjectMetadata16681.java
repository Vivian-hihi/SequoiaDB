package com.sequoias3.head;

import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.Calendar;
import java.util.Date;
import java.util.Locale;

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
import com.sequoias3.testcommon.S3TestBase;
import com.sequoias3.testcommon.s3utils.UserUtils;

/**
 * test content:  指定versionId查询对象 
 * testlink-case: seqDB-16681 
 * @author wangkexin
 * @Date 2018.12.07
 * @version 1.00
 */

public class TestGetObjectMetadata16681  extends S3TestBase{
	private boolean runSuccess = false;
	private String bucketName = "bucket16681";
	private String userName = "user16681";
	private String roleName = "normal";
	private String keyName = "key16681";
	private String content = "content16681";
	private AmazonS3 s3Client = null;

	@BeforeClass
	private void setUp() throws Exception {
		CommLib.clearUser(userName);
		String[] accessKeys = UserUtils.createUser(userName, roleName);
		s3Client = CommLib.buildS3Client(accessKeys[0], accessKeys[1]);
	}

	@Test
	private void testGetObjectMetadata() throws Exception {
		s3Client.createBucket(bucketName);
		CommLib.setBucketVersioning(s3Client, bucketName, "Enabled");
		s3Client.putObject(bucketName, keyName, content);
		s3Client.putObject(bucketName, keyName, content);
		PutObjectResult result = s3Client.putObject(bucketName, keyName, content);
		String expEtag = result.getETag();
		String expVersionid = result.getVersionId();
			
		ObjectMetadata metadata = s3Client.getObjectMetadata(new GetObjectMetadataRequest(bucketName, keyName, expVersionid));
		Calendar expLastModified[] = getGMTDateRange(new Date());
		
		Assert.assertEquals(metadata.getETag(), expEtag, "etag is wrong!");
		Assert.assertEquals(metadata.getContentLength(), (long)content.length(), "size is wrong!");
		Assert.assertEquals(metadata.getVersionId(), expVersionid);
		
		Date actDate = metadata.getLastModified();
		Calendar actCalendar = Calendar.getInstance();
		actCalendar.setTime(actDate);
		
		if(actCalendar.before(expLastModified[0]) || actCalendar.after(expLastModified[1])){
			Assert.fail("get' lastModified is wrong , "
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
