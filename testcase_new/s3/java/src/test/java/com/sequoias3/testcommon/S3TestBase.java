package com.sequoias3.testcommon;

import java.io.File;

import org.testng.annotations.AfterSuite;
import org.testng.annotations.BeforeSuite;
import org.testng.annotations.Parameters;

import com.sequoiadb.base.Sequoiadb;

public class S3TestBase {
    protected static String coordUrl;
    protected static String hostName;
    protected static String serviceName;
    protected static String s3ClientUrl;
    protected static String s3HostName;
    protected static String s3Port;
    protected static String csName;
    protected static int reservedPortBegin;
    protected static int reservedPortEnd;
    protected static String reservedDir;
    protected static String workDir;
    protected static String s3UserName;

    @Parameters({"HOSTNAME", "SVCNAME", "CHANGEDPREFIX", "RSRVPORTBEGIN", "RSRVPORTEND",
    	"RSRVNODEDIR", "WORKDIR","S3HOSTNAME","S3PORT","S3USERNAME"})
    @BeforeSuite
    public static void initSuite(String HOSTNAME, String SVCNAME, String COMMCSNAME,
                                 int RSRVPORTBEGIN, int RSRVPORTEND, String RSRVNODEDIR,
                                 String WORKDIR,String S3HOSTNAME, String S3PORT,String S3USERNAME) {
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
        s3ClientUrl = "http://"+ S3HOSTNAME + ":" + S3PORT;

        Sequoiadb db = null;
        try {
            db = new Sequoiadb(coordUrl, "", "");
            if (db.isCollectionSpaceExist(csName)) db.dropCollectionSpace(csName);
            db.createCollectionSpace(csName);
            File workDirFile = new File(workDir);
            if (!workDirFile.exists()) {
                workDirFile.mkdir();
            }
        } finally {
            if (db != null) {
                db.close();
            }
        }
    }

    @AfterSuite
    public static void finiSuite() {
        Sequoiadb db = null;
        try {
            db = new Sequoiadb(coordUrl, "", "");
            if (db.isCollectionSpaceExist(csName)) {
                db.dropCollectionSpace(csName);
            }
        } finally {
            if (db != null) {
                db.close();
            }
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
}
