package com.sequoiadb.testcommon ;

import java.io.BufferedReader ;
import java.io.File ;
import java.io.IOException ;
import java.io.InputStreamReader ;
import java.util.ArrayList ;
import java.util.List ;
import java.util.concurrent.atomic.AtomicInteger ;

import org.bson.BSONObject ;
import org.bson.BasicBSONObject ;
import org.bson.types.BasicBSONList ;
import org.testng.Assert ;
import org.testng.SkipException ;
import org.testng.annotations.AfterGroups ;
import org.testng.annotations.AfterSuite ;
import org.testng.annotations.AfterTest ;
import org.testng.annotations.BeforeGroups ;
import org.testng.annotations.BeforeSuite ;
import org.testng.annotations.BeforeTest ;
import org.testng.annotations.Parameters ;

import com.sequoiadb.base.ConfigOptions ;
import com.sequoiadb.base.DBCursor ;
import com.sequoiadb.base.Sequoiadb ;
import com.sequoiadb.exception.BaseException ;
import com.sequoiadb.exception.SDBError ;

public class SdbTestBase {
    protected static String coordUrl ;
    protected static String hostName ;
    protected static String serviceName ;
    protected static String csName ;
    protected static int reservedPortBegin ;
    protected static int reservedPortEnd ;
    protected static String reservedDir ;
    protected static String workDir ;
    private static Sequoiadb sequoiadb = null ;
    private static final String ROLE = "Role" ;
    private static final String DATA = "data" ;
    private static final String TRANSACTIONON = "transactionon" ;
    private static final String TRANSISOLATION = "transisolation" ;
    private static final String TRANSLOCKWAIT = "translockwait" ;
    private static final String INDEXSCANSTEP = "indexscanstep" ;
    
    private static AtomicInteger count = new AtomicInteger( 0 ) ;
    private static ConfigOptions options = new ConfigOptions() ;
    
    private static final int newIndexScanStep = 3 ;
    private static final int originalIndexScanStep = 100 ;

    @Parameters( { "HOSTNAME", "SVCNAME", "CHANGEDPREFIX", "RSRVPORTBEGIN",
            "RSRVPORTEND", "RSRVNODEDIR", "WORKDIR" } )
    @BeforeSuite(alwaysRun = true)
    public static void initSuite( String HOSTNAME, String SVCNAME,
            String COMMCSNAME, int RSRVPORTBEGIN, int RSRVPORTEND,
            String RSRVNODEDIR, String WORKDIR ) {
        System.out.println("initSuite.....") ;
        hostName = HOSTNAME ;
        serviceName = SVCNAME ;
        csName = COMMCSNAME ;
        reservedPortBegin = RSRVPORTBEGIN ;
        reservedPortEnd = RSRVPORTEND ;
        reservedDir = RSRVNODEDIR ;
        workDir = WORKDIR ;
        coordUrl = HOSTNAME + ":" + SVCNAME ;

        try {
            options.setSocketKeepAlive( true ) ;
            sequoiadb = new Sequoiadb( SdbTestBase.coordUrl, "", "", options ) ;
            if ( sequoiadb.isCollectionSpaceExist( csName ) ) {
                sequoiadb.dropCollectionSpace( csName ) ;
            }
            sequoiadb.createCollectionSpace( csName ) ;

            File workDirFile = new File( workDir ) ;
            if ( !workDirFile.exists() ) {
                workDirFile.mkdir() ;
            }
            
            try {
                Thread.sleep( 500000 ) ;
            } catch ( InterruptedException e ) {
                // TODO Auto-generated catch block
                e.printStackTrace();
            }
        } catch ( BaseException e ) {
            e.printStackTrace() ;
            throw new SkipException( "initSuite failed" ) ;
        }finally{
            sequoiadb.close() ;
        }
    }

    public static void restartAllDataGroup(Sequoiadb sdb) {
        final String GROUPID = "GroupID" ;
        final int COORDGROUPID = 2 ;
        final String GT = "$gt" ;

        BasicBSONObject cond = new BasicBSONObject() ;
        BasicBSONObject subCond = new BasicBSONObject() ;
        subCond.put( GT, COORDGROUPID ) ;
        cond.put( GROUPID, subCond ) ;
        DBCursor cursor = sdb.getList( Sequoiadb.SDB_LIST_GROUPS, cond,
                null, null ) ;
        while ( cursor.hasNext() ) {
            BasicBSONObject doc = ( BasicBSONObject ) cursor.getNext() ;
            sdb.getReplicaGroup( doc.getInt( GROUPID ) ).stop() ;
            sdb.getReplicaGroup( doc.getInt( GROUPID ) ).start() ;
            
            isGroupNormal( doc ) ;
        }
    }
    
