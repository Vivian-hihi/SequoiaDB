package com.sequoiadb.utils;

import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;
import java.util.HashSet;
import java.util.List;

import org.bson.BSONObject;
import org.elasticsearch.client.Client;
import org.elasticsearch.index.IndexNotFoundException;

import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.DBCursor;
import com.sequoiadb.base.Node;
import com.sequoiadb.base.ReplicaGroup;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.testcommon.CommLib;

/**
 * 全文索引的公共类，检查方法、其他与DB端和ES内部操作无关的方法均可放于此类
 */
public class FullTextUtils {

    // 插入记录数，所有用例公用此变量
    public static final int INSERT_NUMS = 200000; // insert 20w datas

    /**
     * 检查DB端中普通表或分区表下的全文索引数据是否完全同步到ES端，总共分三层检查: 
     * 1.先检查文索引名是否都映射到ES端
     * 2.再检查ES端全文索引的总记录数是否正确 
     * 3.最后检查DB端各个固定集合的最大一条LID记录是否与对应ES端全文索引的SDBCOMMITID值一致
     * 
     * @param esClient
     * @param cl
     * @param textIndexName
     * @param expectCount
     * @return boolean 如果ES端的全文索引完成同步则返回true，否则返回false
     * @throws Exception 
     * @Author liuxiaoxuan
     * @Date 2018-11-15
     */
    private static boolean isFullSyncToES( Client esClient, DBCollection cl, String textIndexName, int expectCount )
            throws Exception {
        List<String> esIndexNames = FullTextDBUtils.getESIndexNames( cl, textIndexName );
        List<DBCollection> cappedCLs = FullTextDBUtils.getCappedCLs( cl, textIndexName );

        // 检查ES端索引名是否存在
        for ( String esIndexName : esIndexNames ) {
            if ( !FullTextESUtils.isExistIndexInES( esClient, esIndexName, true ) ) {
                System.err.println( esIndexName + " is not exist in ES" );
                return false;
            }
        }

        // 检查索引数是否已完全同步到ES
        if ( !isCountRightInES( esClient, esIndexNames, expectCount ) ) {
            return false;
        }
        // 检查固定集合的最后一条lid是否等于ES端SDBCOMMIT._id
        if ( !isLastLidInES( esClient, esIndexNames, cappedCLs ) ) {
            return false;
        }

        return true;
    }

    /**
     * 检查DB端中主子表下的全文索引数据是否完全同步到ES端，总共分三层检查： 
     * 1. 先检查子表的全文索引名是否都映射到ES端 
     * 2. 再检查ES端子表的全文索引总记录数是否正确 
     * 3. 最后检查DB端各个固定集合的最大一条LID记录是否与对应ES端全文索引的dbCOMMITID值一致
     * 
     * @param esClient
     * @param cl
     * @param textIndexName
     * @param expectCount
     * @return boolean 如果主子表在ES端的全文索引完成同步则返回true，否则返回false
     * @throws Exception 
     * @Author liuxiaoxuan
     * @Date 2018-11-15
     */
    private static boolean isMainCLFullSyncToES( Client esClient, DBCollection cl, String textIndexName,
            int expectCount ) throws Exception {
        Sequoiadb db = cl.getSequoiadb();
        List<String> subCLFullNames = FullTextDBUtils.getSubCLNames( db, cl.getFullName() );

        // 获取主表下所有子表的全文索引和固定集合对象
        List<String> esIndexNames = new ArrayList<String>();
        List<DBCollection> cappedCLs = new ArrayList<DBCollection>();
        for ( String subCLFullName : subCLFullNames ) {
            String subCSName = subCLFullName.split( "\\." )[0];
            String subCLName = subCLFullName.split( "\\." )[1];
            DBCollection subCL = db.getCollectionSpace( subCSName ).getCollection( subCLName );
            esIndexNames.addAll( FullTextDBUtils.getESIndexNames( subCL, textIndexName ) );
            cappedCLs.addAll( FullTextDBUtils.getCappedCLs( subCL, textIndexName ) );
        }
        // 索引数组元素去重
        esIndexNames = removeDuplicateItems( esIndexNames );
        // 索引数组元素排序
        Collections.sort( esIndexNames, new Comparator<Object>() {
            @Override
            public int compare( Object o1, Object o2 ) {
                String str1 = (String) o1;
                String str2 = (String) o2;
                if ( str1.compareToIgnoreCase( str2 ) < 0 ) {
                    return -1;
                }
                return 1;
            }
        } );

        // 检查ES端索引名是否存在
        for ( String esIndexName : esIndexNames ) {
            if ( !FullTextESUtils.isExistIndexInES( esClient, esIndexName, true ) ) {
                System.err.println( esIndexName + " is not exist in ES" );
                return false;
            }
        }

        // 检查索引数是否已完全同步到ES
        if ( !isCountRightInES( esClient, esIndexNames, expectCount ) ) {
            return false;
        }
        // 检查固定集合的最后一条lid是否等于ES端SDBCOMMIT._id
        if ( !isLastLidInES( esClient, esIndexNames, cappedCLs ) ) {
            return false;
        }

        return true;
    }

