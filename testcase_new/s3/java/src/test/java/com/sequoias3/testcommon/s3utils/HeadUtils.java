package com.sequoias3.testcommon.s3utils;

import com.amazonaws.services.s3.AmazonS3;
import com.sequoias3.testcommon.S3TestBase;

public class HeadUtils extends S3TestBase {	   
    /**
	 *delete the bucket
	 * @param s3Client
	 * @param bucketName
	 */
	@SuppressWarnings("deprecation")
	public static void clearOneBucket(AmazonS3 s3Client, String bucketName){
    	if( s3Client.doesBucketExist(bucketName)){
    		s3Client.deleteBucket(bucketName);;
    	}
    } 
}
