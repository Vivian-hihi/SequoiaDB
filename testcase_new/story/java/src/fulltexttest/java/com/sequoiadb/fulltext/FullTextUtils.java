package com.sequoiadb.fulltext;

import java.util.ArrayList;
import java.util.List;
import java.util.HashSet;
import java.util.Random;
import org.testng.Assert;

import com.sequoiadb.base.*;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.testcommon.CommLib;
import org.bson.BSONObject;
import org.elasticsearch.client.*;

public class FullTextUtils {

    public static final int INSERT_NUMS = 200000; // insert 20w datas

    /**
     * @param esClient
     * @param db
     * @param csName
     * @param clName
     * @param textIndexName
     * @param expectCount
     */
    public static void checkFullSyncToES( Client esClient, Sequoiadb db,
            String csName, String clName, String textIndexName,
            int expectCount ) {
        List< String > esIndexNames = FullTextDBUtils.getESIndexNames( db,
                csName, clName, textIndexName );
        List< DBCollection > cappedCLs = FullTextDBUtils.getCappedCLs( db,
                csName, clName, textIndexName );

        // check indexnames sync to ES
        for ( String esIndexName : esIndexNames ) {
            String msg = esIndexName + " is not exist";
            Assert.assertTrue(
                    FullTextESUtils.isExistIndexInES( esClient, esIndexName ),
                    msg );
        }

        // check all indices sync to ES
        checkCountInES( esClient, esIndexNames, expectCount );
        checkLidInES( esClient, esIndexNames, cappedCLs );
    }

    /**
     * @param esClient
     * @param db
     * @param csName
     * @param mainCLName
     * @param textIndexName
     * @param expectCount
     */
    public static void checkMainCLFullSyncToES( Client esClient, Sequoiadb db,
            String csName, String mainCLName, String textIndexName,
            int expectCount ) {
        List< String > subCLFullNames = FullTextDBUtils.getSubCLNames( db,
                csName + "." + mainCLName );

        // get all indexs from maincl
        List< String > esIndexNames = new ArrayList<>();
        List< DBCollection > cappedCLs = new ArrayList<>();
        for ( String subCLFullName : subCLFullNames ) {
            String subCSName = subCLFullName.split( "\\." )[ 0 ];
            String subCLName = subCLFullName.split( "\\." )[ 1 ];

            esIndexNames.addAll( FullTextDBUtils.getESIndexNames( db, subCSName,
                    subCLName, textIndexName ) );
            cappedCLs.addAll( FullTextDBUtils.getCappedCLs( db, subCSName,
                    subCLName, textIndexName ) );
        }
        esIndexNames = removeDuplicateItems( esIndexNames );
        // sort esIndexNames
        FullTextDBUtils.compare( esIndexNames );

        // check indexnames sync to ES
        for ( String esIndexName : esIndexNames ) {
            String msg = esIndexName + " is not exist";
            Assert.assertTrue(
                    FullTextESUtils.isExistIndexInES( esClient, esIndexName ),
                    msg );
        }

        // check all indices sync to ES
        checkCountInES( esClient, esIndexNames, expectCount );
        checkLidInES( esClient, esIndexNames, cappedCLs );
    }

    /**
     * @param esClient
     * @param esIndexNames
     */
    public static void checkIndexNotExistInES( Client esClient,
            List< String > esIndexNames ) {
        // check indexnames sync to ES
        for ( String esIndexName : esIndexNames ) {
            String msg = esIndexName + " is exist";
            Assert.assertTrue(
                    FullTextESUtils.isIndexDeletedInES( esClient, esIndexName ),
                    msg );
        }
    }