    /**
    * 检查ES端全文索引总记录数是否正确， 若原始集合中包含多个全文索引，则总记录数为所有全文索引记录数的总和
    * 
    * @param esClient
    * @param esIndexNames
    * @param expectCount
    * @return boolean 如果ES端的全文索引记录数正确则返回true，否则返回false
    * @Author liuxiaoxuan
    * @Date 2018-11-15
    */
    public static boolean isCountRightInES( Client esClient, List<String> esIndexNames, int expectCount )
            throws Exception {
        boolean isSync = false;
        int timeout = 3600; // 超时 1h
        int interval = 1; // 每次检测间隔时间1s
        int doTimes = 0;
        int actCount = 0;

        while ( doTimes * interval < timeout ) {
            actCount = 0;
            // 所有索引的记录数总和
            for ( String esIndexName : esIndexNames ) {
                actCount += ( FullTextESUtils.getCountFromES( esClient, esIndexName ) - 1 );
            }

            if ( actCount == expectCount ) {
                isSync = true;
                break;
            } else {
                doTimes++;
                // System.out.println( "esIndexNames: " +
                // esIndexNames.toString() + ", doTimes: " + doTimes
                // + ", actCount: " + actCount + ", expectCount: " + expectCount
                // );
                try {
                    Thread.sleep( 1000 );
                } catch ( InterruptedException e ) {
                    e.printStackTrace();
                }
            }
        }
        // 同步失败后，打印所有索引名
        if ( !isSync ) {
            System.err.println( "check " + esIndexNames.toString() + " count syn to es timeout" );
        }
        return isSync;
    }

    /**
     * 检查DB端各个固定集合的最大一条LID记录是否与对应ES端全文索引的dbCOMMITID值一致， 一个全文索引对应一个固定集合
     * 每个全文索引数组元素与每个固定集合对象数组元素一一对应
     * 
     * @param esClient
     * @param esIndexNames
     * @param cappedCLs
     * @return boolean 如果ES端的SDBCOMMIT._lid的值与对应固定集合最大一条lid的值一致则返回true，否则返回false
     * @Author liuxiaoxuan
     * @Date 2018-11-15
     */
    public static boolean isLastLidInES( Client esClient, List<String> esIndexNames, List<DBCollection> cappedCLs )
            throws Exception {
        boolean isSync = false;
        int timeout = 3600; // 超时 1h
        int interval = 1; // 每次检测间隔时间1s
        int doTimes;

        // 获取每个数据组主节点下的固定集合最大一条lid
        List<Integer> lastLogicalIDs = new ArrayList<>();
        for ( DBCollection cappedCL : cappedCLs ) {
            lastLogicalIDs.add( FullTextDBUtils.getLastLid( cappedCL ) );
        }

        // 检查每个全文索引的SDBCOMMITID与对应固定集合的最大一条lid是否相同
        for ( int i = 0; i < esIndexNames.size(); i++ ) {
            doTimes = 0;
            Integer commitID = -10000;
            while ( doTimes * interval < timeout ) {
                commitID = FullTextESUtils.getCommitIDFromES( esClient, esIndexNames.get( i ) );
                if ( commitID.intValue() != lastLogicalIDs.get( i ).intValue() ) {
                    isSync = false;
                } else {
                    isSync = true;
                }

                if ( isSync ) {
                    break;
                } else {
                    doTimes++;
                    // System.out.println( "esIndexName: " + esIndexNames.get( i
                    // ).toString() + ", doTimes: " + doTimes
                    // + ", commitID: " + commitID + ", lastLogicalID: " +
                    // lastLogicalIDs.get( i ).toString() );
                    try {
                        Thread.sleep( 1000 );
                    } catch ( InterruptedException e ) {
                        e.printStackTrace();
                    }
                }
            }
            // 如果最终没有完成同步则打屏
            if ( !isSync ) {
                System.err.println( "check " + esIndexNames.get( i ).toString() + " lid syn to es timeout" );
                break;
            }
        }
        return isSync;
    }

