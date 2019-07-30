package com.sequoias3.testcommon.s3utils;

import java.io.File;
import java.util.ArrayList;
import java.util.List;

import org.testng.Assert;

import com.amazonaws.services.s3.AmazonS3;
import com.amazonaws.services.s3.model.CompleteMultipartUploadRequest;
import com.amazonaws.services.s3.model.CompleteMultipartUploadResult;
import com.amazonaws.services.s3.model.InitiateMultipartUploadRequest;
import com.amazonaws.services.s3.model.InitiateMultipartUploadResult;
import com.amazonaws.services.s3.model.ListPartsRequest;
import com.amazonaws.services.s3.model.ObjectMetadata;
import com.amazonaws.services.s3.model.PartETag;
import com.amazonaws.services.s3.model.PartListing;
import com.amazonaws.services.s3.model.PartSummary;
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
	 * list Parts,than check the partNumber of the returned result 
	 * @author wuyan
	 * @param s3Client
	 * @param bucketName
	 * @param keyName	             
	 * @param partEtags	             
	 * @param uploadId	 *            
	 */
	public static void listPartsAndCheckPartNumbers( AmazonS3 s3Client, String bucketName, String keyName,List<PartETag> partEtags,String uploadId ){
		List<Integer> expPartNumbersList = new ArrayList<>();	
		for (PartETag expPartNumbers : partEtags) {
			int partNumber = expPartNumbers.getPartNumber();			
			expPartNumbersList.add(partNumber);
		}
		
		ListPartsRequest request = new ListPartsRequest( bucketName, keyName, uploadId);
		PartListing listResult = s3Client.listParts(request);
		List<PartSummary> listParts = listResult.getParts();
		List<Integer> actPartNumbersList = new ArrayList<>();		
		for (PartSummary partNumbers : listParts) {
			int partNumber = partNumbers.getPartNumber();			
			actPartNumbersList.add(partNumber);
		}
		
		// check the keyName
		Assert.assertEquals(actPartNumbersList, expPartNumbersList, "actPartNumbersList:" + actPartNumbersList);
	}
	
}
