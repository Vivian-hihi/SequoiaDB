package com.sequoias3.object;

import com.amazonaws.services.s3.AmazonS3;
import com.amazonaws.services.s3.model.CreateBucketRequest;
import com.amazonaws.services.s3.model.PutObjectResult;
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

/**
 * test content: 开启版本控制，增加多个同名对象
 * testlink-case: seqDB-16345
 * @author wangkexin
 * @Date 2018.11.12
 * @version 1.00
 */
public class CreateObject16345 extends S3TestBase {
	private boolean runSuccess = false;
	String bucketName = "bucket16345";//TODO:1、建议申明私有变量，bucket和key只对该用例使用
	String keyName = "object16345";
	private AmazonS3 s3Client = null;
	private File localPath = null;
	private String expContent = "object_file16345";

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
		PutObjectResult putObjResult = null;
		int randomNum = (int)(Math.random()* 200);
		for(int i  = 0 ; i < 200 ; i++){
			putObjResult = s3Client.putObject(bucketName, keyName, expContent+"." + i);
			if(i == randomNum){
				String currentExpContent = expContent+"." + i;
				String currVersionID = putObjResult.getVersionId();
				checkPutObjectResultRandom(currVersionID,currentExpContent);
			}
		}
		//TODO:2、只是检查当前对象元数据信息，遗漏其它版本元数据信息的检测。
		String currentContent = expContent+".199";
		String version = putObjResult.getVersionId();
		String eTag = putObjResult.getETag();
		long size = currentContent.getBytes().length;
		checkCurrentObjectResult(version,eTag,size);
		runSuccess = true;
	}

	@AfterClass
	private void tearDown() {
		if (runSuccess) {
			CommLib.deleteAllObjectVersions(s3Client, bucketName);
			s3Client.deleteBucket(bucketName);
		}
	}
	
	private void checkPutObjectResultRandom(String currVersionID,String currExpContent) throws Exception {
		// check object content by md5
		String actMd5 = ObjectUtils.getMd5OfObject(s3Client, localPath, bucketName, keyName, currVersionID);
		String expMd5 = TestTools.getMD5(currExpContent.getBytes());
		Assert.assertEquals(actMd5, expMd5,"md5 is different!");
	}

	private void checkCurrentObjectResult(String versionid, String etag, long size) throws Exception {
		S3Object object = s3Client.getObject(bucketName, keyName);
		String actVersionId = object.getObjectMetadata().getVersionId();
		Assert.assertEquals(actVersionId, versionid,"versionID is different!");
		
		String actETag = object.getObjectMetadata().getETag();
		Assert.assertEquals(actETag, etag,"ETag is different!");
		
		long actSize = object.getObjectMetadata().getContentLength();
		Assert.assertEquals(actSize, size,"size is different!");
	}
}
