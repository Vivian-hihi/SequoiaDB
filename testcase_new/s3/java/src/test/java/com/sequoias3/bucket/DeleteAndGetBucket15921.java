package com.sequoias3.bucket;

import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;
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
import com.amazonaws.services.s3.model.AmazonS3Exception;
import com.amazonaws.services.s3.model.Bucket;
import com.sequoias3.testcommon.RestClient;
import com.sequoias3.testcommon.S3TestBase;
import com.sequoias3.testcommon.S3ThreadBase;


/**
 * @Description seqDB-15921:concurrent delete bucket and get bucketlist
 * @author wuyan
 * @Date 2018.10.10
 * @version 1.00
 */
public class DeleteAndGetBucket15921 extends S3TestBase{
	private boolean runSuccess = false;	
	private String bucketName = "bucket15921";	
	private String clientRegion = "us-east-1";
	private final int defaultNums = 30;
	private String userName = "user15921";
	private String roleName = "normal";
	private AmazonS3 s3Client = null;
	private AWSCredentials credentials = null;
	private AwsClientBuilder.EndpointConfiguration endpointConfiguration = null;
	

	@BeforeClass
	private void setUp() throws Exception {		
		String[] acessKeys = RestClient.createUser(userName, roleName);
		credentials = new BasicAWSCredentials(acessKeys[0], acessKeys[1]);
		endpointConfiguration = new AwsClientBuilder.EndpointConfiguration(
				S3TestBase.s3ClientUrl, clientRegion);
		s3Client = AmazonS3ClientBuilder.standard().withEndpointConfiguration(endpointConfiguration)
				.withCredentials(new AWSStaticCredentialsProvider(credentials)).build();	
		createBuckets( s3Client );
	}

	@Test
	public void testCreateBucket() throws Exception {
		List<DeleteBucketThread> deleteBuckets = new ArrayList<>(20);
		GetBucketThread getBuckets = new GetBucketThread();
		
		List<String> existBuckets = new ArrayList<String>();
		for( int i = 0; i < defaultNums ; i++){
			String subBucketName = bucketName + "." + i;	
			if ( i%2 == 0 ){						
				deleteBuckets.add( new DeleteBucketThread(subBucketName));
			}else{
				existBuckets.add(subBucketName);
			}			
		}
		
		for( DeleteBucketThread deleteBucket : deleteBuckets ){
			deleteBucket.start();
		}		
		getBuckets.start();
		
		for( DeleteBucketThread deleteBucket : deleteBuckets ){
			Assert.assertTrue( deleteBucket.isSuccess(), deleteBucket.getErrorMsg());
		}
		Assert.assertTrue( getBuckets.isSuccess(), getBuckets.getErrorMsg());
		
		checkDeleteBucketResult(s3Client, existBuckets);
		runSuccess = true;			
	}

	@AfterClass
	private void tearDown() throws Exception {
		try {
			if (runSuccess) {
				clearBuckets();	
				RestClient.deleteUser(userName);
			}
		} finally {
			if( s3Client != null ){
				s3Client.shutdown();
			}			
		}
	}
	
	private class DeleteBucketThread extends S3ThreadBase{
		String bucketName;		
		public DeleteBucketThread ( String bucketName ){
			this.bucketName = bucketName;			
		}

		@Override
		public void exec() throws Exception {
			AmazonS3 s3Client = AmazonS3ClientBuilder.standard().withEndpointConfiguration(endpointConfiguration)
					.withCredentials(new AWSStaticCredentialsProvider(credentials)).build();
			try{				
				s3Client.deleteBucket(bucketName);				
			}finally{
				if (s3Client != null) {
					s3Client.shutdown();
				}
			}			
		}		
	}
	
	private class GetBucketThread extends S3ThreadBase{
		@Override
		public void exec() throws Exception {
			AmazonS3 s3Client = AmazonS3ClientBuilder.standard().withEndpointConfiguration(endpointConfiguration)
					.withCredentials(new AWSStaticCredentialsProvider(credentials)).build();
			try{				
				List<Bucket> buckets = s3Client.listBuckets();
				
				//only test list bucket success, the buckets not 0
				Assert.assertNotEquals(buckets.size(), 0);
			}finally{
				if (s3Client != null) {
					s3Client.shutdown();
				}
			}			
		}		
	}
	

	private void createBuckets( AmazonS3 s3Client ){
		for ( int i = 0; i < defaultNums; i++ ){
			String subBucketName = bucketName + "." + i;
			s3Client.createBucket(subBucketName);
		}
	}
	
	private void checkDeleteBucketResult(AmazonS3 s3Client, List<String> existBuckets) {
		// check bucket nums		
		List<Bucket> buckets = s3Client.listBuckets();
		Assert.assertEquals(buckets.size(), defaultNums/2);
		
		List<String> actExistBuckets = new ArrayList<String>();
		for ( int i = 0; i < buckets.size(); i++ ){
			Bucket bucket = buckets.get(i);
			String actBucketName = bucket.getName();	
			actExistBuckets.add(actBucketName);
		}	
		
		Collections.sort(actExistBuckets, new Comparator<Object>(){
			public int compare( Object a, Object b) {
				if( a instanceof String && b instanceof String ){
					return ((String)a).compareTo((String)b);
				}
				return 0;
			}
		});
		
		Collections.sort(existBuckets);
		Assert.assertEquals(actExistBuckets, existBuckets,
				"buckets actual:" + actExistBuckets + ";the expect :" + existBuckets);
	}

	private void clearBuckets(){
		for ( int i = 0; i < defaultNums; i++ ){
			String subBucketName = bucketName + "." + i;			
			try{
				s3Client.deleteBucket(subBucketName);			
			}catch(AmazonS3Exception e){
				if( !e.getErrorCode().equals("NoSuchBucket"))
				{
					Assert.fail("delete bucket:" + e.getErrorCode());
				}
			}	 
		}
	}
	
}
