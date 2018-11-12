package com.sequoias3.object;

import java.io.File;
import java.io.FileOutputStream;
import java.util.Iterator;

import com.amazonaws.services.s3.AmazonS3;
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
    		String keyName) throws Exception {
    	S3Object object = s3Client.getObject(bucketName, keyName);
		S3ObjectInputStream s3is = object.getObjectContent();		
		String downloadPath = TestTools.LocalFile.initDownloadPath(localPath, TestTools.getMethodName(),
				Thread.currentThread().getId());
        FileOutputStream fos = new FileOutputStream(new File(downloadPath), true);
        byte[] read_buf = new byte[1024];
        int read_len = 0;
        while ((read_len = s3is.read(read_buf)) > -1) {
            fos.write(read_buf, 0, read_len);
        }
        s3is.close();
        fos.close();
        
        String getMd5 = TestTools.getMD5(downloadPath);
        return getMd5;
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
				System.out.println("----getkey="+vs.getKey());
				
				if ( getKey.equals(keyName)){
					System.out.println("----deleteKey="+vs.getKey());
					System.out.println("----getversion="+vs.getVersionId());
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
}
