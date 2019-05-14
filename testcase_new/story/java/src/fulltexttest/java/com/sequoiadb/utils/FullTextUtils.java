package com.sequoiadb.utils;

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

/**
 * 全文索引的公共类，检查方法、其他与DB端和ES内部操作无关的方法均可放于此类
 */
public class FullTextUtils {

    // 插入记录数，所有用例公用此变量
    public static final int INSERT_NUMS = 200000; // insert 20w datas

    /**
     * 检查DB端中普通表或分区表下的全文索引数据是否完全同步到ES端，总共分三层检查：
     *                                           1. 先检查文索引名是否都映射到ES端 
     *                                           2. 再检查ES端全文索引的总记录数是否正确
     *                                           3. 最后检查DB端各个固定集合的最大一条LID记录是否与对应ES端全文索引的dbCOMMITID值一致
     * @param esClient
     * @param db
     * @param csName
     * @param clName
     * @param textIndexName
     * @param expectCount
     * @return void
     * @Author liuxiaoxuan
     * @Date 2018-11-15
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
     * 检查DB端中主子表下的全文索引数据是否完全同步到ES端，总共分三层检查：
     *                                           1. 先检查子表的全文索引名是否都映射到ES端 
     *                                           2. 再检查ES端子表的全文索引总记录数是否正确
     *                                           3. 最后检查DB端各个固定集合的最大一条LID记录是否与对应ES端全文索引的dbCOMMITID值一致
     * @param esClient
     * @param db
     * @param csName
     * @param mainCLName
     * @param textIndexName
     * @param expectCount
     * @return void
     * @Author liuxiaoxuan
     * @Date 2018-11-15
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
     * 检查ES端全文索引是否已被清理
     * @param esClient
     * @param esIndexNames
     * @return void
     * @Author liuxiaoxuan
     * @Date 2018-11-15
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
     * 检查ES端全文索引总记录数是否正确，
     * 若原始集合中包含多个全文索引，则总记录数为所有全文索引记录数的总和
     * @param esClient
     * @param esIndexNames
     * @param expectCount
     * @return void
     * @Author liuxiaoxuan
     * @Date 2018-11-15
     */
    public static void checkCountInES( Client esClient,
            List< String > esIndexNames, int expectCount ) {
        boolean isSync = false;
        int timeout = 3600; // timeout 1h
        int interval = 1; // interval 1s
        int doTimes = 0;
        int actCount = 0;

        while ( doTimes * interval < timeout ) {
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
        Assert.assertTrue( isSync, "check " + msg + " count syn to es timeout" );
    }

    /**
     * 检查DB端各个固定集合的最大一条LID记录是否与对应ES端全文索引的dbCOMMITID值一致，
     * 一个全文索引对应一个固定集合
     * @param esClient
     * @param esIndexNames
     * @param cappedCLs
     * @return void
     * @Author liuxiaoxuan
     * @Date 2018-11-15
     */
    public static void checkLidInES( Client esClient,
            List< String > esIndexNames, List< DBCollection > cappedCLs ) {
        boolean isSync = false;
        int timeout = 3600; // timeout 1h
        int interval = 1; // interval 1s
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
            while ( doTimes * interval < timeout ) {
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
            // print message while not finish sync
            Assert.assertTrue( isSync,
                    "check " + esIndexNames.get( i ).toString() + " lid syn to es timeout" );
        }
    }

    /**
     * 数组元素去重
     * @param arrayList
     * @return List< String >
     * @Author liuxiaoxuan
     * @Date 2018-11-15
     */
    public static List< String > removeDuplicateItems(
            List< String > arrayList ) {
        HashSet<String> uniqueSet = new HashSet<String>( arrayList );
        arrayList.clear();
        arrayList.addAll( uniqueSet );
        return arrayList;
    }

    /**
     * 检查主备节点数据一致性
     * @param db
     * @param csName
     * @param clName
     * @return void
     * @Author liuxiaoxuan
     * @Date 2019-05-09
     */
    public static void checkDataConsistency( Sequoiadb db, String csName,
            String clName, String textIndexName ) {            
        // check CL consistency
        checkCLDataConsistency( db, csName, clName );
        // check cappedCL consistency
        checkCappedCLDataConsistency( db, csName, clName, textIndexName);
    }
    
    /**
     * 检查主备节点的普通表、分区表数据一致性
     * @param db
     * @param csName
     * @param clName
     * @return void
     * @Author yinzhen
     * @Date 2018-12-21
     */
    public static void checkCLDataConsistency( Sequoiadb db, String csName,
            String clName ) {
        boolean isConsistency = false;
        
        List< String > groupNames = FullTextDBUtils.getCLGroups( db,
                csName + "." + clName );
        // in case of duplicate groupNames
        groupNames = removeDuplicateItems( groupNames );
        
        for ( String groupName: groupNames ) {
            List< String > nodeNames = CommLib.getNodeAddress( db, groupName );
            List< Node > nodes = new ArrayList<  >();
            for ( String nodeName : nodeNames ) {
                nodes.add(db.getReplicaGroup( groupName ) .getNode( nodeName ) );                           
            }
            isConsistency = isConsistency( nodes, csName, clName );
            Assert.assertTrue( isConsistency, "check cl consistency timeout" );
        }
    }
    
    /**
     * 检查主备节点的固定集合数据一致性
     * @param db
     * @param csName
     * @param clName
     * @return void
     * @Author liuxiaoxuan
     * @Date 2019-05-09
     */
    public static void checkCappedCLDataConsistency( Sequoiadb db, String csName,
            String clName, String textIndexName ) {
        boolean isConsistency = false;
        
        String cappedName = FullTextDBUtils.getCappedName(db, csName, clName, textIndexName);
        List< String > groupNames = FullTextDBUtils.getCLGroups( db,
                csName + "." + clName );
        // in case of duplicate groupNames
        groupNames = removeDuplicateItems( groupNames );
        for ( String groupName: groupNames ) {
            List< String > nodeNames = CommLib.getNodeAddress( db, groupName );
            List< Node > nodes = new ArrayList<  >();
            for ( String nodeName : nodeNames ) {
                nodes.add(db.getReplicaGroup( groupName ) .getNode( nodeName ) );                           
            }
            isConsistency = isConsistency( nodes, cappedName, cappedName );
            Assert.assertTrue( isConsistency, "check cappedcl consistency timeout" );
        }      
    }

    /**
     * 检查主备节点的主子表数据一致性
     * @param db
     * @param mainclFullName
     * @return void
     * @Author yinzhen
     * @Date 2018-12-21
     */
    public static void checkMainCLDataConsistency( Sequoiadb db,
            String mainclFullName, String textIndexName ) {
        List< String > subclNames = FullTextDBUtils.getSubCLNames( db,
                mainclFullName );
        for ( int i = 0; i < subclNames.size(); i++ ) {
            String subcsName = subclNames.get( i ).split( "\\." )[ 0 ];
            String subclName = subclNames.get( i ).split( "\\." )[ 1 ];
            checkDataConsistency( db, subcsName, subclName, textIndexName );
        }
    }

    /**
     * 判断主备节点的集合数据是否一致
     * @param nodes
     * @param csName
     * @param clName
     * @return boolean
     * @Author yinzhen
     * @Date 2018-12-21
     */
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

    /**
     * 判断主备节点的集合是否存在
     * @param nodes
     * @param csName
     * @param clName
     * @return boolean
     * @Author yinzhen
     * @Date 2019-04-13
     */
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
    
    /**
     * 判断主备节点的集合数据是否一致
     * @param nodes
     * @param csName
     * @param clName
     * @return boolean
     * @Author yinzhen
     * @Date 2018-12-21
     */
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

    /**
     * 判断主备节点的集合数据是否一致
     * @param cl1Cursor
     * @param cl2Cursor
     * @return boolean
     * @Author yinzhen
     * @Date 2018-12-21
     */
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