    /**
     * 检查ES端全文索引是否已重建。当全文索引重建后，ES端SDBCOMMIT记录下的_cllid值会递增，因此重建后的索引_cllid值会比重建前的大
     * 每个全文索引名对应一个_cllid，数组元素要严格按照一一映射
     * 通过 FullTextESUtils.getCommitCLLIDFromES ( esClient, esIndexNames ) 获取每个全文索引对应的SDBCOMMIT._cllid
     * 
     * @param esClient  es连接
     * @param esIndexNames 多个全文索引名
     * @param preCLLids 每个全文索引对应的SDBCOMMIT._cllid
     * @return boolean  如果重建后的_cllid大于重建前的值则返回true，否则返回false
     * @throws Exception 
     * @Author liuxiaoxuan
     * @Date 2019-05-16
     */
    public static boolean isFulltextRebuild( Client esClient, List<String> esIndexNames, List<Integer> preCLLids )
            throws Exception {
        // 比较索引个数和cllid个数，如果数量不一致则直接退出
        if ( esIndexNames.size() != preCLLids.size() ) {
            System.err.println( "esIndexNames' size is not equal to cllids' size, esIndexNames: " + esIndexNames.size()
                    + ", cllids: " + preCLLids.size() );
            return false;
        }

        // 检查每个全文索引下的_cllid值有没有变化
        for ( int i = 0; i < esIndexNames.size(); i++ ) {
            boolean isSync = false;
            isSync = isFulltextRebuild( esClient, esIndexNames.get( i ), preCLLids.get( i ) );
            if ( !isSync ) {
                return false;
            }
            if ( i == esIndexNames.size() - 1 ) {
                return isSync;
            }
        }

        return false;
    }

    /**
     * 检查ES端全文索引是否已重建。当全文索引重建后，ES端SDBCOMMIT记录下的_cllid值会递增，因此重建后的索引_cllid值会比重建前的大
     * 通过 FullTextESUtils.getCommitCLLIDFromES ( esClient, esIndexName ) 获取单个全文索引对应的SDBCOMMIT._cllid
     * 
     * @param esClient  es连接
     * @param esIndexName 全文索引名
     * @param preCLLid 全文索引对应的SDBCOMMIT._cllid
     * @return boolean  如果重建后的_cllid大于重建前的值则返回true，否则返回false
     * @throws Exception 
     * @Author liuxiaoxuan
     * @Date 2019-05-16
     */
    public static boolean isFulltextRebuild( Client esClient, String esIndexName, int preCLLid ) throws Exception {
        boolean isSync = false;
        int timeout = 3600; // timeout 1h
        int interval = 1; // interval 1s
        int doTimes;

        // 检查全文索引下的_cllid值有没有变化
        doTimes = 0;
        Integer curCLLID = -10000;
        while ( doTimes * interval < timeout ) {
            try {
                curCLLID = FullTextESUtils.getCommitCLLIDFromES( esClient, esIndexName );
                if ( curCLLID <= preCLLid ) {
                    isSync = false;
                } else {
                    isSync = true;
                }

                if ( isSync ) {
                    break;
                } else {
                    doTimes++;
                    // System.out.println( "esIndexName: " + esIndexName + ",
                    // doTimes: " + doTimes + ", previousCLLid: "
                    // + preCLLid + ", currentCLLID: " + curCLLID );
                    try {
                        Thread.sleep( 1000 );
                    } catch ( InterruptedException e ) {
                        e.printStackTrace();
                    }
                }
            } catch ( Exception e ) {
                Throwable ths = e.getCause();
                if ( ths instanceof IndexNotFoundException ) {
                    doTimes++;
                    // System.out.println(
                    // "esIndexName: " + esIndexName + ", doTimes: " + doTimes +
                    // " is being truncated now" );
                    try {
                        Thread.sleep( 1000 );
                    } catch ( InterruptedException e2 ) {
                        e2.printStackTrace();
                    }
                } else {
                    System.out.println( "isFulltextRebuild exception: " + ths.getClass() );
                    throw e;
                }
            }
        }

        return isSync;
    }

