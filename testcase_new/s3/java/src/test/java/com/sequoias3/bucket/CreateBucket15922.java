package com.sequoias3.bucket;

import java.util.ArrayList;
import java.util.List;

import org.testng.Assert;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import com.amazonaws.auth.AWSCredentials;
import com.amazonaws.auth.AWSStaticCredentialsProvider;
import com.amazonaws.auth.BasicAWSCredentials;
import com.amazonaws.client.builder.AwsClientBuilder;
import com.amazonaws.services.s3.AmazonS3;
import com.amazonaws.services.s3.AmazonS3ClientBuilder;
import com.amazonaws.services.s3.model.Bucket;
import com.sequoiadb.exception.BaseException;
import com.sequoias3.testcommon.CommLib;
import com.sequoias3.testcommon.S3TestBase;
import com.sequoias3.testcommon.S3ThreadBase;


/**
 * @Description seqDB-15922:concurrent create bucket by different s3 client * 
 * @author wuyan
 * @Date 2018.10.10
 * @version 1.00
 */
public class CreateBucket15922 extends S3TestBase{
	private boolean runSuccess = false;	
	private String bucketName = "bucket15922";	
	private static String clientRegion = "us-east-1";
	private final int defaultNums = 100;
	private AmazonS3 s3Client1 = null;
	private AmazonS3 s3Client2 = null;
	private String url1 = "http://192.168.31.1:8080";
	private String url2 = "http://192.168.31.2:8020";

	@BeforeClass(enabled=false)
	private void setUp() throws Exception {			
		s3Client1 = buildS3Client( url1 );	
		s3Client2 = buildS3Client( url2 );
		CommLib.clearBuckets(s3Client1);	
		CommLib.clearBuckets(s3Client2);	
	}

	@Test(enabled=false)
	public void testCreateBucket() throws Exception {
		List<CreateBucketThread> createBuckets = new ArrayList<>(20);		
		GetBucketThread getBuckets1 = new GetBucketThread(url1);
		GetBucketThread getBuckets2 = new GetBucketThread(url2);
		for( int i = 0; i < defaultNums ; i++){
			if ( i % 2 == 0){				
				String subBucketName = bucketName + "aa." + i;							
				createBuckets.add( new CreateBucketThread(subBucketName, url1));	
			}else{
				String subBucketName = bucketName + "bb." + i;							
				createBuckets.add( new CreateBucketThread(subBucketName, url2));	
			}
					
		}
		
		for( CreateBucketThread createBucket : createBuckets ){
			createBucket.start();
		}	
		getBuckets1.start();
		getBuckets2.start();
		
		for( CreateBucketThread createBucket : createBuckets ){
			Assert.assertTrue( createBucket.isSuccess(), createBucket.getErrorMsg());
		}		
		
		//checkDeleteBucketResult(s3Client, existBuckets);
		runSuccess = true;			
	}

	@AfterClass(enabled=false)
	private void tearDown() throws Exception {
		try {
			if (runSuccess) {
				//s3Client.deleteBucket(bucketName);
				CommLib.clearBuckets(s3Client1);									
			}
		} catch (BaseException e) {
			Assert.fail("clean up failed:" + e.getMessage());
		} finally {
			if( s3Client1 != null ){
				s3Client1.shutdown();
			}
			if( s3Client2 != null ){
				s3Client2.shutdown();
			}
			
		}
	}
	
	private class GetBucketThread extends S3ThreadBase{		
		String url;
		public GetBucketThread ( String url ){				
			this.url = url;
		}
		
		@Override
		public void exec() throws Exception {
			AmazonS3 s3Client = buildS3Client(url);
			try{
				System.out.println("---%%%%%begin to list");
				List<Bucket> buckets = s3Client.listBuckets();
				System.out.println("---%%%%%end to list");
				//only test list bucket success, the buckets not 0
				Assert.assertNotEquals(buckets.size(), 0);
			}finally{
				if (s3Client != null) {
					s3Client.shutdown();
				}
			}			
		}		
	}
	
	private class CreateBucketThread extends S3ThreadBase{
		String bucketName;	
		String url;
		public CreateBucketThread ( String bucketName,String url ){
			this.bucketName = bucketName;	
			this.url = url;
		}

		@Override
		public void exec() throws Exception {
			AmazonS3 s3Client = buildS3Client(url);
			try{
				System.out.println("$$$$$$$---begin to create:"+bucketName);
				s3Client.createBucket(bucketName);
				System.out.println("$$$$$$$---end to create:"+bucketName);
			}finally{
				if (s3Client != null) {
					s3Client.shutdown();
				}
			}			
		}		
	}	
	
	public static AmazonS3 buildS3Client(String s3ClientUrl){
		AmazonS3 s3Client = null;
		AWSCredentials credentials = new BasicAWSCredentials("ABCDEFGHIJKLMNOPQRST",
				"abcdefghijklmnopqrstuvwxyz0123456789ABCD");
		AwsClientBuilder.EndpointConfiguration endpointConfiguration = new AwsClientBuilder.EndpointConfiguration(
				s3ClientUrl, clientRegion);
		s3Client = AmazonS3ClientBuilder.standard()
				.withEndpointConfiguration(endpointConfiguration)
				.withCredentials(new AWSStaticCredentialsProvider(credentials)).build();
		return s3Client;		
	}
	
}
