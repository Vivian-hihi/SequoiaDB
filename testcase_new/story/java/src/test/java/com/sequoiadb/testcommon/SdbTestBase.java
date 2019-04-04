package com.sequoiadb.testcommon ;

import java.io.BufferedReader ;
import java.io.File ;
import java.io.FileInputStream ;
import java.io.FileWriter ;
import java.io.IOException ;
import java.io.InputStream ;
import java.io.InputStreamReader ;
import java.util.Arrays ;
import java.util.Properties ;
import java.util.concurrent.atomic.AtomicInteger ;

import org.bson.BSONObject ;
import org.bson.BasicBSONObject ;
import org.bson.types.BasicBSONList ;
import org.testng.annotations.AfterGroups ;
import org.testng.annotations.AfterSuite ;
import org.testng.annotations.AfterTest ;
import org.testng.annotations.BeforeGroups ;
import org.testng.annotations.BeforeSuite ;
import org.testng.annotations.BeforeTest ;
import org.testng.annotations.Optional ;
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
    private static String confToolScript ;
    private static String enableTransaction ;
    private static Sequoiadb sequoiadb = null ;
    private static final String ROLE = "Role" ;
    private static final String DATA = "data" ;
    private static final String TRANSACTIONON = "transactionon" ;
    private static final String TRANSISOLATION = "transisolation" ;
    private static final String TRANSLOCKWAIT = "translockwait" ;
    private static final String INDEXSCANSTEP = "indexscanstep" ;
    private static final String TRANSTIMEOUT = "transactiontimeout" ;

    private static AtomicInteger rcCnt = new AtomicInteger( 0 ) ;
    private static AtomicInteger ruCnt = new AtomicInteger( 0 ) ;
    private static AtomicInteger rcLockCnt = new AtomicInteger( 0 ) ;
    private static AtomicInteger rsLockCnt = new AtomicInteger( 0 ) ;
    private static ConfigOptions options = new ConfigOptions() ;

    private static AtomicInteger runCaseNum = new AtomicInteger(0) ;
    private static final int newIndexScanStep = 3 ;
    private static final int originalIndexScanStep = 100 ;
    private static final int timeOutLen = 60 ;

    @Parameters( { "HOSTNAME", "SVCNAME", "CHANGEDPREFIX", "RSRVPORTBEGIN",
            "RSRVPORTEND", "RSRVNODEDIR", "WORKDIR", "CONFTOOL", "ENABLETRANSACTION" } )
    @BeforeSuite( alwaysRun = true )
    public static void initSuite( String HOSTNAME, String SVCNAME,
            String COMMCSNAME, int RSRVPORTBEGIN, int RSRVPORTEND,
            String RSRVNODEDIR, String WORKDIR, @Optional( "" ) String CONFTOOL,
            @Optional( "false" ) String ENABLETRANSACTION ) {
        System.out.println( "initSuite....." ) ;
        hostName = HOSTNAME ;
        serviceName = SVCNAME ;
        csName = COMMCSNAME ;
        reservedPortBegin = RSRVPORTBEGIN ;
        reservedPortEnd = RSRVPORTEND ;
        reservedDir = RSRVNODEDIR ;
        workDir = WORKDIR ;
        coordUrl = HOSTNAME + ":" + SVCNAME ;
        confToolScript = CONFTOOL ;
        enableTransaction = ENABLETRANSACTION ;
        try {
            if ( !enableTransaction.equals(  "false" ) ){
                modifyNodeConfAndRestart( true, "before" ) ;
            }
            
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

        } catch ( BaseException e ) {
            e.printStackTrace() ;
            throw new RuntimeException( "initSuite failed" ) ;
        } finally {
            sequoiadb.close() ;
        }
    }

    private static BSONObject buildNodeConf( boolean transactionon) {
        BasicBSONObject configs = new BasicBSONObject() ;
        configs.append( TRANSACTIONON, transactionon ) ;
        return configs ;
    }
    
    public static void incCaseNum(){
        runCaseNum.incrementAndGet() ;
    }
    
    public static void decCaseNum(){
        runCaseNum.decrementAndGet() ;
    }
    
    private static BSONObject buildNodeConf( int transisolation, boolean translockwait, int indexscanstep ){
        BasicBSONObject configs = new BasicBSONObject() ;
        configs.append( TRANSISOLATION, transisolation ) ;
        configs.append( TRANSLOCKWAIT, translockwait ) ;
        configs.append( INDEXSCANSTEP, indexscanstep ) ;
        configs.append( TRANSTIMEOUT, timeOutLen ) ;
        return configs ;
    }

    private static void modifyNodeConfAndRestart( boolean transactionon,
            String mode ) {
        BSONObject defaultConf = new BasicBSONObject() ;
        BSONObject dataConf = buildNodeConf( transactionon ) ;
        BSONObject stdalnConf = buildNodeConf( transactionon ) ;
        try {
            createConfFile( defaultConf, defaultConf, defaultConf, defaultConf,
                    dataConf, defaultConf, stdalnConf, defaultConf ) ;
        } catch ( IOException e1 ) {
            e1.printStackTrace() ;
            throw new RuntimeException( "initGroups failed!!!" ) ;
        }

        String[] cmd ;
        try {
            cmd = getConfCmd( mode, confToolScript ) ;
            if ( !execCmd( cmd ) ) {
                throw new RuntimeException(
                        "exec script failed, initGroups failed!!!" ) ;
            }
        } catch ( IOException | InterruptedException e ) {
            e.printStackTrace() ;
            throw new RuntimeException( "initGroups failed!!!" ) ;
        }
    }

    private static void modifyNodeConf( int transisolation, boolean translockwait, int indexscanstep ){
        BSONObject cfg = buildNodeConf( transisolation, translockwait, indexscanstep ) ;
        BSONObject opt = new BasicBSONObject().append( ROLE, DATA ) ;
        try ( Sequoiadb sdb = new Sequoiadb( SdbTestBase.coordUrl, "", "", options )){
            sdb.updateConfig( cfg, opt ) ;
        }catch( BaseException e ){
            e.printStackTrace() ;
            throw e ;
        }
    }
    
    @BeforeGroups( groups = "ru" )
    public static synchronized void initRuGroups() {
        System.out.println( "initRuGroups..........." ) ;
        if ( ruCnt.getAndIncrement() > 0 ) {
            return ;
        }

        modifyNodeConf( 0, false, newIndexScanStep ) ;
    }

    @BeforeGroups( groups = "rc" )
    public static synchronized void initRcGroups() {
        System.out.println( "initRcGroups..........." ) ;
        if ( rcCnt.getAndIncrement() > 0 ) {
            return ;
        }

        modifyNodeConf( 1, false, newIndexScanStep ) ;
    }

    @BeforeGroups( groups = "rcwaitlock" )
    public static synchronized void initRcLockwaitGroups() {
        System.out.println( "initRcLockwaitGroups..........." ) ;
        if ( rcLockCnt.getAndIncrement() > 0 ) {
            return ;
        }

        modifyNodeConf( 1, true, newIndexScanStep ) ;
    }
    
    @BeforeGroups( groups = "rs" )
    public static synchronized void initRsGroups() {
        System.out.println( "initRsGroups..........." ) ;
        if ( rsLockCnt.getAndIncrement() > 0 ) {
            return ;
        }

        modifyNodeConf( 2, false, newIndexScanStep ) ;
    }

    @AfterGroups(groups = {"rc", "ru", "rcwaitlock", "rswaitlock"}, alwaysRun = true )
    public static synchronized void finiGroups() {
        if ( runCaseNum.get() != 0){
            return ;
        }
        System.out.println( "finiGroups..........." ) ;
        modifyNodeConf( 0, false, originalIndexScanStep ) ;
    }
    
    @AfterSuite( alwaysRun = true )
    public static void finiSuite() {
        try {
            if ( !enableTransaction.equals(  "false" ) ){
                modifyNodeConfAndRestart( false, "after" ) ;
            }
            sequoiadb = new Sequoiadb( SdbTestBase.coordUrl, "", "", options ) ;
            if ( sequoiadb.isCollectionSpaceExist( csName ) ) {
                sequoiadb.dropCollectionSpace( csName ) ;
            }
            // sdb.close() ;
        } catch ( BaseException e ) {
            e.printStackTrace() ;
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

    private static boolean execCmd( String[] cmd ) throws IOException,
            InterruptedException {
        System.out.println( "cmd:" + Arrays.toString( cmd ) ) ;
        Process process = Runtime.getRuntime().exec( cmd ) ;

        BufferedReader input = new BufferedReader( new InputStreamReader(
                process.getInputStream() ) ) ;
        String line = "" ;
        while ( ( line = input.readLine() ) != null ) {
            System.out.println( line ) ;
        }

        int exitValue = process.waitFor() ;
        if ( 0 != exitValue ) {
            System.out
                    .println( "fail to change node configure beforetest, return code="
                            + exitValue ) ;
            return false ;
        } else {
            return true ;
        }
    }

    private static String[] getConfCmd( String mode, String confToolScript )
            throws IOException {
        String[] cmd = new String[ 5 ] ;
        String confFullName = "" ;
        if ( mode.equals( "before" ) ) {
            confFullName = getCurrentClass().getResource( "" ) + "/node.conf" ;
            confFullName = confFullName.substring( 5 ) ;
        } else {
            confFullName = System.getProperty( "user.dir" ) + "/node.conf.ini" ;
        }

        Properties prop = new Properties() ;
        InputStream in = new FileInputStream( new File(
                "/etc/default/sequoiadb" ) ) ;
        prop.load( in ) ;
        String installPath = prop.getProperty( "INSTALL_DIR" ) ;
        String sdbFullName = installPath + "/bin/sdb" ;
        System.out.println( sdbFullName ) ;
        cmd[0] = sdbFullName ;
        cmd[1] = "-f" ;
        cmd[2] = confFullName + "," + confToolScript ;
        cmd[3] = "-e" ;
        cmd[4] = "var hostname='" + hostName + "';" + "var svcname="
                + serviceName + ";" + "var mode='" + mode + "'" ;

        return cmd ;
    }

    private static void createConfFile( BSONObject cataConf,
            BSONObject cataDynaConf, BSONObject coordConf,
            BSONObject coordDynaConf, BSONObject dataConf,
            BSONObject dataDynaConf, BSONObject stdalnConf,
            BSONObject stdalnDynaConf ) throws IOException {
        String confPath = getCurrentClass().getResource( "" ).getPath()
                + "/node.conf" ;
        FileWriter confFile = new FileWriter( confPath ) ;
        addStaticConf( confFile, cataConf, coordConf, dataConf, stdalnConf ) ;
        addDynConf( confFile, cataDynaConf, coordDynaConf, dataDynaConf,
                stdalnDynaConf ) ;
        confFile.flush() ;
        confFile.close() ;
        System.out.println( "create file: " + confPath ) ;
    }

    @SuppressWarnings( "rawtypes" )
    private static final Class getCurrentClass() {
        return new Object() {
            public Class getClassForStatic() {
                return this.getClass() ;
            }
        }.getClassForStatic() ;
    }

    private static void addStaticConf( FileWriter confFile,
            BSONObject cataConf, BSONObject coordConf, BSONObject dataConf,
            BSONObject stdalnConf ) throws IOException {
        confFile.write( "catalogConf = " + cataConf + ";\n" ) ;
        confFile.write( "coordConf = " + coordConf + ";\n" ) ;
        confFile.write( "dataConf = " + dataConf + ";\n" ) ;
        confFile.write( "standaloneConf = " + stdalnConf + ";\n" ) ;
    }

    private static void addDynConf( FileWriter confFile,
            BSONObject cataDynaConf, BSONObject coordDynaConf,
            BSONObject dataDynaConf, BSONObject stdalnDynaConf )
            throws IOException {
        confFile.write( "catalogDynaConf = " + cataDynaConf + ";\n" ) ;
        confFile.write( "coordDynaConf = " + coordDynaConf + ";\n" ) ;
        confFile.write( "dataDynaConf = " + dataDynaConf + ";\n" ) ;
        confFile.write( "standaloneConf = " + stdalnDynaConf + ";\n" ) ;
    }
}