    /**
     * @param esClient
     * @param esIndexNames
     * @param expectCount
     */
    public static void checkCountInES( Client esClient,
            List< String > esIndexNames, int expectCount ) {
        boolean isSync = false;
        // int timeout = 600;
        // int interval = 1; // interval 1s
        int doTimes = 0;
        int actCount = 0;

        while ( true ) {
            actCount = 0;
            // Add counts of all indices
            for ( String esIndexName : esIndexNames ) {
                actCount += ( FullTextESUtils.getCountFromES( esClient,
                        esIndexName ) - 1 );
            }

            // if expect count < act count, exit
            if ( actCount == expectCount ) {
                isSync = true;
                break;
            } else {
                doTimes++;
                System.out.println( "esIndexNames: " + esIndexNames.toString()
                        + ", doTimes: " + doTimes + ", actCount: " + actCount
                        + ", expectCount: " + expectCount );
                try {
                    Thread.sleep( 1000 );
                } catch ( InterruptedException e ) {
                    e.printStackTrace();
                }
            }
        }
        // print message while not finish sync
        String msg = "";
        for ( String esIndexName : esIndexNames ) {
            msg += esIndexName + "/";
        }
        Assert.assertTrue( isSync, "check " + msg + " count syn to es fail" );
    }

    /**
     * @param esClient
     * @param esIndexNames
     * @param cappedCLs
     */
    public static void checkLidInES( Client esClient,
            List< String > esIndexNames, List< DBCollection > cappedCLs ) {
        boolean isSync = false;
        // int timeout = 600;
        // int interval = 1; // interval 1s
        int doTimes;

        // get all lids from all groups
        List< Integer > lastLogicalIDs = new ArrayList<>();
        for ( DBCollection cappedCL : cappedCLs ) {
            lastLogicalIDs.add( FullTextDBUtils.getLastLid( cappedCL ) );
        }

        // check out each fulltext
        for ( int i = 0; i < esIndexNames.size(); i++ ) {
            doTimes = 0;
            Integer commitID = -10000;
            while ( true ) {
                commitID = FullTextESUtils.getCommitIDFromES( esClient,
                        esIndexNames.get( i ) );
                if ( commitID.intValue() != lastLogicalIDs.get( i )
                        .intValue() ) {
                    isSync = false;
                } else {
                    isSync = true;
                }

                if ( isSync ) {
                    break;
                } else {
                    doTimes++;
                    System.out.println(
                            "esIndexName: " + esIndexNames.get( i ).toString()
                                    + ", doTimes: " + doTimes + ", commitID: "
                                    + commitID + ", lastLogicalID: "
                                    + lastLogicalIDs.get( i ).toString() );
                    try {
                        Thread.sleep( 1000 );
                    } catch ( InterruptedException e ) {
                        e.printStackTrace();
                    }
                }
            }
        }

        // print message while not finish sync
        Assert.assertTrue( isSync,
                "check " + esIndexNames.toString() + " lid syn to es fail" );
    }

    /**
     * @param arrayList
     */
    public static List< String > removeDuplicateItems(
            List< String > arrayList ) {
        HashSet<String> uniqueSet = new HashSet<String>( arrayList );
        arrayList.clear();
        arrayList.addAll( uniqueSet );
        return arrayList;
    }

    /**
     * @param length
     */
    public static String getRandomString( int length ) {
        String base = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789@#$%^&*()!";
        StringBuffer sb = new StringBuffer();
        for ( int i = 0; i < length; i++ ) {
            char randomChar = base
                    .charAt( new Random().nextInt( base.length() ) );
            sb.append( randomChar );
        }
        return sb.toString();
    }

    public static void checkConsistency( Sequoiadb sdb, String csName,
            String clName ) {
        boolean isConsistency = false;
        List< String > groups = FullTextDBUtils.getCLGroups( sdb,
                csName + "." + clName );
        groups = removeDuplicateItems( groups );
        for ( int i = 0; i < groups.size(); i++ ) {
            List< Node > nodes = new ArrayList< Node >();
            String groupName = groups.get( i );
            List< String > nodeNames = CommLib.getNodeAddress( sdb, groupName );
            for ( String nodeName : nodeNames ) {
                nodes.add(
                        sdb.getReplicaGroup( groupName ).getNode( nodeName ) );
            }
            isConsistency = isConsistency( nodes, csName, clName );
            Assert.assertTrue( isConsistency, "check inspect fail" );
        }
    }

