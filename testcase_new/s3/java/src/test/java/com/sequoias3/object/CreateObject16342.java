package com.sequoias3.object;

import com.amazonaws.services.s3.AmazonS3;
import com.amazonaws.services.s3.model.CreateBucketRequest;
import com.amazonaws.services.s3.model.ObjectMetadata;
import com.amazonaws.services.s3.model.S3Object;
import com.sequoias3.testcommon.CommLib;
import com.sequoias3.testcommon.S3TestBase;
import com.sequoias3.testcommon.TestTools;
import com.sequoias3.testcommon.s3utils.ObjectUtils;
import org.testng.Assert;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import java.io.File;
import java.io.IOException;
import java.util.Date;

/**
 * test content: 禁用版本控制，增加同名对象 
 * testlink-case: seqDB-16342
 * @author wangkexin
 * @Date 2018.11.8
 * @version 1.00
 */
public class CreateObject16342 extends S3TestBase {
	//TODO:1、建议申明私有变量，bucket和key只对该用例使用
	String bucketName = "bucket16342";
	String keyName = "/aa/bb/object16342.png";
	private AmazonS3 s3Client = null;
	private File localPath = null;
	private String expContent = null;
	private String expCreateTime = null;
	private boolean runSuccess = false;

	@BeforeClass
	private void setUp() throws IOException {
		localPath = new File(S3TestBase.workDir + File.separator + TestTools.getClassName());
		s3Client = CommLib.buildS3Client();
		//create bucket and set bucket version status
		s3Client.createBucket(new CreateBucketRequest(bucketName));
		CommLib.setBucketVersioning(s3Client, bucketName, "Suspended");
	}

	@Test
	public void testPutSameNameObject() throws Exception {
		//put the same object with the same content and different contents.
		s3Client.putObject(bucketName, keyName, "same_file16342");
		Thread.sleep(1000);//TODO:2、为啥要sleep？
		s3Client.putObject(bucketName, keyName, "same_file16342");
		Thread.sleep(1000);
		expContent = "different_file16342";
		s3Client.putObject(bucketName, keyName, expContent);
		expCreateTime = new Date().toString();//TODO:3、获取当前时间为创建对象时间不准确，自动化可以获取最后修改时间不超过当前时间，手工检查时间准确性
		
		//check result
		checkPutObjectResult();
		runSuccess = true;
	}

	@AfterClass
	private void tearDown() {
		if (runSuccess) {
			CommLib.deleteAllObjectVersions(s3Client, bucketName);
			s3Client.deleteBucket(bucketName);
		}
	}

	private void checkPutObjectResult() throws Exception {
		S3Object object = s3Client.getObject(bucketName, keyName);
		
		//check object update by create time
		ObjectMetadata metadata = object.getObjectMetadata();
		String actCreateTime = metadata.getLastModified().toString();
		Assert.assertEquals(actCreateTime, expCreateTime,"create time is different!");
				
		// check object content by md5
		String actMd5 = ObjectUtils.getMd5OfObject(s3Client, localPath, bucketName, keyName);
		String expMd5 = TestTools.getMD5(expContent.getBytes());
		Assert.assertEquals(actMd5, expMd5,"md5 is different!");
	}
}
