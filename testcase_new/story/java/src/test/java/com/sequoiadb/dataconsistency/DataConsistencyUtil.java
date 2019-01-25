package com.sequoiadb.dataconsistency ;

import java.util.ArrayList ;

import java.util.List ;
import org.bson.BSONObject ;
import org.bson.BasicBSONObject ;
import org.bson.types.BSONDecimal ;
import org.bson.util.JSON ;
import org.testng.Assert ;

import com.sequoiadb.base.CollectionSpace ;
import com.sequoiadb.base.DBCollection ;
import com.sequoiadb.base.DBCursor ;
import com.sequoiadb.base.Sequoiadb ;
import com.sequoiadb.testcommon.CommLib ;

/**
 * @Description DataConsistencyUtil.java
 * @author wuyan
 * @date 2018.12.28
 */

public class DataConsistencyUtil {
    public static ArrayList< BSONObject > insertDatas( DBCollection dbcl,
            int insertNums, int beginNo ) {
        int batchNums = 10000 ;
        int times = insertNums / batchNums ;
        int remainder = insertNums % batchNums ;
        if ( times == 0 ) {
            times = 1 ;
        }

        ArrayList< BSONObject > insertRecords = new ArrayList< BSONObject >() ;
        for ( int k = 0; k < times; k++ ) {
            if ( remainder != 0 && k == times - 1 ) {
                batchNums = remainder ;
            }

            ArrayList< BSONObject > insertRecord = new ArrayList< BSONObject >() ;
            for ( int i = 0; i < batchNums; i++ ) {
                int count = beginNo++ ;
                BSONObject obj = new BasicBSONObject() ;
                obj.put( "testa", "test" + count ) ;
                String str = "32345.06789123456" + count ;
                BSONDecimal decimal = new BSONDecimal( str ) ;
                obj.put( "decimala", decimal ) ;
                obj.put( "no", count ) ;
                obj.put( "order", count ) ;
                obj.put( "inta", count ) ;
                obj.put( "ftest", count + 0.2345 ) ;
                obj.put( "str", "test_" + String.valueOf( count ) ) ;
               
                insertRecord.add( obj ) ;
                insertRecords.add( obj ) ;
            }
            dbcl.insert( insertRecord ) ;
        }
        return insertRecords ;
    }

    public static void checkDataContent( DBCollection dbcl,
            ArrayList< BSONObject > expRecord ) {
        checkDataContent( dbcl, expRecord, "" ) ;
    }
    
    public static void checkDataContent( DBCollection dbcl,
            List< BSONObject > expRecord, String matcher ) {
        DBCursor cursor = dbcl.query( matcher, "", "{'order':1}", "" ) ;
        
        int count = 0 ;
        while ( cursor.hasNext() ) {
            BSONObject record = cursor.getNext() ;
            Assert.assertEquals( record, expRecord.get( count++ ) ) ;
        }
        cursor.close() ;
        Assert.assertEquals( count, expRecord.size() ) ;
    }

    public static void checkDataConsistency( Sequoiadb sdb, String groupName,
            String csName, String clName, ArrayList< BSONObject > expRecord ) {
        checkDataConsistency( sdb, groupName, csName, clName, expRecord, "" ) ;
    }
    
    public static boolean isLsnConsistency(List< String > nodeUrls){
        boolean isConsistency = true ;
        int eachSleepTime = 1000 ;
        int maxSleetTime = 600000 ;
        int alreadySleepTime = 0 ;
        
        do{
            long lsnOfPrevNode = -1 ;
            int versionOfPrevNode = 0 ;
            for (String nodeUrl: nodeUrls){
                try ( Sequoiadb dataDB = new Sequoiadb( nodeUrl, "", "" ); ) {
                    DBCursor cursor = dataDB.getSnapshot( Sequoiadb.SDB_SNAP_DATABASE, null,
                                                       "{CurrentLSN:null, CompleteLSN:null}", null );
                    while(cursor.hasNext()){
                        BasicBSONObject doc = ( BasicBSONObject ) cursor.getNext() ;
                        long lsnOfCurNode = doc.getLong( "CompleteLSN" )  ;
                        int versionOfCurNode = ((BasicBSONObject)doc.get( "CurrentLSN" )).getInt("Version") ;
                        if ( lsnOfPrevNode == -1 ){
                            lsnOfPrevNode = lsnOfCurNode ;
                            versionOfPrevNode = versionOfCurNode ;
                            continue ;
                        }else if ( lsnOfPrevNode != lsnOfCurNode
                                || versionOfPrevNode != versionOfCurNode ){
                            isConsistency = false ;
                        }
                    }
                    cursor.close() ;
                }
            }
            
            if (isConsistency){
                break ;
            }
            
            if ( alreadySleepTime >= maxSleetTime){
                break ;
            }
            
            try {
                Thread.sleep( eachSleepTime ) ;
                alreadySleepTime += eachSleepTime ;
            } catch ( InterruptedException e ) {
                // TODO Auto-generated catch block
                e.printStackTrace() ;
            }
        }while(true) ;
        
        return isConsistency ;
    }

    public static void checkDataConsistency( Sequoiadb sdb, String groupName,
            String csName, String clName, List< BSONObject > expRecord,
            String matcher ) {
        List< String > nodeInfo = CommLib.getNodeAddress( sdb, groupName ) ;
        if (!isLsnConsistency(nodeInfo)){
            Assert.fail( "lsn consistency fail exceeds maximum waiting time") ;
        }
        
        for ( int i = 0; i < nodeInfo.size(); i++ ) {
            try ( Sequoiadb dataDB = new Sequoiadb( nodeInfo.get( i ), "", "" ); ) {
                
                DBCollection dbcl = dataDB.getCollectionSpace( csName )
                        .getCollection( clName ) ;
                checkDataContent(dbcl, expRecord, matcher) ;
            }
        }
    }

    public static String getGroupName( Sequoiadb sdb ) {
        ArrayList< String > rgNames = CommLib.getDataGroupNames( sdb ) ;
        int serino = ( int ) ( Math.random() * rgNames.size() ) ;
        String groupName = rgNames.get( serino ) ;
        return groupName ;
    }

    public static DBCollection createCL( CollectionSpace cs, String clName,
            String option ) {
        DBCollection cl = null ;
        BSONObject options = ( BSONObject ) JSON.parse( option ) ;
        if ( cs.isCollectionExist( clName ) ) {
            cs.dropCollection( clName ) ;
        }

        cl = cs.createCollection( clName, options ) ;
        return cl ;
    }

    public static DBCollection createCL( CollectionSpace cs, String clName ) {
        return createCL( cs, clName, null ) ;
    }
}
