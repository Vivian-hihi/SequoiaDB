package com.sequoias3.commlibs3;

import com.amazonaws.services.s3.AmazonS3;
import com.amazonaws.services.s3.model.Bucket;
import com.amazonaws.services.s3.model.CreateBucketRequest;
import com.sequoiadb.base.DBCursor;
import com.sequoiadb.base.ReplicaGroup;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.commlib.SdbTestBase;
import com.sequoiadb.commlib.Ssh;
import com.sequoiadb.exception.BaseException;
import org.bson.BSONObject;
import org.bson.types.BasicBSONList;
import org.testng.Assert;
import org.testng.annotations.AfterSuite;
import org.testng.annotations.BeforeSuite;
import org.testng.annotations.Parameters;

import java.io.*;
import java.util.Arrays;
import java.util.List;
import java.util.Properties;

public class S3TestBase {
    public static String coordUrl;
    public static String hostName;
    public static String serviceName;
    public static String csName;
    public static int reservedPortBegin;
    public static int reservedPortEnd;
    public static String reservedDir;
    public static String workDir;
    public static String rootPwd;
    public static String remoteUser;
    public static String remotePwd;
    public static String scriptDir;

    public static String s3ClientUrl;
    public static String s3HostName;
    public static String s3Port;
    public static String bucketName;
    public static String enableVerBucketName;
    public static String s3UserName;
    public static String s3AccessKeyId;
    public static String confTool;
    public static SdbConfTestBase sdbConfTestBase = new SdbConfTestBase();
    public static String installPath;

    @Parameters({"HOSTNAME", "SVCNAME", "CHANGEDPREFIX", "RSRVPORTBEGIN", "RSRVPORTEND",
        "RSRVNODEDIR", "WORKDIR","S3HOSTNAME","S3PORT","S3USERNAME","S3ACCESSKEYID","CONFTOOL",
        "ROOTPASSWD", "REMOTEUSER", "REMOTEPASSWD", "SCRIPTDIR"})
    @BeforeSuite
    public void initSuite(String HOSTNAME, String SVCNAME, String COMMCSNAME,
                                 int RSRVPORTBEGIN, int RSRVPORTEND, String RSRVNODEDIR,
                                 String WORKDIR,String S3HOSTNAME, String S3PORT,String S3USERNAME,
                                 String S3ACCESSKEYID, String CONFTOOL,String ROOTPASSWD,
                                 String REMOTEUSER, String REMOTEPASSWD, String SCRIPTDIR) {

        SdbTestBase.hostName = hostName = HOSTNAME;
        SdbTestBase.serviceName = serviceName = SVCNAME;
        SdbTestBase.csName = csName = COMMCSNAME;
        SdbTestBase.reservedPortBegin = reservedPortBegin  = RSRVPORTBEGIN;
        SdbTestBase.reservedPortEnd = reservedPortEnd = RSRVPORTEND;
        SdbTestBase.reservedDir = reservedDir = RSRVNODEDIR;
        SdbTestBase.workDir = workDir = WORKDIR;
        SdbTestBase.coordUrl = coordUrl = HOSTNAME + ":" + SVCNAME;
        SdbTestBase.rootPwd = rootPwd = ROOTPASSWD;
        SdbTestBase.remoteUser = remoteUser = REMOTEUSER;
        SdbTestBase.remotePwd = remotePwd = REMOTEPASSWD;
        SdbTestBase.scriptDir = scriptDir = SCRIPTDIR;

        s3HostName = S3HOSTNAME;
        s3Port = S3PORT;
        s3UserName = S3USERNAME;
        s3AccessKeyId=S3ACCESSKEYID;
        s3ClientUrl = "http://"+ S3HOSTNAME + ":" + S3PORT;
        bucketName = "commbucket";
        enableVerBucketName = "commbucketwithversion";
        confTool = CONFTOOL;

        getInstallPath();
        sdbConfTestBase.openTransaction(confTool, hostName, serviceName);
        createCSCLAndStartS3();
        //clean file
        File workDirFile = new File(workDir);
        if (!workDirFile.exists()) {
            workDirFile.mkdir();
        }
        createCommonCS();
        cleanS3EnvAndPrepare();
    }