    public static void isGroupNormal( BasicBSONObject groupInfo ) {
        boolean hasPrimary = false ;
        boolean isOk = true ;
        long prevLsn = -1 ;
        int prevVersion = -1 ;
        int totalTimeLen = 6000 ;
        int alreadySleepTime = 0 ;
        
        BasicBSONList nodes = ( BasicBSONList ) groupInfo.get( "Group" ) ;
        BSONObject nullObj = null ;
        
        do {
            for ( int pos = 0; pos < nodes.size(); ++pos ) {
                BasicBSONObject node = ( BasicBSONObject ) nodes.get( pos ) ;
                
                String srvHost = node.getString( "HostName" ) ;
                String    srvName = ( ( BasicBSONObject ) ( ( BasicBSONList ) node
                        .get( "Service" ) ).get( 0 ) ).getString( "Name" ) ;
                
                String url = srvHost + ":" + srvName ;
                try ( Sequoiadb dataDb = new Sequoiadb( url, "", "" ) ) {
                    DBCursor cursor = dataDb.getSnapshot(
                            Sequoiadb.SDB_SNAP_DATABASE, nullObj, nullObj,
                            nullObj ) ;
                    while ( cursor.hasNext() ) {
                        BasicBSONObject doc = ( BasicBSONObject ) cursor
                                .getNext() ;
                        if ( doc.getBoolean( "IsPrimary" ) ) {
                            hasPrimary = true ;
                        }

                        if ( prevLsn == -1 ) {
                            prevLsn = doc.getLong( "CompleteLSN" ) ;
                        }

                        int curVersion = ( ( BasicBSONObject ) doc
                                .get( "CurrentLSN" ) ).getInt( "Version" ) ;
                        if ( prevVersion == -1 ) {
                            prevVersion = curVersion ;
                            continue ;
                        }

                        if ( prevLsn != doc.getLong( "CompleteLSN" )
                                || prevVersion != curVersion ) {
                            isOk = false ;
                        }
                    }
                    cursor.close() ;
                }

                if ( !isOk ) {
                    break ;
                }
            }

            if ( (hasPrimary && isOk)) {
                break ;
            } else if (alreadySleepTime > totalTimeLen){
                throw new SkipException("group restart restore failed!!!!") ;
            }
            else {
                try {
                    Thread.sleep( 100 ) ;
                    alreadySleepTime += 1;
                } catch ( InterruptedException e ) {
                    // TODO Auto-generated catch block
                    e.printStackTrace() ;
                }
            }
        } while ( true ) ;
    }

    private static void modifyNodeConf( boolean transactionon,
            int transisolation, boolean translockwait, int indexscanstep ) {
        BasicBSONObject configs = new BasicBSONObject() ;
        configs.append( TRANSACTIONON, transactionon ) ;
        configs.append( TRANSISOLATION, transisolation ) ;
        configs.append( TRANSLOCKWAIT, translockwait ) ;
        configs.append( INDEXSCANSTEP, indexscanstep ) ;
        
        BasicBSONObject options = new BasicBSONObject() ;
        options.put( ROLE, DATA ) ;
        
        Sequoiadb sdb = null ;
        try {
            sdb = new Sequoiadb( SdbTestBase.coordUrl, "", "" ) ;
            sdb.updateConfig( configs, options ) ;
        } catch ( BaseException e ) {
            if ( CommLib.isStandAlone( sdb ) 
                 &&  e.getErrorCode() != SDBError.SDB_RTN_CONF_NOT_TAKE_EFFECT
                 .getErrorCode() ){
                e.printStackTrace() ;
                throw e ;
            }else if ( e.getErrorCode() != SDBError.SDB_COORD_NOT_ALL_DONE
                    .getErrorCode() ) {
                e.printStackTrace() ;
                throw e ;
            }
            
            if ( CommLib.isStandAlone( sdb )){
                restartStandAlone(sdb.getHost()) ;
            }else{
                restartAllDataGroup(sdb) ;
            }
        }finally{
            sdb.close() ;
        }
    }

