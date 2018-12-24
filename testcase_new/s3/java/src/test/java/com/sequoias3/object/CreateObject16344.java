package com.sequoias3.object;

import java.io.IOException;
import java.util.ArrayList;
import java.util.List;

import org.testng.Assert;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import com.amazonaws.services.s3.AmazonS3;
import com.amazonaws.services.s3.model.CreateBucketRequest;
import com.amazonaws.services.s3.model.PutObjectResult;
import com.amazonaws.services.s3.model.S3Object;
import com.sequoias3.testcommon.CommLib;
import com.sequoias3.testcommon.S3TestBase;
import com.sequoias3.testcommon.S3ThreadBase;

/**
 * test content: 开启版本控制增加对象，桶中增加/删除多个对象
 * testlink-case: seqDB-16344
 * @author wangkexin
 * @Date 2018.11.12
 * @version 1.00
 */
public class CreateObject16344 extends S3TestBase {
	private boolean runSuccess = false;
	String bucketName = "bucket16344";
	String keyName = "object16344";
	private AmazonS3 s3Client = null;
	private String expContent = "object_file16344";
	private String expETag = null;
	private long expSize = 0;
	private int defaultNums = 10;

	@BeforeClass
	private void setUp() throws IOException {
		s3Client = CommLib.buildS3Client();
		//create bucket and set bucket version status
		s3Client.createBucket(new CreateBucketRequest(bucketName));
		CommLib.setBucketVersioning(s3Client, bucketName, "Enabled");
	}

	@Test
	public void testPutObject() throws Exception {
		List<PutObjectThread> putObjects = new ArrayList<>(20);		
		
		for( int i = 0; i < defaultNums ; i++){
			String subKeyName = keyName + "." + i;							
			putObjects.add( new PutObjectThread(subKeyName));
		}
		
		for( PutObjectThread putObject : putObjects ){
			putObject.start();
		}	
		
		for( PutObjectThread putObject : putObjects ){
			Assert.assertTrue( putObject.isSuccess(), putObject.getErrorMsg());
		}		
		//TODO:3、检测结果遗漏比较上传对象元数据信息，建议将key和对应etag写到队列中，获取结果时分别比较。另外对象内容不要写一样的，如果内容被覆盖无法验证
		String randomKeyName = keyName+"."+(int)(Math.random()*10)+"."+(int)(Math.random()*1000);
		checkCurrentObjectResult(randomKeyName);
		runSuccess = true;
	}

	@AfterClass
	private void tearDown() {
		if (runSuccess) {
			CommLib.deleteAllObjectVersions(s3Client, bucketName);
			s3Client.deleteBucket(bucketName);
		}
	}
	
	private class PutObjectThread extends S3ThreadBase{
		String keyName = null;		
		public PutObjectThread ( String keyName ){
			this.keyName = keyName;			
		}
		@Override
		public void exec() throws Exception {
			AmazonS3 s3Client = CommLib.buildS3Client();
			PutObjectResult putObjResult = null;
			try{
				for(int i  = 0 ; i < 1000 ; i++){
					putObjResult = s3Client.putObject(bucketName, keyName+ "." + i,expContent);
				}
				expETag = putObjResult.getETag();//TODO:2、这里的etag和size为全局变量，实际检测只是测试了最后一次的结果
				expSize = expContent.getBytes().length;
			}finally{
				if (s3Client != null) {
					s3Client.shutdown();
				}
			}			
		}		
	}
	
	private void checkCurrentObjectResult(String keyName) throws Exception {
		S3Object object = s3Client.getObject(bucketName, keyName);
		String actETag = object.getObjectMetadata().getETag();
		Assert.assertEquals(expETag, actETag,"ETag is different!");
		
		long actSize = object.getObjectMetadata().getContentLength();
		Assert.assertEquals(expSize, actSize,"size is different!");
	}
}
