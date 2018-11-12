package com.sequoias3.object;

import java.io.File;
import java.io.IOException;

import org.testng.Assert;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import com.amazonaws.services.s3.AmazonS3;
import com.amazonaws.services.s3.model.PutObjectResult;
import com.sequoiadb.exception.BaseException;
import com.sequoias3.testcommon.CommLib;
import com.sequoias3.testcommon.S3TestBase;
import com.sequoias3.testcommon.TestTools;

/**
 * test content: enabling bucket versioning , create object on the bucket
 * testlink-case: seqDB-16336 
 * @author wuyan
 * @Date 2018.11.6
 * @version 1.00
 */
public class CreateObjectWithVersion16336 extends S3TestBase{
	private boolean runSuccess = false;			
	private String keyName = "object16336";	
	private AmazonS3 s3Client = null;
	private int fileSize = 1024 * 5;	
	private File localPath = null;
	private String filePath = null;	

	@BeforeClass
	private void setUp() throws IOException {		
		localPath = new File(S3TestBase.workDir + File.separator + TestTools.getClassName());
		filePath = localPath + File.separator + "localFile_" + fileSize + ".txt";
		TestTools.LocalFile.removeFile(localPath);
		TestTools.LocalFile.createDir(localPath.toString());
		TestTools.LocalFile.createFile(filePath, fileSize);
		s3Client = CommLib.buildS3Client();	
		ObjectUtils.deleteObjectAllVersions( s3Client,S3TestBase.enableVerBucketName,keyName );	
	}

	@Test
	public void testCreateObject() throws Exception {				
		PutObjectResult result= s3Client.putObject(S3TestBase.enableVerBucketName, 
				keyName, new File(filePath));
		checkObjectAttributeInfo( result );		
		checkPutObjectResult( S3TestBase.enableVerBucketName );
		runSuccess = true;
	}

	@AfterClass
	private void tearDown() {
		try {
			if (runSuccess) {
				ObjectUtils.deleteObjectAllVersions( s3Client,S3TestBase.enableVerBucketName,keyName );				
			}
		} catch (BaseException e) {
			Assert.fail("clean up failed:" + e.getMessage());
		}
	}

		
	
	private void checkPutObjectResult(String bucketName) throws Exception {		
		//down file
		String downfileMd5 = ObjectUtils.getMd5OfObject(s3Client, localPath,
				bucketName, keyName);		
        Assert.assertEquals(downfileMd5, TestTools.getMD5(filePath));        
	}
	
	private void checkObjectAttributeInfo( PutObjectResult objAttrInfo) throws IOException{
		String expMd5 = TestTools.getMD5(filePath);
		Assert.assertEquals(objAttrInfo.getETag(), expMd5 );
		String isModify = null;
		Assert.assertEquals(objAttrInfo.getExpirationTimeRuleId(), isModify );
		
		//check the versionId,enable versiong the versionId is 0
		Assert.assertEquals(objAttrInfo.getVersionId(),"0");		
	}	
}
	
	