    /**
     * 数组元素去重
     * 
     * @param arrayList
     * @return List< String > 返回新的无重复元素的数组
     * @Author liuxiaoxuan
     * @Date 2018-11-15
     */
    public static List<String> removeDuplicateItems( List<String> arrayList ) {
        List<String> newArrayList = arrayList;
        HashSet<String> uniqueSet = new HashSet<String>( newArrayList );
        newArrayList.clear();
        newArrayList.addAll( uniqueSet );
        return newArrayList;
    }

    /**
     * 检查主备节点下原始集合和固定集合的数据一致性
     * 
     * @param cl
     * @param textIndexName
     * @return boolean 如果主备节点数据一致则返回true，否则返回false
     * @Author liuxiaoxuan
     * @Date 2019-05-09
     */
    private static boolean isDataConsistency( DBCollection cl, String textIndexName ) {
        // 检查主备节点原始集合的数据一致性
        if ( !isCLDataConsistency( cl ) ) {
            return false;
        }
        if ( !isCappedCLDataConsistency( cl, textIndexName ) ) {
            return false;
        }

        return true;
    }

    /**
     * 检查主备节点的普通表、分区表数据一致性： 1. 先校验主备节点原始集合记录数是否一致 2. 再检验主备节点原始集合每一条记录内容是否一致
     *
     * @param cl
     * @return boolean 如果主备节点原始集合的数据一致则返回true，否则返回false
     * @Author yinzhen
     * @Date 2018-12-21
     */
    public static boolean isCLDataConsistency( DBCollection cl ) {
        boolean isConsistency = false;
        Sequoiadb db = cl.getSequoiadb();
        List<String> groupNames = FullTextDBUtils.getCLGroups( cl );
        // 防止数据组元素重复
        groupNames = removeDuplicateItems( groupNames );

        for ( String groupName : groupNames ) {
            List<String> nodeNames = CommLib.getNodeAddress( db, groupName );
            List<Node> nodes = new ArrayList<>();
            for ( String nodeName : nodeNames ) {
                nodes.add( db.getReplicaGroup( groupName ).getNode( nodeName ) );
            }
            isConsistency = isConsistency( db, groupName, cl.getCSName(), cl.getName() );
            if ( !isConsistency ) {
                break;
            }
        }
        return isConsistency;
    }

    /**
     * 检查主备节点的固定集合数据一致性 1. 先校验主备节点固定集合记录数是否一致 2. 再检验主备节点固定集合每一条记录内容是否一致
     * 
     * @param cl
     * @param textIndexName
     * @return boolean 如果主备节点固定集合的数据一致则返回true，否则返回false
     * @Author liuxiaoxuan
     * @Date 2019-05-09
     */
    public static boolean isCappedCLDataConsistency( DBCollection cl, String textIndexName ) {
        boolean isConsistency = false;
        Sequoiadb db = cl.getSequoiadb();
        String cappedName = FullTextDBUtils.getCappedName( cl, textIndexName );
        List<String> groupNames = FullTextDBUtils.getCLGroups( cl );
        // 防止数据组元素重复
        groupNames = removeDuplicateItems( groupNames );

        for ( String groupName : groupNames ) {
            // List<String> nodeNames = CommLib.getNodeAddress( db, groupName );
            // List<Node> nodes = new ArrayList<>();
            // for ( String nodeName : nodeNames ) {
            // nodes.add( db.getReplicaGroup( groupName ).getNode( nodeName ) );
            // }
            isConsistency = isConsistency( db, groupName, cappedName, cappedName );
            if ( !isConsistency ) {
                break;
            }
        }
        return isConsistency;
    }

