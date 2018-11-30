package com.sequoias3.object;

import java.io.File;
import java.io.IOException;
import java.util.Date;

import org.testng.Assert;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import com.amazonaws.services.s3.AmazonS3;
import com.amazonaws.services.s3.model.CreateBucketRequest;
import com.amazonaws.services.s3.model.GetObjectRequest;
import com.amazonaws.services.s3.model.ObjectMetadata;
import com.amazonaws.services.s3.model.PutObjectResult;
import com.amazonaws.services.s3.model.S3Object;
import com.sequoias3.testcommon.CommLib;
import com.sequoias3.testcommon.S3TestBase;
import com.sequoias3.testcommon.TestTools;

/**
 * test content: 开启版本控制增加对象，禁用版本控制后上传同名对象 
 * testlink-case: seqDB-16343
 * @author wangkexin
 * @Date 2018.11.9
 * @version 1.00
 */
public class CreateObject16343 extends S3TestBase {
	private boolean runSuccess = false;
	String bucketName = "bucket16343";
	String keyName = "/aa/bb/object16343.png";
	private AmazonS3 s3Client = null;
	private File localPath = null;
	private String historyVersionId = null;
	private String firstTime_expContent = null;
	private String secondTime_expContent = null;
	private String expFirstCreateTime = null;
	private String expSecondCreateTime = null;

	@BeforeClass
	private void setUp() throws IOException {
		localPath = new File(S3TestBase.workDir + File.separator + TestTools.getClassName());
		s3Client = CommLib.buildS3Client();
		//create bucket and set bucket version status
		s3Client.createBucket(new CreateBucketRequest(bucketName));
		CommLib.setBucketVersioning(s3Client, bucketName, "Enabled");
	}

	@Test
	public void testPutObject() throws Exception {
		//put the same object with contents.
		firstTime_expContent = "first_time_file16343";
		PutObjectResult putObjResult = s3Client.putObject(bucketName, keyName,firstTime_expContent);
		expFirstCreateTime = new Date().toString();
		historyVersionId = putObjResult.getVersionId();
		
		Thread.sleep(1000);
		CommLib.setBucketVersioning(s3Client, bucketName, "Suspended");
		
		secondTime_expContent = "second_time_file16343";
		s3Client.putObject(bucketName, keyName,secondTime_expContent);
		expSecondCreateTime = new Date().toString();
		
		//check result
		checkCurrentObjectResult();
		checkHistoryObjectData();
		runSuccess = true;
	}

	@AfterClass
	private void tearDown() {
		if (runSuccess) {
			CommLib.deleteAllObjectVersions(s3Client, bucketName);
			s3Client.deleteBucket(bucketName);
		}
	}

	private void checkCurrentObjectResult() throws Exception {
		S3Object object = s3Client.getObject(bucketName, keyName);
		//check create time
		checkCreateTime(object,expSecondCreateTime);
		//check object content by md5
		String actMd5 = ObjectUtils.getMd5OfObject(s3Client, localPath, bucketName, keyName);
		String expMd5 = TestTools.getMD5(secondTime_expContent.getBytes());
		Assert.assertEquals(actMd5, expMd5,"The md5 value of the current version is different.");
	}
	
	private void checkHistoryObjectData() throws Exception{
		S3Object object = s3Client.getObject(new GetObjectRequest(bucketName, keyName, historyVersionId));
		checkCreateTime(object,expFirstCreateTime);
		String actMd5 = ObjectUtils.getMd5OfObject(s3Client, localPath, bucketName, keyName, historyVersionId);
		String expMd5 = TestTools.getMD5(firstTime_expContent.getBytes());
		Assert.assertEquals(actMd5, expMd5,"The md5 value of the previous version is different.");
	}
	
	private void checkCreateTime(S3Object object,String expCreateTime) throws Exception{
		//check object update by create time
		ObjectMetadata metadata = object.getObjectMetadata();
		String actCreateTime = metadata.getLastModified().toString();
		Assert.assertEquals(actCreateTime, expCreateTime,"create time is different!");
	}
}
