package com.sequoias3.testcommon.s3utils;

import java.io.File;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

import org.testng.Assert;

import com.amazonaws.services.s3.AmazonS3;
import com.amazonaws.services.s3.model.CompleteMultipartUploadRequest;
import com.amazonaws.services.s3.model.CompleteMultipartUploadResult;
import com.amazonaws.services.s3.model.InitiateMultipartUploadRequest;
import com.amazonaws.services.s3.model.InitiateMultipartUploadResult;
import com.amazonaws.services.s3.model.ListObjectsV2Request;
import com.amazonaws.services.s3.model.ListObjectsV2Result;
import com.amazonaws.services.s3.model.ObjectMetadata;
import com.amazonaws.services.s3.model.PartETag;
import com.amazonaws.services.s3.model.S3ObjectSummary;
import com.amazonaws.services.s3.model.UploadPartRequest;
import com.amazonaws.services.s3.model.UploadPartResult;
import com.sequoias3.testcommon.S3TestBase;

/**
 * @Description part upload function common class
 * @author wuyan
 * @Date 2019.04.12
 * @version 1.00
 */
public class PartUploadUtils extends S3TestBase {	
	public static final int partLimitMinSize = 1024 * 1024 * 5;
	
	/**
	 * initiate multipart upload *
	 * 
	 * @author wuyan
	 * @param s3Client
	 * @param bucketName
	 * @param key	             
	 * @return the uploadId for partUpload,the type is string.
	 */
	public static String initPartUpload(AmazonS3 s3Client, String bucketName, String key){
		InitiateMultipartUploadRequest initRequest = new InitiateMultipartUploadRequest(bucketName, key);
		ObjectMetadata metadata = new ObjectMetadata();
		initRequest.setObjectMetadata(metadata);
		InitiateMultipartUploadResult result = s3Client.initiateMultipartUpload(initRequest);
		String uploadId = result.getUploadId();
		return uploadId;		
	}
	
	/**
	 * upload mulitpart*
	 * 
	 * @author wuyan
	 * @param s3Client
	 * @param bucketName
	 * @param key
	 * @param uploadId
	 * @param file
	 *       upload object file		             
	 * @return the list of part number and Etag
	 */
	public static List<PartETag> partUpload(AmazonS3 s3Client, String bucketName, String key, String uploadId, File file){
		return PartUploadUtils.partUpload( s3Client, bucketName, key, uploadId, file,PartUploadUtils.partLimitMinSize);
	}
	
	
	public static List<PartETag> partUpload(AmazonS3 s3Client, String bucketName, String key, String uploadId, File file,long partSize){
		List<PartETag> partEtags = new ArrayList<>();
		int filePosition = 0;		
		long fileSize = file.length();		
		for( int i = 1; filePosition < fileSize; i++){
			long eachPartSize = Math.min( partSize, fileSize -  filePosition);
			UploadPartRequest partRequest = new UploadPartRequest().withFile(file)
					.withFileOffset(filePosition).withPartNumber(i).withPartSize(eachPartSize)
					.withBucketName(bucketName).withKey(key).withUploadId(uploadId);
			UploadPartResult uploadPartResult = s3Client.uploadPart(partRequest);
			partEtags.add(uploadPartResult.getPartETag());
			filePosition += partSize;	
		}	
		return partEtags;
	}

	/**
	 * complete multipart upload *
	 * 
	 * @author wuyan
	 * @param s3Client
	 * @param bucketName
	 * @param key
	 * @param uploadId
	 * @param partEtags	
	 * 				container for the part number and Etag of an uploaded part              
	 * @return the result infos by complete multipart upload.
	 */
	public static CompleteMultipartUploadResult completeMultipartUpload(AmazonS3 s3Client, String bucketName, String key,String uploadId,List<PartETag> partEtags){
		CompleteMultipartUploadRequest completeRequest = new CompleteMultipartUploadRequest()
				.withBucketName(bucketName).withKey(key).withUploadId(uploadId)
				.withPartETags(partEtags);		
		CompleteMultipartUploadResult result = s3Client.completeMultipartUpload(completeRequest);
		return result;
	}
	
	
	
	/**
	 * list objects with delimiter,than check the correctness of the returned
	 * result *
	 * 
	 * @author wuyan
	 * @param s3Client
	 * @param bucketName
	 * @param delimiter
	 *            specify the delimiter to list
	 * @param expKeyList
	 *            generated directory list,expected to match commonPrefixes
	 * @param matchContentsList
	 *            the keys expected to not match delimter
	 */
	public static void listObjectsWithDelimiter(AmazonS3 s3Client, String bucketName, String delimiter,
			List<String> expKeyList, List<String> matchContentsList) {
		ListObjectsV2Request request = new ListObjectsV2Request().withBucketName(bucketName).withEncodingType("url");
		request.withDelimiter(delimiter);
		ListObjectsV2Result result = s3Client.listObjectsV2(request);
		List<String> commonPrefixes = result.getCommonPrefixes();
		Collections.sort(expKeyList);
		Collections.sort(commonPrefixes);
		Assert.assertEquals(commonPrefixes, expKeyList,
				"actPrefixes:" + commonPrefixes.toString() + "\n ecpPrefixes:" + expKeyList.toString());
		// objects do not match delimiter are displayed in contents,num is 10
		List<String> actContentsList = new ArrayList<>();
		List<S3ObjectSummary> objects = result.getObjectSummaries();
		for (S3ObjectSummary os : objects) {
			String key = os.getKey();
			actContentsList.add(key);
		}

		// check the keyName
		Collections.sort(actContentsList);
		Collections.sort(matchContentsList);
		Assert.assertEquals(actContentsList, matchContentsList, "actcontent:" + actContentsList.toString());
	}

	

	
	
	
	
}