    /**
     * 检查主备节点固定集合UniqueID一致
     * 
     * @param db
     * @param fullName
     * @param groupNames
     * @return boolean 如果主备节点固定集合UniqueID一致返回true,否则返回false
     */
    private static boolean isNewCLConsistency( Sequoiadb db, String groupName, String csName, String clName ) {

        String clFullName = csName + "." + clName;
        boolean isConsistency = false;
        List<String> nodeNames = CommLib.getNodeAddress( db, groupName );
        ReplicaGroup rg = db.getReplicaGroup( groupName );
        Sequoiadb masterNode = rg.getMaster().connect();
        DBCursor cursor = masterNode.getSnapshot( Sequoiadb.SDB_SNAP_COLLECTIONS, "{Name:'" + clFullName + "'}",
                "{UniqueID: ''}", null );
        long uniqueID = 0;
        if ( cursor.hasNext() ) {
            uniqueID = (long) cursor.getNext().get( "UniqueID" );
        }
        cursor.close();
        for ( String nodeName : nodeNames ) {
            isConsistency = false;
            Sequoiadb nodeConn = rg.getNode( nodeName ).connect();
            DBCursor cur = null;
            long checkUniqueID = 0;
            for ( int i = 0; i < 300; i++ ) {
                cur = nodeConn.getSnapshot( Sequoiadb.SDB_SNAP_COLLECTIONS, "{Name:'" + clFullName + "'}",
                        "{UniqueID: ''}", null );
                if ( cur.hasNext() ) {
                    checkUniqueID = (long) cur.getNext().get( "UniqueID" );
                }
                cur.close();
                if ( uniqueID != 0 && uniqueID == checkUniqueID ) {
                    isConsistency = true;
                    break;
                }
                try {
                    Thread.sleep( 1000 );
                } catch ( InterruptedException e ) {
                    e.printStackTrace();
                }
            }
            if ( !isConsistency ) {
                System.err.println( "Group [" + groupName + "] UniqueID is not the same, masterNode UniqueID: "
                        + uniqueID + ", " + nodeNames + " UniqueID: " + checkUniqueID );
                break;
            }
        }

        return isConsistency;
    }

    /**
     * 检查主备节点主子表的原始集合和固定集合的数据一致性
     * 
     * @param cl
     * @param textIndexName
     * @return boolean 如果主备节点主子表的数据一致则返回true，否则返回false
     * @Author yinzhen
     * @Date 2018-12-21
     */
    private static boolean isMainCLDataConsistency( DBCollection cl, String textIndexName ) {
        boolean isConsistency = true;
        Sequoiadb db = cl.getSequoiadb();
        List<String> subclNames = FullTextDBUtils.getSubCLNames( db, cl.getFullName() );
        for ( int i = 0; i < subclNames.size(); i++ ) {
            String subcsName = subclNames.get( i ).split( "\\." )[0];
            String subclName = subclNames.get( i ).split( "\\." )[1];
            DBCollection subCL = db.getCollectionSpace( subcsName ).getCollection( subclName );
            isConsistency = isDataConsistency( subCL, textIndexName );
            if ( !isConsistency ) {
                break;
            }
        }
        return isConsistency;
    }

