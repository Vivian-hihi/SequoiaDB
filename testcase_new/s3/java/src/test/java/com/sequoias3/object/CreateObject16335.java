package com.sequoias3.object;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import org.testng.Assert;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import com.amazonaws.AmazonServiceException;
import com.amazonaws.services.s3.AmazonS3;
import com.amazonaws.services.s3.model.CreateBucketRequest;
import com.amazonaws.services.s3.model.GetObjectRequest;
import com.amazonaws.services.s3.model.PutObjectResult;
import com.amazonaws.services.s3.model.S3Object;
import com.amazonaws.services.s3.model.S3ObjectInputStream;
import com.sequoiadb.exception.BaseException;
import com.sequoias3.testcommon.CommLib;
import com.sequoias3.testcommon.S3TestBase;
import com.sequoias3.testcommon.TestTools;

/**
 * test content: create object 
 * testlink-case: seqDB-16335 
 * @author wuyan
 * @Date 2018.11.6
 * @version 1.00
 */
public class CreateObject16335 extends S3TestBase{
	private boolean runSuccess = false;	
	String bucketName = "bucket16335";	
	String keyName = "object16335";	
	private AmazonS3 s3Client = null;
	private int fileSize = 1024 * 10;	
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
		//s3Client.deleteObject(bucketName, keyName);
		CommLib.clearBucket(s3Client, bucketName);	
	}

	@Test
	public void testCreateBucket() throws Exception {
		s3Client.createBucket(new CreateBucketRequest(bucketName));	
		//PutObjectResult result = s3Client.putObject(bucketName, keyName, "file");
		putObject(bucketName,  keyName);
		//File file =  new File(filePath);
		//PutObjectResult result = s3Client.putObject(bucketName, keyName, "file");
		GetObjectRequest request = new GetObjectRequest(bucketName, keyName);
		S3Object object = s3Client.getObject(request);
		System.out.println("---object153 ="+object.toString());	
		
		//checkPutObjectResult();
		runSuccess = true;
	}

	@AfterClass
	private void tearDown() {
		try {
			if (runSuccess) {
				// s3Client.deleteBucket(bucketName);				
			}
		} catch (BaseException e) {
			Assert.fail("clean up failed:" + e.getMessage());
		}
	}

		
	private void putObject(String bucketname, String keyName) throws IOException{        
        try {        	
            PutObjectResult result = s3Client.putObject(bucketname, keyName, "file123456");
        }catch (AmazonServiceException e){
            System.out.println("status code"+e.getStatusCode());
            System.out.println("status code:"+e.getErrorCode());
            System.out.println("status code:"+e.getErrorMessage());
        }catch (Exception e){
            System.out.println("status code:"+e.getMessage());
        }
    }
	private void checkPutObjectResult() throws Exception {
		// create one bucket,check the bucket name and owner name
		
		// down file
		String downloadPath = TestTools.LocalFile.initDownloadPath(localPath, TestTools.getMethodName(),
				Thread.currentThread().getId());
		S3Object object = s3Client.getObject(bucketName, keyName);
		S3ObjectInputStream s3is = object.getObjectContent();
		int index = keyName.lastIndexOf("/");
        String newKeyName = keyName.substring(index+1);

        FileOutputStream fos = new FileOutputStream(new File(downloadPath + newKeyName), true);
        byte[] read_buf = new byte[1024];
        int read_len = 0;
        while ((read_len = s3is.read(read_buf)) > -1) {
            fos.write(read_buf, 0, read_len);
        }
        s3is.close();
        fos.close();
        
        System.out.println("----getmd5="+TestTools.getMD5(downloadPath));
        System.out.println("----expmd5="+TestTools.getMD5(filePath));
	}

}
