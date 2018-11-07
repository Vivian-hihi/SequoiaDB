package com.sequoias3.bucket;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.OutputStreamWriter;
import java.io.Writer;
import java.util.List;
import org.testng.Assert;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import com.amazonaws.AmazonServiceException;
import com.amazonaws.auth.AWSCredentials;
import com.amazonaws.auth.AWSStaticCredentialsProvider;
import com.amazonaws.auth.BasicAWSCredentials;
import com.amazonaws.client.builder.AwsClientBuilder;
import com.amazonaws.services.s3.AmazonS3;
import com.amazonaws.services.s3.AmazonS3ClientBuilder;
import com.amazonaws.services.s3.model.Bucket;
import com.amazonaws.services.s3.model.BucketAccelerateConfiguration;
import com.amazonaws.services.s3.model.BucketVersioningConfiguration;
import com.amazonaws.services.s3.model.CreateBucketRequest;
import com.amazonaws.services.s3.model.DeleteObjectRequest;
import com.amazonaws.services.s3.model.GetBucketAccelerateConfigurationRequest;
import com.amazonaws.services.s3.model.GetObjectRequest;
import com.amazonaws.services.s3.model.ListObjectsV2Request;
import com.amazonaws.services.s3.model.ListObjectsV2Result;
import com.amazonaws.services.s3.model.ListVersionsRequest;
import com.amazonaws.services.s3.model.Owner;
import com.amazonaws.services.s3.model.PutObjectResult;
import com.amazonaws.services.s3.model.S3Object;
import com.amazonaws.services.s3.model.SetBucketAccelerateConfigurationRequest;
import com.amazonaws.services.s3.model.SetBucketVersioningConfigurationRequest;
import com.amazonaws.services.s3.model.VersionListing;
import com.sequoiadb.exception.BaseException;
import com.sequoias3.testcommon.CommLib;
import com.sequoias3.testcommon.S3TestBase;

/**
 * test content: create bucket 
 * testlink-case: seqDB-15901  
 * @author wuyan
 * @Date 2018.09.28
 * @version 1.00
 */
public class CreateBucketVersion extends S3TestBase{
	private boolean runSuccess = false;	
	private String bucketName = "bucketversion15901";
	private String keyName = "testpicture";
	private String keyName1 = "/app/testpicture2.jpg";
	private AmazonS3 s3Client = null;

	@BeforeClass
	private void setUp() {	
//		BucketVersioningConfiguration configuration =
//				new BucketVersioningConfiguration().withStatus("Enabled");
//		SetBucketVersioningConfigurationRequest setBucketVersioningConfigurationRequest =
//				new SetBucketVersioningConfigurationRequest(bucketName, configuration);
		s3Client = CommLib.buildS3Client();
		/*s3Client.setBucketVersioningConfiguration(setBucketVersioningConfigurationRequest);*/
		
//		putObject(, keyName);
//		DeleteObjectRequest doRequest= new DeleteObjectRequest(bucketName, keyName);
//		s3Client.deleteObject(doRequest);
		
		s3Client.deleteObject(bucketName, keyName);
		CommLib.clearBucket(s3Client, bucketName);	
		//CommLib.clearBuckets(s3Client);
	}

	@Test
	public void testCreateBucket() throws IOException {
		s3Client.createBucket(new CreateBucketRequest(bucketName));	
		
		putObject(bucketName,  keyName);
		//putObject(bucketName,  keyName1);
		//checkCreateBucketResult();
		GetObjectRequest request = new GetObjectRequest(bucketName, keyName);
		S3Object object = s3Client.getObject(request);
		System.out.println("---object ="+object.toString());	
		
		//list
		ListObjectsV2Request list = new ListObjectsV2Request()
				.withBucketName(bucketName)
				.withEncodingType("url");
		list.withPrefix("test");
		list.withStartAfter("c");
		ListObjectsV2Result request2 = s3Client.listObjectsV2(list);
		
		System.out.println("---list--=="+request2.getKeyCount());
		
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

		
	private void checkCreateBucketResult() {
		// create one bucket,check the bucket name and owner name
		List<Bucket> buckets = s3Client.listBuckets();
		boolean findBucketFlag = false;
		for ( int i = 0; i < buckets.size(); i++ ){
			String actBucketName = buckets.get(i).getName();
			//get the create bucket,then check the bucket name and owner
			if ( actBucketName.equals(bucketName)){
				Owner actOwner = buckets.get(i).getOwner();
				Assert.assertEquals(actOwner.getDisplayName(), S3TestBase.s3UserName);
				findBucketFlag = true;
				break;				
			}
		}				
		Assert.assertTrue(findBucketFlag, " the bucket must be exist!");		
	}
	
	private File createSampleFile() throws IOException {
        File file = File.createTempFile("aws-java-sdk-", ".txt");       
        file.deleteOnExit();

        Writer writer = new OutputStreamWriter(new FileOutputStream(file));
        writer.write("abcdefghijklmnopqrstuvwxyz\n");
        writer.write("01234567890112345678901234\n");
        writer.write("!@#$%^&*()-=[]{};':',.<>/?\n");
        writer.write("01234567890112345678901234\n");
        writer.write("abcdefghijklmnopqrstuvwxyz\n");
        writer.close();
        return file;
    }
	
	private void putObject(String bucketname, String keyName) throws IOException{
        File file = createSampleFile();
        try {        	
            PutObjectResult result = s3Client.putObject(bucketname, keyName, "file");
        }catch (AmazonServiceException e){
            System.out.println("status code"+e.getStatusCode());
            System.out.println("status code:"+e.getErrorCode());
            System.out.println("status code:"+e.getErrorMessage());
        }catch (Exception e){
            System.out.println("status code:"+e.getMessage());
        }
    }
}
