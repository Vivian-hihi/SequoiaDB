package com.sequoias3.testcommon.s3utils;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.util.Iterator;

import com.amazonaws.services.s3.AmazonS3;
import com.amazonaws.services.s3.model.GetObjectRequest;
import com.amazonaws.services.s3.model.ListVersionsRequest;
import com.amazonaws.services.s3.model.S3Object;
import com.amazonaws.services.s3.model.S3ObjectInputStream;
import com.amazonaws.services.s3.model.S3VersionSummary;
import com.amazonaws.services.s3.model.VersionListing;
import com.sequoias3.testcommon.S3TestBase;
import com.sequoias3.testcommon.TestTools;

public class ObjectUtils extends S3TestBase {
    

	/**
	 * download the object ,than get the object content md5
	 * @param s3Client
	 * @param localPath
	 * @param bucketName
	 * @param keyName
	 * @return md5
	 */
    public static String getMd5OfObject(AmazonS3 s3Client, File localPath,String bucketName, 
    		String key) throws Exception {
    	return getMd5OfObject(s3Client, localPath,bucketName, key, null);
    }   
    
    /**
	 * download the object with versionId,than get the object content md5
	 * @param s3Client
	 * @param localPath
	 * @param bucketName
	 * @param keyName
	 * @param versionId
	 * @return md5
	 */
    public static String getMd5OfObject(AmazonS3 s3Client, File localPath,String bucketName, 
    		String key, String versionId) throws Exception { 
    	GetObjectRequest request = new GetObjectRequest(bucketName, key, versionId);
    	S3Object object = s3Client.getObject(request);
		S3ObjectInputStream s3is = object.getObjectContent();		
		String downloadPath = TestTools.LocalFile.initDownloadPath(localPath, TestTools.getMethodName(),
				Thread.currentThread().getId());
		inputStream2File(s3is,downloadPath);
		s3is.close();
        String getMd5 = TestTools.getMD5(downloadPath);
        return getMd5;
    }

	/**
	 *input stream to file
	 * @param inputStream
	 * @param downloadPath
	 */
    public static String inputStream2File(InputStream inputStream,String downloadPath) throws IOException {
		FileOutputStream fos = null;
		try {
			fos = new FileOutputStream(new File(downloadPath), true);
			byte[] read_buf = new byte[1024];
			int read_len = 0;
			while ((read_len = inputStream.read(read_buf)) > -1) {
				fos.write(read_buf, 0, read_len);
			}
		}finally{
			if(fos != null){
				fos.close();
			}
		}
		return downloadPath;
	}

    /**
	* delete the object of all versions(required for versioned buckets)
	* @param s3Client
	* @param bucketName	
	* @param keyName	
	*/
	public static void deleteObjectAllVersions( AmazonS3 s3Client,String bucketName,String keyName ){
		VersionListing versionList = s3Client.listVersions(new ListVersionsRequest()
					.withBucketName(bucketName));		
		while (true) {		
			Iterator<S3VersionSummary> versionIter = versionList.getVersionSummaries()
					.iterator();	
			
			while (versionIter.hasNext()) {			
				S3VersionSummary vs = versionIter.next();	
				String getKey = vs.getKey();				
				
				if ( getKey.equals(keyName)){					
					s3Client.deleteVersion(bucketName, vs.getKey(), vs.getVersionId());
				}
								
				s3Client.deleteVersion(bucketName, vs.getKey(), vs.getVersionId());
			}
				
			if( versionList.isTruncated()){
				versionList = s3Client.listNextBatchOfVersions(versionList);			
			}else{
				break;
			}
		}		
	}	
	
	public static void clearOneObject(AmazonS3 s3Client, String bucketName,String key){
    	if( s3Client.doesObjectExist(bucketName, key)){
    		s3Client.deleteObject(bucketName,key);
    	}
    } 
}
