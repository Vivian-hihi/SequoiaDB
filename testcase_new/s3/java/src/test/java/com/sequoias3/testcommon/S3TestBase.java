package com.sequoias3.testcommon;

import java.io.File;
import java.util.List;

import org.testng.annotations.AfterSuite;
import org.testng.annotations.BeforeSuite;
import org.testng.annotations.Parameters;

import com.amazonaws.services.s3.AmazonS3;
import com.amazonaws.services.s3.model.Bucket;
import com.amazonaws.services.s3.model.CreateBucketRequest;

public class S3TestBase {
    protected static String coordUrl;
    protected static String hostName;
    protected static String serviceName;
    protected static String s3ClientUrl;
    protected static String s3HostName;
    protected static String s3Port;
    protected static String csName;
    protected static String bucketName;
    protected static String enableVerBucketName;
    protected static int reservedPortBegin;
    protected static int reservedPortEnd;
    protected static String reservedDir;
    protected static String workDir;
    protected static String s3UserName;
    protected static String s3AccessKeyId;

    @Parameters({"HOSTNAME", "SVCNAME", "CHANGEDPREFIX", "RSRVPORTBEGIN", "RSRVPORTEND",
    	"RSRVNODEDIR", "WORKDIR","S3HOSTNAME","S3PORT","S3USERNAME","S3ACCESSKEYID"})
    @BeforeSuite
    public static void initSuite(String HOSTNAME, String SVCNAME, String COMMCSNAME,
                                 int RSRVPORTBEGIN, int RSRVPORTEND, String RSRVNODEDIR,
                                 String WORKDIR,String S3HOSTNAME, String S3PORT,String S3USERNAME,
                                 String S3ACCESSKEYID) {
        hostName = HOSTNAME;
        serviceName = SVCNAME;
        csName = COMMCSNAME;
        reservedPortBegin = RSRVPORTBEGIN;
        reservedPortEnd = RSRVPORTEND;
        reservedDir = RSRVNODEDIR;
        workDir = WORKDIR;
        coordUrl = HOSTNAME + ":" + SVCNAME;
        s3HostName = S3HOSTNAME;
        s3Port = S3PORT;
        s3UserName = S3USERNAME;
        s3AccessKeyId=S3ACCESSKEYID;
        s3ClientUrl = "http://"+ S3HOSTNAME + ":" + S3PORT;
        bucketName = "commbucket";
        enableVerBucketName = "commbucketwithversion";

        //clean file
        File workDirFile = new File(workDir);
        if (!workDirFile.exists()) {
            workDirFile.mkdir();
        }        
       
        AmazonS3 s3Client = null;
        try{
        	//clean up existing buckets         
            s3Client = CommLib.buildS3Client();        
            List<Bucket> buckets = s3Client.listBuckets();
            for ( int i = 0; i < buckets.size(); i++ ){
    			String bucketName = buckets.get(i).getName();
    			String bucketVerStatus = s3Client.getBucketVersioningConfiguration(bucketName).getStatus();
    			if( bucketVerStatus == "null"){
    				CommLib.deleteAllObjects(s3Client, bucketName);
    			}else{
    				CommLib.deleteAllObjectVersions( s3Client, bucketName );;	
    			}
    			s3Client.deleteBucket(bucketName);
    		}          
            
            //create bucket       
            s3Client.createBucket(new CreateBucketRequest(bucketName));	
            
            //create bucket by enable versioning            
            s3Client.createBucket(new CreateBucketRequest(enableVerBucketName));
            CommLib.setBucketVersioning(s3Client, enableVerBucketName, "Enabled");            
        }finally {
            if (s3Client != null) {
                s3Client.shutdown();
            }
        }        
    }

    @AfterSuite
    public static void finiSuite() {
       
    }

    public static String getDefaultCoordUrl() {
        return coordUrl;
    }

    public static String getWorkDir() {
        return workDir;
    }
    
    public static String getDefaultS3ClientUrl() {
        return s3ClientUrl;
    }
}
