package com.sequoiadb.testcommon;

import org.testng.annotations.AfterSuite;
import org.testng.annotations.BeforeSuite;
import org.testng.annotations.Parameters;

import com.sequoiadb.base.Sequoiadb;

public class SdbTestBase {
    protected static String coordUrl;
    protected static String hostName;
    protected static String serviceName;
    protected static String csName;
    protected static int reservedPortBegin;
    protected static int reservedPortEnd;
    protected static String reservedDir;
    protected static String workDir;

    @Parameters({"HOSTNAME", "SVCNAME", "CHANGEDPREFIX",
            "RSRVPORTBEGIN", "RSRVPORTEND", "RSRVNODEDIR", "WORKDIR"})
    @BeforeSuite
    public static void initSuite(String HOSTNAME, String SVCNAME, String COMMCSNAME,
                                 int RSRVPORTBEGIN, int RSRVPORTEND, String RSRVNODEDIR,
                                 String WORKDIR) {
        hostName = HOSTNAME;
        serviceName = SVCNAME;
        csName = COMMCSNAME;
        reservedPortBegin = RSRVPORTBEGIN;
        reservedPortEnd = RSRVPORTEND;
        reservedDir = RSRVNODEDIR;
        workDir = WORKDIR;
        coordUrl = HOSTNAME + ":" + SVCNAME;

        Sequoiadb db = null;
        try {
            db = new Sequoiadb(coordUrl, "", "");
            if (db.isCollectionSpaceExist(csName)) db.dropCollectionSpace(csName);
            db.createCollectionSpace(csName);
        } finally {
            if (db != null) {
                db.disconnect();
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
                db.disconnect();
            }
        }
    }

    public static String getDefaultCoordUrl() {
        return coordUrl;
    }

    public static String getWorkDir() {
        return workDir;
    }
}