    public static void checkMainCLConsistency( Sequoiadb sdb,
            String mainclFullName ) {
        List< String > subclNames = FullTextDBUtils.getSubCLNames( sdb,
                mainclFullName );
        for ( int i = 0; i < subclNames.size(); i++ ) {
            String subcsName = subclNames.get( i ).split( "\\." )[ 0 ];
            String subclName = subclNames.get( i ).split( "\\." )[ 1 ];
            checkConsistency( sdb, subcsName, subclName );
        }
    }

    public static boolean isConsistency( List< Node > nodes, String csName,
            String clName ) {
        boolean isConsistency = false;
        int doTimes = 0;
        int timeout = 600;
        while ( true ) {
            isConsistency = isNodeRecordsConsistency( nodes, csName, clName );
            if ( isConsistency ) {
                return isConsistency;
            } else {
                doTimes++;
                System.out.println( "csName : " + csName + " clName: " + clName
                        + " isConsistency : " + isConsistency + " , doTimes: "
                        + doTimes );
                try {
                    Thread.sleep( 1000 );
                } catch ( InterruptedException e ) {
                    e.printStackTrace();
                }
            }
            if ( doTimes >= timeout ) {
                break;
            }
        }
        return isConsistency;
    }

    private static boolean isNodeCLExist(List<Node> nodes, String csName, String clName) {
        try {
            for (Node node : nodes) {
                node.connect().getCollectionSpace(csName).getCollection(clName);
            }
        } catch (BaseException e) {
            if (-23 == e.getErrorCode()) {
                return false;
            }
            throw e;
        }
        return true;
    }
    
    public static boolean isNodeRecordsConsistency( List< Node > nodes,
            String csName, String clName ) {
        if ( nodes.size() == 1 ) {
            return true;
        }
        
        // 判断所有节点是否已同步集合
        if(!isNodeCLExist(nodes, csName, clName)) {
            return false;
        }
        
        Sequoiadb firstNode = nodes.get( 0 ).connect();
        DBCollection cl1 = firstNode.getCollectionSpace( csName )
                .getCollection( clName );
        for ( int i = 1; i < nodes.size(); i++ ) {
            Sequoiadb nextNode = nodes.get( i ).connect();
            DBCollection cl2 = nextNode.getCollectionSpace( csName )
                    .getCollection( clName );
            if ( cl1.getCount() != cl2.getCount() ) {
                System.out.println( "cl from " + nodes.get( 0 ).getNodeName()
                        + "'s count: " + cl1.getCount() + ", cl from "
                        + nodes.get( i ).getNodeName() + "'s count: "
                        + cl2.getCount() );
                return false;
            }
            DBCursor cl1Cursor = cl1.query( null, null, "{\"_id\":1}", null );
            DBCursor cl2Cursor = cl2.query( null, null, "{\"_id\":1}", null );
            if ( !isCLRecordsConsistency( cl1Cursor, cl2Cursor ) ) {
                return false;
            }
        }
        return true;
    }

    public static boolean isCLRecordsConsistency( DBCursor cl1Cursor,
            DBCursor cl2Cursor ) {
        try {
            while ( cl1Cursor.hasNext() && cl2Cursor.hasNext() ) {
                BSONObject cl1Record = cl1Cursor.getNext();
                BSONObject cl2Record = cl2Cursor.getNext();
                if ( !cl1Record.equals( cl2Record ) ) {
                    System.out.println( "collection from first node's record : "
                            + cl1Record.toString()
                            + "\n collection from anohter node's record : "
                            + cl2Record.toString() );
                    return false;
                }
            }
        } finally {
            cl1Cursor.close();
            cl2Cursor.close();
        }
        return true;
    }
}
