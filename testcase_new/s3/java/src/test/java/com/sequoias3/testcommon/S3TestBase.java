package com.sequoias3.testcommon;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileWriter;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.util.Arrays;
import java.util.List;
import java.util.Properties;

import org.bson.BSONObject;
import org.bson.types.BasicBSONList;
import org.testng.Assert;
import org.testng.annotations.AfterSuite;
import org.testng.annotations.BeforeSuite;
import org.testng.annotations.Parameters;

import com.amazonaws.services.s3.AmazonS3;
import com.amazonaws.services.s3.model.Bucket;
import com.amazonaws.services.s3.model.CreateBucketRequest;
import com.sequoiadb.base.ReplicaGroup;
import com.sequoiadb.base.Sequoiadb;

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
    protected static String confTool;
    private static SdbConfTestBase sdbConfTestBase = new SdbConfTestBase();

    @Parameters({"HOSTNAME", "SVCNAME", "CHANGEDPREFIX", "RSRVPORTBEGIN", "RSRVPORTEND",
        "RSRVNODEDIR", "WORKDIR","S3HOSTNAME","S3PORT","S3USERNAME","S3ACCESSKEYID","CONFTOOL"})
    @BeforeSuite
    public static void initSuite(String HOSTNAME, String SVCNAME, String COMMCSNAME,
                                 int RSRVPORTBEGIN, int RSRVPORTEND, String RSRVNODEDIR,
                                 String WORKDIR,String S3HOSTNAME, String S3PORT,String S3USERNAME,
                                 String S3ACCESSKEYID, String CONFTOOL) {
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
        confTool = CONFTOOL;

        sdbConfTestBase.openTransaction(confTool, hostName, serviceName);
        createCSCLAndStartS3();
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
                    CommLib.deleteAllObjectVersions( s3Client, bucketName );
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
        sdbConfTestBase.closeTransaction(hostName, serviceName);
        stopS3();
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
    
    public static void createCSCLAndStartS3() {
        try{
            Properties prop = new Properties();
            InputStream in = new FileInputStream( new File("/etc/default/sequoiadb") );
            prop.load( in );
            String installPath = prop.getProperty( "INSTALL_DIR" );
            String sdbFullName = installPath + "/bin/sdb";
            System.out.println("begin exec createCSCLexample.js");
            String[] strCmd = new String[5];
            strCmd[0] = sdbFullName;
            strCmd[1] = "-f";
            strCmd[2] = installPath+"/tools/sequoias3/createCSCLexample.js";
            strCmd[3] = "-e";
            strCmd[4] = "var COORDSVCNAME='" + serviceName + "'";
            System.out.println( "exec cmd: " + Arrays.toString( strCmd ) );
            Process process = Runtime.getRuntime().exec( strCmd );
         
            BufferedReader input = new BufferedReader( new InputStreamReader( process.getInputStream() ) );
            String line = "";
            while( (line = input.readLine()) != null ){
                System.out.println(line);
            }
         
            int exitValue = process.waitFor();
            if( 0 != exitValue ){
                Assert.fail( "fail to exec createCSCLexample.js, return code=" + exitValue );
            }
            System.out.println("finish exec createCSCLexample.js");
            //更新properties
            System.out.println("begin update application.properties");
            Sequoiadb localdb = null;
            try{
                localdb = new Sequoiadb(coordUrl, "", "");
                ReplicaGroup rg = localdb.getReplicaGroup("SYSCoord");
                BSONObject rgDetail = rg.getDetail();
                BasicBSONList groupInfo = (BasicBSONList) rgDetail.get("Group");
                String coordUrls = "";
                for(int i = 0;i<groupInfo.toArray().length;i++){
                    BSONObject groupObj =  (BSONObject) groupInfo.toArray()[i];
                    String groupName = (String) groupObj.get("HostName");
                    if ( i != 0 ) {
                        coordUrls = coordUrls+",";
                    }
                    coordUrls = coordUrls + groupName+":"+serviceName;
                }

                
                FileWriter f = new FileWriter( installPath+"/tools/sequoias3/config/application.properties" );
                f.write( "sdbs3.sequoiadb.url=sequoiadb://"     + coordUrls       + "\n" );
                f.flush();
                f.close();
                
                System.out.println("write properties: " + installPath + "/tools/sequoias3/config/application.properties" );
            }catch( Exception e ){
                e.printStackTrace();
                Assert.fail( "update application.properties file failed" );
            }finally {
                if(localdb != null) {
                    localdb.close();
                }
            }
            System.out.println("finish update application.properties");
            
            //启动
            String[] cmd = new String[2];
            cmd[0] = installPath+"/tools/sequoias3/sequoias3.sh";
            cmd[1] = "start";
            System.out.println( "exec cmd: " + Arrays.toString( cmd ) );
            process = Runtime.getRuntime().exec( cmd );
            input = new BufferedReader( new InputStreamReader( process.getInputStream() ) );

            exitValue = process.waitFor();
            if( 0 != exitValue ){
                Assert.fail( "fail to start s3, return code=" + exitValue );
            }  
        }
        catch( InterruptedException | IOException e ){
            e.printStackTrace();
            Assert.fail( "fail to createCSCLAndStartS3" );
        }
    }
    
    public static void stopS3() {
        try{
            Properties prop = new Properties();
            InputStream in = new FileInputStream( new File("/etc/default/sequoiadb") );
            prop.load( in );
            String installPath = prop.getProperty( "INSTALL_DIR" );
            String[] cmd = new String[2];
            cmd[0] = installPath+"/tools/sequoias3/sequoias3.sh";
            cmd[1] = "stop -a";
            System.out.println( "exec cmd: " + Arrays.toString( cmd ) );
            Process process = Runtime.getRuntime().exec( cmd );
            BufferedReader input = new BufferedReader( new InputStreamReader( process.getInputStream() ) );
    
            int exitValue = process.waitFor();
            if( 0 != exitValue ){
                Assert.fail( "fail to stop s3, return code=" + exitValue );
            } 
        }
        catch( InterruptedException | IOException e ){
            e.printStackTrace();
            Assert.fail( "fail to stop s3" );
        }
    }
}
