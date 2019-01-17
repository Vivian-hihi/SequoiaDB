package com.sequoiadb.testcommon;

import java.io.File;

import org.bson.BSONObject ;
import org.bson.BasicBSONObject ;
import org.testng.SkipException ;
import org.testng.annotations.AfterGroups ;
import org.testng.annotations.AfterSuite;
import org.testng.annotations.AfterTest ;
import org.testng.annotations.BeforeGroups ;
import org.testng.annotations.BeforeSuite;
import org.testng.annotations.BeforeTest ;
import org.testng.annotations.Parameters;

import com.sequoiadb.base.DBCursor ;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.exception.BaseException ;
import com.sequoiadb.exception.SDBError ;


public class SdbTestBase {
    protected static String coordUrl;
    protected static String hostName;
    protected static String serviceName;
    protected static String csName;
    protected static int reservedPortBegin;
    protected static int reservedPortEnd;
    protected static String reservedDir;
    protected static String workDir;
    private static Sequoiadb sdb = null;
    private static final String ROLE = "Role" ;
    private static final String DATA = "data" ;
    private static final String TRANSACTIONON  = "transactionon" ;
    private static final String TRANSISOLATION = "transisolation" ;
    private static final String TRANSLOCKWAIT  = "translockwait" ;

    @Parameters({"HOSTNAME", "SVCNAME", "CHANGEDPREFIX",
            "RSRVPORTBEGIN", "RSRVPORTEND", "RSRVNODEDIR", "WORKDIR"})
    @BeforeSuite(groups={"ru", "rc", "rcwaitlock"},inheritGroups = true)
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
        
        try {
            sdb = new Sequoiadb(coordUrl, "", "");
            if (sdb.isCollectionSpaceExist(csName)){ 
                sdb.dropCollectionSpace(csName);
            }
            sdb.createCollectionSpace(csName);
            
            File workDirFile = new File(workDir);
            if (!workDirFile.exists()) {
                workDirFile.mkdir();
            }
        } catch(BaseException e){
            e.printStackTrace() ;
            throw new SkipException("initSuite failed") ;
        }
    }
    
    public static void restartAllDataGroup(){
        final String GROUPID = "GroupID" ;
        final int COORDGROUPID = 2 ;
        final String GT = "$gt" ;
        
        if (sdb == null || sdb.isClosed()){
            sdb = new Sequoiadb( coordUrl, "", "") ;
        }
        
        BasicBSONObject cond = new BasicBSONObject() ;
        BasicBSONObject subCond = new BasicBSONObject() ;
        subCond.put( GT, COORDGROUPID );
        cond.put( GROUPID, subCond ) ;
        DBCursor cursor = sdb.getList( Sequoiadb.SDB_LIST_GROUPS , cond, null, null ) ;
        while ( cursor.hasNext() ){
            BasicBSONObject doc = ( BasicBSONObject ) cursor.getNext() ;
            sdb.getReplicaGroup( doc.getInt( GROUPID ) ).stop();
            sdb.getReplicaGroup( doc.getInt( GROUPID ) ).start();
        }
    }
   
    private static void modifyNodeConf(boolean transactionon, 
                                       int transisolation, 
                                       int translockwait ){
        BasicBSONObject configs  = new BasicBSONObject() ;
        configs.append( TRANSACTIONON,  true ) ;
        configs.append( TRANSISOLATION, transisolation ) ;
        configs.append( TRANSLOCKWAIT, translockwait ) ;
        BasicBSONObject options = new BasicBSONObject() ;
        options.put( ROLE, DATA ) ; 
        
        try{
            sdb.updateConfig( configs, options ) ;
        }catch(BaseException e){
            if ( e.getErrorCode() != SDBError.SDB_COORD_NOT_ALL_DONE.getErrorCode()){
                e.printStackTrace() ;
                throw e ;
            }
        }
        restartAllDataGroup() ;
    }
    
    @Parameters({"TRANSACTIONON", "TRANSISOLATION", "TRANSLOCKWAIT"})
    @BeforeTest(enabled=false)
    public static void initTest(boolean transactionon, 
                                int transisolation, 
                                int translockwait){
        if ( !transactionon ){
            return ;
        }
        
        try{
            modifyNodeConf(transactionon, transisolation, translockwait ) ;
        }catch(BaseException e){
            e.printStackTrace() ;
            throw new SkipException("initTest failed!!!") ;
        }
    }
    
    @Parameters({"TRANSACTIONON"})
    @AfterTest(enabled=false)
    public static void finiTest(boolean transactionon){
        if ( !transactionon ){
            return ;
        }
        
        try{
            modifyNodeConf(!transactionon, 0, 0 ) ;
        }catch(BaseException e){
            e.printStackTrace() ;
            throw new SkipException("initTest failed!!!") ;
        }
    }
    
    
    @Parameters({"TRANSACTIONON"})
    @BeforeGroups(groups = "ru", inheritGroups = true )
    public static void initRuGroups( boolean transactionon){
        if ( !transactionon ){
            return ;
        }
        
        int transisolation = 0 ; 
        int translockwait = 0;
        try{
            modifyNodeConf(transactionon, transisolation, translockwait ) ;
        }catch(BaseException e){
            e.printStackTrace() ;
            throw new SkipException("initGroups failed!!!") ;
        }
    }
    
    @Parameters({"TRANSACTIONON"})
    @BeforeGroups(groups = "rc", inheritGroups = true)
    public static void initRcGroups( boolean transactionon){
        if ( !transactionon ){
            return ;
        }
        
        int transisolation = 1 ; 
        int translockwait = 0;
        try{
            modifyNodeConf(transactionon, transisolation, translockwait ) ;
        }catch(BaseException e){
            e.printStackTrace() ;
            throw new SkipException("initGroups failed!!!") ;
        }
    }
    
    @Parameters({"TRANSACTIONON"})
    @BeforeGroups(groups = "rcwaitlock", inheritGroups = true)
    public static void initRcLockwaitGroups( boolean transactionon){
        if ( !transactionon ){
            return ;
        }
        
        int transisolation = 1 ; 
        int translockwait = 1;
        try{
            modifyNodeConf(transactionon, transisolation, translockwait ) ;
        }catch(BaseException e){
            e.printStackTrace() ;
            throw new SkipException("initGroups failed!!!") ;
        }
    }
    
    @Parameters({"TRANSACTIONON"})
    @AfterGroups(groups = {"ru", "rc", "rcwaitlock"},inheritGroups = true)
    public static void finiGroups( boolean transactionon ){
        if ( !transactionon ){
            return ;
        }
        
        try{
            modifyNodeConf(!transactionon, 0, 0);
        }catch(BaseException e){
            e.printStackTrace() ;
            throw e ;
        }
    }

    @AfterSuite(groups={"ru", "rc", "rcwaitlock"},inheritGroups = true)
    public static void finiSuite() {
        try {
            if ( sdb.isClosed() ){
                sdb = new Sequoiadb(coordUrl, "", "");
            }
            
            if (sdb.isCollectionSpaceExist(csName)) {
                sdb.dropCollectionSpace(csName);
            }
        } finally {
            if (sdb != null) {
                sdb.close();
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
