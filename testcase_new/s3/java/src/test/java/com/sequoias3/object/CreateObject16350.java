package com.sequoias3.object;

import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.util.Base64;
import java.util.Base64.Encoder;

import org.testng.Assert;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import com.amazonaws.services.s3.AmazonS3;
import com.amazonaws.services.s3.model.AmazonS3Exception;
import com.amazonaws.services.s3.model.CreateBucketRequest;
import com.amazonaws.services.s3.model.ObjectMetadata;
import com.sequoias3.testcommon.CommLib;
import com.sequoias3.testcommon.S3TestBase;
import com.sequoias3.testcommon.TestTools;

/**
 * test content: 增加对象，携带md5值
 * testlink-case: seqDB-16350
 * @author wangkexin
 * @Date 2018.11.13
 * @version 1.00
 */
public class CreateObject16350 extends S3TestBase {
	private boolean runSuccess = false;
	String bucketName = "bucket16350";
	String keyName = "/aa/bb/object16350";
	private AmazonS3 s3Client = null;
	private byte[] beforeMd5 = null;
	private String wrongMd5 = null;
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
		beforeMd5 = getMD5ByteArray(filePath);
		
		s3Client = CommLib.buildS3Client();
		//create bucket
		s3Client.createBucket(new CreateBucketRequest(bucketName));
	}

	@Test
	public void testPutObject() throws Exception {
		//put object with correct md5 value.
		File f = new File(filePath);
		InputStream input = new FileInputStream(f);
		ObjectMetadata metadata = new ObjectMetadata();
		metadata.setContentMD5(getBase64(beforeMd5));
		s3Client.putObject(bucketName,keyName, input, metadata);
		input.close();
		checkPutObjectResult();
		
		//put object with wrong md5 value.
		wrongMd5 = TestTools.getMD5(filePath);
		metadata.setContentMD5(wrongMd5);
		try(InputStream input2 = new FileInputStream(f)){
			s3Client.putObject(bucketName,keyName, input2, metadata);
			Assert.fail("exp fail but found success");
		}catch(AmazonS3Exception e){	
			Assert.assertEquals(e.getErrorCode(), "BadDigest");
		}
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
		//down file
		String downfileMd5 = ObjectUtils.getMd5OfObject(s3Client, localPath,bucketName, keyName);		
        Assert.assertEquals(downfileMd5, TestTools.getMD5(filePath));        
	}
	
	private String getBase64(byte[] b){
		String result = null;
		Encoder encoder = Base64.getEncoder();
		result = encoder.encodeToString(b);
		return result;
	}
	
	private static byte[] getMD5ByteArray(String pathName) throws IOException {
		File file = new File(pathName);
		byte[] value = null;
		try (FileInputStream fis = new FileInputStream(file)) {
			byte[] buffer = new byte[(int) file.length()];
			fis.read(buffer);
			try {
				MessageDigest md5 = MessageDigest.getInstance("MD5");
				md5.update((byte[]) buffer);
				value = md5.digest();
			} catch (NoSuchAlgorithmException e) {
				e.printStackTrace();
				throw new RuntimeException("fail to get md5base64!" + e.getMessage());
			}
			return value;
		}
	}
}