    /**
     * 判断主备节点的集合数据是否一致
     * 
     * @param nodes
     * @param csName
     * @param clName
     * @return boolean
     * @Author yinzhen
     * @Date 2018-12-21
     */
    public static boolean isConsistency( Sequoiadb db, String groupName, String csName, String clName ) {
        boolean isConsistency = false;
        int doTimes = 0;
        int timeout = 600;
        while ( true ) {
            isConsistency = isNodeRecordsConsistency( db, groupName, csName, clName );
            if ( isConsistency ) {
                return isConsistency;
            } else {
                doTimes++;
                // System.out.println( "csName : " + csName + " clName: " +
                // clName + " isConsistency : " + isConsistency
                // + " , doTimes: " + doTimes );
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
     * 判断主备节点的集合数据是否一致
     * 
     * @param nodes
     * @param csName
     * @param clName
     * @return boolean
     * @Author yinzhen
     * @Date 2018-12-21
     */
    public static boolean isNodeRecordsConsistency( Sequoiadb db, String groupName, String csName, String clName ) {

        List<String> nodeNames = CommLib.getNodeAddress( db, groupName );
        if ( nodeNames.size() == 1 ) {
            return true;
        }

        // 判断所有节点是否已同步集合
        if ( !isNewCLConsistency( db, groupName, csName, clName ) ) {
            return false;
        }

        ReplicaGroup rg = db.getReplicaGroup( groupName );
        Sequoiadb firstNode = rg.getNode( nodeNames.get( 0 ) ).connect();
        DBCollection cl1 = firstNode.getCollectionSpace( csName ).getCollection( clName );
        for ( int i = 1; i < nodeNames.size(); i++ ) {
            Sequoiadb nextNode = rg.getNode( nodeNames.get( i ) ).connect();
            DBCollection cl2 = nextNode.getCollectionSpace( csName ).getCollection( clName );
            if ( cl1.getCount() != cl2.getCount() ) {
                System.err.println( "cl from " + nodeNames.get( 0 ) + "'s count: " + cl1.getCount() + ", cl from "
                        + nodeNames.get( i ) + "'s count: " + cl2.getCount() );
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
     * 
     * @param cl1Cursor
     * @param cl2Cursor
     * @return boolean
     * @Author yinzhen
     * @Date 2018-12-21
     */
    public static boolean isCLRecordsConsistency( DBCursor cl1Cursor, DBCursor cl2Cursor ) {
        try {
            while ( cl1Cursor.hasNext() && cl2Cursor.hasNext() ) {
                BSONObject cl1Record = cl1Cursor.getNext();
                BSONObject cl2Record = cl2Cursor.getNext();
                if ( !cl1Record.equals( cl2Record ) ) {
                    System.err.println( "collection from first node's record : " + cl1Record.toString()
                            + "\n collection from anohter node's record : " + cl2Record.toString() );
                    return false;
                }
            }
        } finally {
            cl1Cursor.close();
            cl2Cursor.close();
        }
        return true;
    }

    /**
     * 检查全文索引是否创建,包括检查索引在es端是否被创建,固定集合空间是否被创建,数据是否同步
     * @param db
     * @param esClient
     * @param indexName
     * @return
     * @throws Exception 
     */
    public static boolean isIndexCreated( Client esClient, DBCollection cl, String indexName, int expectCount )
            throws Exception {

        if ( isFullSyncToES( esClient, cl, indexName, expectCount ) && isDataConsistency( cl, indexName ) ) {
            return true;
        } else {
            return false;
        }
    }

    /**
     * 检查主表全文索引是否创建,包括检查索引在es端是否被创建,固定集合空间是否被创建,数据是否同步
     * @param db
     * @param esClient
     * @param indexName
     * @return
     * @throws Exception 
     */
    public static boolean isMainCLIndexCreated( Client esClient, DBCollection cl, String indexName, int expectCount )
            throws Exception {

        if ( isMainCLFullSyncToES( esClient, cl, indexName, expectCount )
                && isMainCLDataConsistency( cl, indexName ) ) {
            return true;
        } else {
            return false;
        }
    }

    /**
     * 检查全文索引是否删除,包括检查索引在es端是否被删除,固定集合空间是否被删除
     * @param db
     * @param esClient
     * @param esIndexName
     * @param cappedName
     * @return boolean 删除成功返回true,否则返回false
     */
    public static boolean isIndexDeleted( Sequoiadb db, Client esClient, String esIndexName, String cappedName ) {

        if ( new FullTextESUtils().isIndexDeletedInES( esClient, esIndexName )
                && new FullTextDBUtils().isCSDropSuccess( db, cappedName ) ) {
            return true;
        } else {
            return false;
        }

    }

    /**
     * 检查全文索引是否删除,包括检查索引在es端是否被删除,固定集合空间是否被删除
     * @param db
     * @param esClient
     * @param esIndexName
     * @param cappedNames
     * @return boolean 删除成功返回true,否则返回false
     */
    public static boolean isIndexDeleted( Sequoiadb db, Client esClient, List<String> esIndexNames,
            List<String> cappedNames ) {

        if ( new FullTextESUtils().isIndexDeletedInES( esClient, esIndexNames )
                && new FullTextDBUtils().isCSDropSuccess( db, cappedNames ) ) {
            return true;
        } else {
            return false;
        }

    }
}
