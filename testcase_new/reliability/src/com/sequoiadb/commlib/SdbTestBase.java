package com.sequoiadb.commlib ;

import org.testng.Assert ;
import org.testng.annotations.AfterSuite ;
import org.testng.annotations.BeforeSuite ;
import org.testng.annotations.Parameters ;

import com.sequoiadb.base.Sequoiadb ;
import com.sequoiadb.exception.BaseException ;

public class SdbTestBase {
    public static String coordUrl ;
    public static String hostName ;
    public static String serviceName ;
    public static String csName;
    public static int reservedPortBegin;
    public static int reservedPortEnd;
    public static String reservedDir ;
    public static String workDir;
    public static String rootPwd ;
    public static String remoteUser;
    public static String remotePwd;
   
    @Parameters( { "HOSTNAME", "SVCNAME", "CHANGEDPREFIX", "RSRVPORTBEGIN",
            "RSRVPORTEND", "RSRVNODEDIR", "WORKDIR", "ROOTPASSWD",
            "REMOTEUSER", "REMOTEPASSWD" } )
    @BeforeSuite
    public static void initSuite( String HOSTNAME, String SVCNAME,
            String COMMCSNAME, int RSRVPORTBEGIN, int RSRVPORTEND,
            String RSRVNODEDIR, String WORKDIR, String ROOTPASSWD,
            String REMOTEUSER, String REMOTEPASSWD ) {
        hostName = HOSTNAME ;
        serviceName = SVCNAME ;
        csName = COMMCSNAME ;
        reservedPortBegin = RSRVPORTBEGIN ;
        reservedPortEnd = RSRVPORTEND ;
        reservedDir = RSRVNODEDIR ;
        workDir = WORKDIR ;
        coordUrl = HOSTNAME + ":" + SVCNAME ;
        rootPwd = ROOTPASSWD ;
        remoteUser = REMOTEUSER ;
        remotePwd = REMOTEPASSWD ;
        Sequoiadb db = null ;
        try {
            db = new Sequoiadb( coordUrl, "", "" ) ;
            boolean ret = createCommonCS( db ) ;
            Assert.assertTrue( ret ) ;
        } catch ( BaseException e ) {
            Assert.fail( "connect " + coordUrl + ": " + e.getErrorCode() ) ;
        } finally {
            if ( db != null ) {
                db.disconnect() ;
            }
        }
    }

    @AfterSuite
    public static void finiSuite() {
        Sequoiadb db = null ;
        System.out.println( "finiSuite" ) ;
        try {
            db = new Sequoiadb( coordUrl, "", "" ) ;
            if ( db.isCollectionSpaceExist( csName ) ) {
                db.dropCollectionSpace( csName ) ;
            }
        } catch ( BaseException e ) {
            e.printStackTrace() ;
        } finally {
            if ( db != null ) {
                db.disconnect() ;
            }
        }
    }

    private static boolean createCommonCS( Sequoiadb sdb ) {
        boolean isCreateSuccess = true ;
        try {
            if ( sdb.isCollectionSpaceExist( csName ) ) {
                sdb.dropCollectionSpace( csName ) ;
            }
            sdb.createCollectionSpace( csName ) ;
        } catch ( BaseException e ) {
            System.out.printf( "create CollectionSpace %s failed, errMsg:%s\n",
                    csName, e.getMessage() ) ;
            isCreateSuccess = false ;
        }
        return isCreateSuccess ;
    }
    
    

}