    @AfterSuite
    public  void finiSuite(){
        try {
            getClusterInfo();
            sdbConfTestBase.closeTransaction(hostName, serviceName);
        } catch (IOException e) {
            e.printStackTrace();
            Assert.fail(e.getMessage());
        } finally {
            stopS3();
        }
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
                //change log level
                
                File file = new File(installPath+"/tools/sequoias3/config/logback.xml" );
                BufferedReader br = new BufferedReader(new InputStreamReader(new FileInputStream(file), "UTF-8"));
                CharArrayWriter caw = new CharArrayWriter();
                line = null;
                while((line=br.readLine()) != null) {
                    line = line.replaceAll("INFO", "DEBUG");
                    caw.write(line);
                    caw.append(System.getProperty("line.separator"));
                }
                br.close();
                FileWriter fw = new FileWriter(file);
                caw.writeTo(fw);
                fw.close();
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
            startS3();
        }
        catch( InterruptedException | IOException e ){
            e.printStackTrace();
            Assert.fail( "fail to createCSCLAndStartS3" );
        }
    }

    public static void getInstallPath() {
        try {
           Properties prop = new Properties();
           InputStream in = new FileInputStream( new File("/etc/default/sequoiadb") );
           prop.load( in );
           installPath = prop.getProperty( "INSTALL_DIR" );
           installPath = "/opt/sequoiadb";
        }
        catch( IOException e ){
            e.printStackTrace();
            Assert.fail( "fail to get installPath" );
        }
    }
    
    public static void getClusterInfo() throws IOException{
        Sequoiadb db = null;
        try{
            db = new Sequoiadb(coordUrl, "", "");
            DBCursor cur = db.getList(7, null, null, null);
            String info = "";
            while (cur.hasNext()) {
                info+=cur.getNext().toString();
            }
            cur.close();
            File file = new File(installPath+"/tools/sequoias3/log/cluster.log");
            if(!file.exists()) {
                file.createNewFile();
            }
            
            FileWriter fw = new FileWriter(file.getAbsoluteFile(),false);
            BufferedWriter bw = new BufferedWriter(fw);
            bw.write(info);
            bw.close();
        } catch(BaseException e) {
            Assert.fail("connect " + coordUrl + " get cluster info error : " + e.getErrorCode());
        } finally {
            if (db != null) {
                db.disconnect();
            }
        }
    }

    public static void startS3() throws IOException, InterruptedException {
        BufferedReader input = null;
        Process process = null;
        try {
            String[] cmd = new String[2];
            cmd[0] = installPath + "/tools/sequoias3/sequoias3.sh";
            cmd[1] = "start";
            System.out.println("exec cmd: " + Arrays.toString(cmd));
            process = Runtime.getRuntime().exec(cmd);
            input = new BufferedReader(new InputStreamReader(process.getInputStream()));
            int exitValue = process.waitFor();
            if (0 != exitValue) {
                Assert.fail("fail to start s3, return code=" + exitValue);
            }
        } finally {
            if (input != null) {
                input.close();
            }
            if(process != null){
                process.destroy();
            }
        }
    }
    
    public static void stopS3() {
        try{
            String[] cmd = new String[3];
            cmd[0] = installPath+"/tools/sequoias3/sequoias3.sh";
            cmd[1] = "stop";
            cmd[2] = "-a";
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

    public static String getSdbInstallDir(String host,String username,String password) throws Exception {
        Ssh ssh = new Ssh(host, username, password);
        String dir = null;
        try {
            ssh.exec("cat /etc/default/sequoiadb |grep INSTALL_DIR");
            String str = ssh.getStdout();
            if (str.length() <= 0) {
                throw new Exception(
                        "exec command:cat /etc/default/sequoiadb |grep INSTALL_DIR can not find sequoiacm install dir");
            }
            dir = str.substring(str.indexOf("=") + 1, str.length() - 1);
        } finally {
            ssh.disconnect();
        }
        return dir;
    }

    public static void createCommonCS(){
        Sequoiadb db = null;
        try {
            db = new Sequoiadb(coordUrl, "", "");
            if(!db.isCollectionSpaceExist(csName)){
                db.createCollectionSpace(csName);
            }
        }  finally {
            if (db != null) {
                db.close();
            }
        }
    }

    public void cleanS3EnvAndPrepare() {
        AmazonS3 s3Client = null;
        try {
            //clean up existing buckets
            s3Client = CommLibS3.buildS3Client();
            List<Bucket> buckets = s3Client.listBuckets();
            for (int i = 0; i < buckets.size(); i++) {
                String bucketName = buckets.get(i).getName();
                String bucketVerStatus = s3Client.getBucketVersioningConfiguration(bucketName).getStatus();
                if (bucketVerStatus == "null") {
                    CommLibS3.deleteAllObjects(s3Client, bucketName);
                } else {
                    CommLibS3.deleteAllObjectVersions(s3Client, bucketName);
                }
                s3Client.deleteBucket(bucketName);
            }
            //create bucket
            s3Client.createBucket(new CreateBucketRequest(bucketName));
            //create bucket by enable versioning
            s3Client.createBucket(new CreateBucketRequest(enableVerBucketName));
            CommLibS3.setBucketVersioning(s3Client, enableVerBucketName, "Enabled");
        } finally {
            if (s3Client != null) {
                s3Client.shutdown();
            }
        }
    }
}
