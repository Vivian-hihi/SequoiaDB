package com.sequoiadb.testcommon;

import java.io.File;

import org.testng.annotations.AfterSuite;
import org.testng.annotations.BeforeSuite;
import org.testng.annotations.Optional;
import org.testng.annotations.Parameters;
import org.elasticsearch.client.Client;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.utils.FullTextESUtils;
import com.sequoiadb.utils.FullTextUtils;

public class SdbTestBase {
    protected static String coordUrl;
    protected static String esUrl;
    protected static String hostName;
    protected static String serviceName;
    protected static String esHostName;
    protected static String esServiceName;
    protected static String csName;
    protected static int reservedPortBegin;
    protected static int reservedPortEnd;
    protected static String reservedDir;
    protected static String workDir;

    @Parameters({ "HOSTNAME", "SVCNAME", "ESHOSTNAME", "ESSVCNAME",
            "FULLTEXTPREFIX", "CHANGEDPREFIX", "RSRVPORTBEGIN", "RSRVPORTEND",
            "RSRVNODEDIR", "WORKDIR" })
    @BeforeSuite
    public static void initSuite( String HOSTNAME, String SVCNAME,
            @Optional("localhost") String ESHOSTNAME,
            @Optional("9300") String ESSVCNAME,
            @Optional("") String FULLTEXTPREFIX, String COMMCSNAME,
            int RSRVPORTBEGIN, int RSRVPORTEND, String RSRVNODEDIR,
            String WORKDIR ) {
        hostName = HOSTNAME;
        serviceName = SVCNAME;
        esHostName = ESHOSTNAME;
        esServiceName = ESSVCNAME;
        csName = COMMCSNAME;
        reservedPortBegin = RSRVPORTBEGIN;
        reservedPortEnd = RSRVPORTEND;
        reservedDir = RSRVNODEDIR;
        workDir = WORKDIR;
        coordUrl = HOSTNAME + ":" + SVCNAME;
        FullTextUtils.setFulltextPrefix( FULLTEXTPREFIX );

        Sequoiadb db = null;
        Client esClient = null;
        try {
            esClient = FullTextESUtils.createTransportClient( esHostName,
                    Integer.parseInt( esServiceName ) );
            db = new Sequoiadb( coordUrl, "", "" );
            if ( db.isCollectionSpaceExist( csName ) )
                db.dropCollectionSpace( csName );
            db.createCollectionSpace( csName );
            File workDirFile = new File( workDir );
            if ( !workDirFile.exists() ) {
                workDirFile.mkdir();
            }
        } finally {
            if ( db != null ) {
                db.disconnect();
            }
            if ( esClient != null ) {
                esClient.close();
            }
        }
    }

    @AfterSuite
    public static void finiSuite() {
        Sequoiadb db = null;
        try {
            db = new Sequoiadb( coordUrl, "", "" );
            if ( db.isCollectionSpaceExist( csName ) ) {
                db.dropCollectionSpace( csName );
            }
        } finally {
            if ( db != null ) {
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
