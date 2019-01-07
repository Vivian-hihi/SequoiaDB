package com.sequoias3.object;

import java.util.ArrayList;
import java.util.List;

import org.testng.Assert;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import com.amazonaws.services.s3.AmazonS3;
import com.amazonaws.services.s3.model.GetObjectRequest;
import com.amazonaws.services.s3.model.ObjectMetadata;
import com.amazonaws.services.s3.model.S3Object;
import com.sequoiadb.exception.BaseException;
import com.sequoias3.testcommon.CommLib;
import com.sequoias3.testcommon.S3TestBase;
import com.sequoias3.testcommon.S3ThreadBase;
import com.sequoias3.testcommon.TestTools;
import com.sequoias3.testcommon.s3utils.ObjectUtils;
import com.sequoias3.testcommon.s3utils.UserUtils;

/**
 * test content: 禁用版本控制，并发获取不同版本的同一对象 
 * testlink-case: seqDB-16511
 * @author wangkexin
 * @Date 2019.01.04
 * @version 1.00
 */
public class GetSameObjectWithoutVersion16511 extends S3TestBase {
	private boolean runSuccess = false;	
	private String userName = "user16511";
	private String bucketName = "bucket16511";
	private String keyName = "key16511";
	private String roleName = "normal";
	private String content = "testContent16511";
	private List<String> etagList = new ArrayList<>();
	private String[] acessKeys = null;
	private AmazonS3 s3Client = null;
	
	@BeforeClass
	private void setUp() throws Exception {	
		CommLib.clearUser(userName);
		acessKeys = UserUtils.createUser(userName, roleName);
		s3Client = CommLib.buildS3Client(acessKeys[0], acessKeys[1]);	
		s3Client.createBucket(bucketName);
		CommLib.setBucketVersioning(s3Client, bucketName, "Enabled");
		
		//put three versions of the object
		for(int i = 0 ; i < 3 ; i++){
			String currentContent = content + ObjectUtils.getRandomString(i);
			s3Client.putObject(bucketName, keyName, currentContent);
			etagList.add(TestTools.getMD5(currentContent.getBytes()));
		}
		CommLib.setBucketVersioning(s3Client, bucketName, "Suspended");
	}
	
	@Test
	public void testGetObject() throws Exception {
		//Getting different versions of objects
		List<GetDifferentObjectThread> getDiffVerObjects = new ArrayList<>();	
		for( int i = 0; i < 3 ; i++){
			getDiffVerObjects.add( new GetDifferentObjectThread(String.valueOf(i)));
		}
		
		for( GetDifferentObjectThread getDiffVerObject : getDiffVerObjects ){
			getDiffVerObject.start();
		}	
		
		for( GetDifferentObjectThread getDiffVerObject : getDiffVerObjects ){
			Assert.assertTrue( getDiffVerObject.isSuccess(), getDiffVerObject.getErrorMsg());
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
			if( s3Client != null ){
				s3Client.shutdown();
			}
		}
	}
	
	private class GetDifferentObjectThread extends S3ThreadBase{
		String versionid;
		public GetDifferentObjectThread(String versionid){
			this.versionid = versionid;
		}
		@Override
		public void exec() throws Exception {
			AmazonS3 s3Client = CommLib.buildS3Client(acessKeys[0], acessKeys[1]);	
			try{
				S3Object object = s3Client.getObject(new GetObjectRequest(bucketName, keyName, versionid));
				ObjectMetadata metadata = object.getObjectMetadata();
				Assert.assertEquals(metadata.getETag(), etagList.get(Integer.parseInt(versionid)),"md5 is wrong!");
				Assert.assertEquals(metadata.getVersionId(), versionid);
			}finally{
				if (s3Client != null) {
					s3Client.shutdown();
				}
			}			
		}		
	}
}
