package com.sequoias3.testcommon;

import java.util.List;

import org.testng.Assert;

import com.amazonaws.ClientConfiguration;
import com.amazonaws.auth.AWSCredentials;
import com.amazonaws.auth.AWSStaticCredentialsProvider;
import com.amazonaws.auth.BasicAWSCredentials;
import com.amazonaws.client.builder.AwsClientBuilder;
import com.amazonaws.services.s3.AmazonS3;
import com.amazonaws.services.s3.AmazonS3ClientBuilder;
import com.amazonaws.services.s3.model.AmazonS3Exception;
import com.amazonaws.services.s3.model.Bucket;

public class CommLib {
	private static String AWS_ACCESS_KEY = "ABCDEFGHIJKLMNOPQRST";
	private static String AWS_SECRET_KEY = "abcdefghijklmnopqrstuvwxyz0123456789ABCD";
	private static String clientRegion = "us-east-1";
	
	/**
	 * build S3 client by admin
	 * @param sdb
	 * @return s3Client
	 */
	public static AmazonS3 buildS3Client(){
		return buildS3Client(AWS_ACCESS_KEY,AWS_SECRET_KEY);
	}
	
	public static AmazonS3 buildS3Client(String ACCESS_KEY,String SECRET_KEY){
		AmazonS3 s3Client = null;
		AWSCredentials credentials = new BasicAWSCredentials(ACCESS_KEY,SECRET_KEY);
		AwsClientBuilder.EndpointConfiguration endpointConfiguration = new AwsClientBuilder.EndpointConfiguration(
				S3TestBase.s3ClientUrl, clientRegion);
		ClientConfiguration config = new ClientConfiguration();
		config.setUseExpectContinue(false);
		s3Client = AmazonS3ClientBuilder.standard()
				.withEndpointConfiguration(endpointConfiguration)
				.withClientConfiguration(config)
				.withChunkedEncodingDisabled(true)
				.withCredentials(new AWSStaticCredentialsProvider(credentials)).build();
		return s3Client;		
	}
	
	public static AmazonS3 buildS3ClientWithVersion(){
		AmazonS3 s3Client = null;
		AWSCredentials credentials = new BasicAWSCredentials(AWS_ACCESS_KEY,AWS_SECRET_KEY);
		AwsClientBuilder.EndpointConfiguration endpointConfiguration = new AwsClientBuilder.EndpointConfiguration(
				S3TestBase.s3ClientUrl, clientRegion);
//		BucketVersioningConfiguration configuration =
//				new BucketVersioningConfiguration().withStatus("Enabled");
		s3Client = AmazonS3ClientBuilder.standard()
				.withEndpointConfiguration(endpointConfiguration)
				.withCredentials(new AWSStaticCredentialsProvider(credentials)).build();
		return s3Client;		
	}
	
	/**
	 * delete one bucket by bucketName
	 * @param s3Client,bucketName 
	 */	
	public static void clearBucket( AmazonS3 s3Client, String bucketName ){
		// if(s3Client.doesBucketExist(bucketName)){
			// System.out.println("----is exist="+s3Client.doesBucketExist(bucketName));
			 //s3Client.deleteBucket(bucketName);
		// }
		try{
			s3Client.deleteBucket(bucketName);			
		}catch(AmazonS3Exception e){
			if( !e.getErrorCode().equals("NoSuchBucket"))
			{
				Assert.fail("delete bucket:" + e.getErrorCode());
			}
		}	 
	}
	
	
	/**
	 * delete all buckets
	 * @param s3Client
	 */
	public static void clearBuckets( AmazonS3 s3Client ){
		List<Bucket> buckets = s3Client.listBuckets();
		if( buckets.size() != 0 ){
			for ( int i = 0; i < buckets.size(); i++ ){
				String bucketName = buckets.get(i).getName();
				s3Client.deleteBucket(bucketName);			
			}
		}
	}
	
}