    @Parameters( { "TRANSACTIONON", "TRANSISOLATION", "TRANSLOCKWAIT" } )
    @BeforeTest( enabled = false )
    public static void initTest( boolean transactionon, int transisolation,
            boolean translockwait ) {
        if ( !transactionon ) {
            return ;
        }

        try {
            modifyNodeConf( transactionon, transisolation, translockwait, 3 ) ;
        } catch ( BaseException e ) {
            e.printStackTrace() ;
            throw new SkipException( "initTest failed!!!" ) ;
        }
    }

    @Parameters( { "TRANSACTIONON" } )
    @AfterTest( enabled = false )
    public static void finiTest( boolean transactionon ) {
        if ( !transactionon ) {
            return ;
        }

        try {
            modifyNodeConf( !transactionon, 0, false, 3 ) ;
        } catch ( BaseException e ) {
            e.printStackTrace() ;
            throw new SkipException( "initTest failed!!!" ) ;
        }
    }
    
    public static void restartStandAlone(String hostIp){
            if ( System.getProperty("os.name").contains("Windows")){
                return ;
            }
            try{
                List< String > cmdLine = new ArrayList< String >() ; 
                cmdLine.add( "ssh" ) ;
                cmdLine.add("root@" + hostIp) ;
                cmdLine.add( "service sdbcm stop && service sdbcm start") ;
                
                ProcessBuilder pb = new ProcessBuilder( cmdLine ) ;
                pb.redirectError( ProcessBuilder.Redirect.INHERIT ) ;

                Process startProc = pb.start() ;  
                BufferedReader input = new BufferedReader( new InputStreamReader( startProc.getInputStream() ) );
                String line = "";
                while( (line = input.readLine()) != null ){
                    System.out.println(line);
                }
             
                int exitValue = startProc.waitFor();
                if( 0 != exitValue ){
                    System.out.println( "fail to restart sdbcm, return code=" + exitValue );
                }
                
                input.close() ;
            }
            catch( InterruptedException | IOException e ){
                e.printStackTrace();
                System.out.println( "fail to restart sdbcm" );
            }
    }

    @Parameters( { "TRANSACTIONON" } )
    @BeforeGroups( groups = "ru", alwaysRun = true )
    public static void initRuGroups() {
        System.out.println("initRuGroups...........");
        int transisolation = 0 ;
        boolean translockwait = false ;
        try {
            modifyNodeConf( true, transisolation, translockwait, newIndexScanStep ) ;
        } catch ( BaseException e ) {
            e.printStackTrace() ;
            throw new SkipException( "initGroups failed!!!" ) ;
        }
    }

    @BeforeGroups( groups = "rc", alwaysRun = true )
    public static void initRcGroups() {
        System.out.println("initRcGroups...........");
        int transisolation = 1 ;
        boolean translockwait = false ;
        try {
            modifyNodeConf( true, transisolation, translockwait, newIndexScanStep ) ;
        } catch ( BaseException e ) {
            e.printStackTrace() ;
            throw new SkipException( "initGroups failed!!!" ) ;
        }
    }

    @BeforeGroups( groups = "rcwaitlock", alwaysRun = true)
    public static void initRcLockwaitGroups() {
        System.out.println("initRcLockwaitGroups...........");
        int transisolation = 1 ;
        boolean translockwait = true ;
        try {
            modifyNodeConf( true, transisolation, translockwait, newIndexScanStep ) ;
        } catch ( BaseException e ) {
            e.printStackTrace() ;
            throw new SkipException( "initGroups failed!!!" ) ;
        }
    }

    @AfterGroups( groups = { "ru", "rc", "rcwaitlock" }, alwaysRun = true )
    public static void finiGroups() {
        System.out.println("finiGroups...........");
        try {
            modifyNodeConf( false, 0, false, originalIndexScanStep ) ;
        } catch ( BaseException e ) {
            e.printStackTrace() ;
            throw e ;
        }
    }

    @AfterSuite( alwaysRun = true )
    public static void finiSuite() {
        count.getAndIncrement() ;
        try {
            sequoiadb = new Sequoiadb( SdbTestBase.coordUrl, "", "", options ) ;
            if ( sequoiadb.isCollectionSpaceExist( csName ) ) {
                sequoiadb.dropCollectionSpace( csName ) ;
            }
            // sdb.close() ;
        } catch ( BaseException e ) {
            e.printStackTrace() ;
            Assert.fail( e.getMessage() + " called: " + count.get() ) ;
        } finally {
            if ( sequoiadb != null ) {
                sequoiadb.close() ;
            }

            SdbThreadBase.shutdown() ;
        }
    }

    public static String getDefaultCoordUrl() {
        return coordUrl ;
    }

    public static String getWorkDir() {
        return workDir ;
    }
}
