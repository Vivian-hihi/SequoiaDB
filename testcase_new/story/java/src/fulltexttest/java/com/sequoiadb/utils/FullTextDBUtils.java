package com.sequoiadb.utils;

import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;
import java.util.List;

import org.bson.BSONObject;
import org.bson.BasicBSONObject;
import org.bson.types.BasicBSONList;
import org.bson.util.JSON;
import org.testng.Assert;

import com.sequoiadb.base.CollectionSpace;
import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.DBCursor;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.testcommon.CommLib;

/**
 * DB的公共类，涉及DB内部操作的方法放于此类
 */
public class FullTextDBUtils {

    /**
     * 获取全文索引对应的固定集合名
     * 
     * @param cl
     * @param textIndexName
     * @return String 返回固定集合名
     * @Author liuxiaoxuan
     * @Date 2018-11-15
     */
    public static String getCappedName( Sequoiadb db, String csName, String clName, String textIndexName ) {
        String cappedName = "";
        DBCollection cl = db.getCollectionSpace( csName ).getCollection( clName );
        DBCursor cur = cl.getIndex( textIndexName );
        cappedName = (String) cur.getNext().get( "ExtDataName" );

        return cappedName;
    }

    /**
     * 获取原始集合下的所有固定集合对象，原始集合可以是普通表、分区表
     * 
     * @param db
     * @param csName
     * @param clName
     * @param textIndexName
     * @return List<DBCollection> 返回所有固定集合对象
     * @Author liuxiaoxuan
     * @Date 2018-11-15
     */
    public static List<DBCollection> getCappedCLs( Sequoiadb db, String csName, String clName, String textIndexName ) {
        String cappedName = getCappedName( db, csName, clName, textIndexName );
        List<String> groupNames = getCLGroups( db, csName + "." + clName );
        // get each cappedCL from each group
        List<DBCollection> cappedCLs = new ArrayList<>();
        for ( String groupName : groupNames ) {
            DBCollection cappedCL = db.getReplicaGroup( groupName ).getMaster().connect()
                    .getCollectionSpace( cappedName ).getCollection( cappedName );
            cappedCLs.add( cappedCL );
        }
        return cappedCLs;
    }

    /**
     * 获取原始集合下的所有全文索引名，原始集合可以是普通表、分区表
     * 
     * @param db
     * @param csName
     * @param clName
     * @param textIndexName
     * @return List<String> 返回所有全文索引名
     * @Author liuxiaoxuan
     * @Date 2018-11-15
     */
    public static List<String> getESIndexNames( Sequoiadb db, String csName, String clName, String textIndexName ) {
        String cappedName = getCappedName( db, csName, clName, textIndexName );

        // get es index names
        List<String> esIndexNames = new ArrayList<>();
        List<String> groupNames = getCLGroups( db, csName + "." + clName );

        for ( String groupName : groupNames ) {
            esIndexNames.add( cappedName.toLowerCase() + "_" + groupName );
        }

        // if sharding cl, return all indices
        return esIndexNames;
    }

    /**
     * 获取固定集合的最大LogicalID
     * 
     * @param cappedCL
     * @return int 返回最大_id值
     * @Author liuxiaoxuan
     * @Date 2018-11-15
     */
    public static int getLastLid( DBCollection cappedCL ) {
        long lastLogicalID = -1;
        BSONObject sortObj = new BasicBSONObject();
        sortObj.put( "_id", 1 );
        List<BSONObject> records = getRecordsFromCL( cappedCL, null, null, sortObj, null, 0, -1 );
        if ( records.size() > 0 ) {
            BSONObject lastMatch = records.get( records.size() - 1 );
            lastLogicalID = (long) lastMatch.get( "_id" );
        }
        return (int) lastLogicalID;
    }

    /**
     * 获取原始集合下符合匹配条件的记录
     * 
     * @param cl
     * @param matcher
     * @param selector
     * @param orderBy
     * @param hint
     * @param skip
     * @param limit
     * @return List<BSONObject> 返回记录
     * @Author liuxiaoxuan
     * @Date 2018-11-15
     */
    public static List<BSONObject> getRecordsFromCL( DBCollection cl, BSONObject matcher, BSONObject selector,
            BSONObject orderBy, BSONObject hint, long skip, long limit ) {
        List<BSONObject> objs = new ArrayList<>();
        DBCursor cur = cl.query( matcher, selector, orderBy, hint, skip, limit );
        while ( cur.hasNext() ) {
            BSONObject obj = cur.getNext();
            objs.add( obj );
        }
        return objs;
    }

    /**
     * 获取原始集合对应的数据组，原始集合可以是普通表、分区表
     * 
     * @param db
     * @param clFullName
     * @return List<String> 返回所有数据组
     * @Author liuxiaoxuan
     * @Date 2018-11-15
     */
    public static List<String> getCLGroups( Sequoiadb db, String clFullName ) {
        if ( CommLib.isStandAlone( db ) ) {
            return new ArrayList<>();
        }
        List<String> groupNames = new ArrayList<>();
        BSONObject matcher = new BasicBSONObject();
        matcher.put( "Name", clFullName );
        DBCursor cur = db.getSnapshot( Sequoiadb.SDB_SNAP_CATALOG, matcher, null, null );
        while ( cur.hasNext() ) {
            BasicBSONList bsonLists = (BasicBSONList) cur.getNext().get( "CataInfo" );
            for ( int i = 0; i < bsonLists.size(); i++ ) {
                BasicBSONObject obj = (BasicBSONObject) bsonLists.get( i );
                groupNames.add( obj.getString( "GroupName" ) );
            }
        }

        groupNames = FullTextUtils.removeDuplicateItems( groupNames );
        compare( groupNames );

        return groupNames;
    }

    /**
     * 获取主表下所有子表的表名
     * 
     * @param db
     * @param mainCLFullName
     * @return List<String> 返回所有子表名
     * @Author liuxiaoxuan
     * @Date 2018-11-15
     */
    public static List<String> getSubCLNames( Sequoiadb db, String mainCLFullName ) {
        if ( CommLib.isStandAlone( db ) ) {
            return new ArrayList<>();
        }
        List<String> subCLNames = new ArrayList<>();
        BSONObject matcher = new BasicBSONObject();
        matcher.put( "Name", mainCLFullName );
        DBCursor cur = db.getSnapshot( Sequoiadb.SDB_SNAP_CATALOG, matcher, null, null );
        while ( cur.hasNext() ) {
            BasicBSONList bsonLists = (BasicBSONList) cur.getNext().get( "CataInfo" );
            for ( int i = 0; i < bsonLists.size(); i++ ) {
                BasicBSONObject obj = (BasicBSONObject) bsonLists.get( i );
                subCLNames.add( obj.getString( "SubCLName" ) );
            }
        }

        return subCLNames;
    }

    /**
     * 删除全文索引，循环规避-147。该问题在bug#SEQUOIADBMAINSTAREM-3778跟踪，待问题解决后此方法可去除
     * 
     * @param cl
     * @param textIndexName
     * @return void
     * @Author liuxiaoxuan
     * @Date 2018-11-26
     */
    public static void dropFullTextIndex( DBCollection cl, String textIndexName ) {
        int doTimes = 0;
        while ( true ) {
            try {
                cl.dropIndex( textIndexName );
                // drop success
                break;
            } catch ( BaseException e ) {
                doTimes++;
                if ( -147 == e.getErrorCode() ) {
                    try {
                        Thread.sleep( 1000 );
                    } catch ( InterruptedException e2 ) {
                        e2.printStackTrace();
                    }
                    continue;
                } else if ( -47 == e.getErrorCode() ) { // index not exists
                    System.out.println( textIndexName + " is not exist" );
                    break;
                }

                Assert.assertEquals( e, -147, "drop " + textIndexName + "failed, detail: " + e.getMessage() );
            }
        }
        System.out.println( textIndexName + " drop success,  drop times: " + doTimes );
    }

    /**
     * 删除原始集合空间，循环规避-147。该问题在bug#SEQUOIADBMAINSTAREM-3778跟踪，待问题解决后此方法可去除
     * 
     * @param db
     * @param csName
     * @return void
     * @Author liuxiaoxuan
     * @Date 2018-11-26
     */
    public static void dropCollectionSpace( Sequoiadb db, String csName ) {
        int doTimes = 0;
        while ( true ) {
            try {
                db.dropCollectionSpace( csName );
                // drop success
                break;
            } catch ( BaseException e ) {
                doTimes++;
                if ( -147 == e.getErrorCode() ) {
                    try {
                        Thread.sleep( 1000 );
                    } catch ( InterruptedException e2 ) {
                        e2.printStackTrace();
                    }
                    continue;
                } else if ( -34 == e.getErrorCode() ) { // cs not exists
                    System.out.println( csName + " is not exist" );
                    break;
                }

                Assert.assertEquals( e, -147, "drop " + csName + "failed, detail: " + e.getMessage() );
            }
        }
        System.out.println( csName + " drop success,  drop times: " + doTimes );

    }

    /**
     * 删除原始集合，循环规避-147。该问题在bug#SEQUOIADBMAINSTAREM-3778跟踪，待问题解决后此方法可去除
     * 
     * @param cs
     * @param clName
     * @return void
     * @Author liuxiaoxuan
     * @Date 2018-11-26
     */
    public static void dropCollection( CollectionSpace cs, String clName ) {
        int doTimes = 0;
        while ( true ) {
            try {
                cs.dropCollection( clName );
                // drop success
                break;
            } catch ( BaseException e ) {
                doTimes++;
                if ( -147 == e.getErrorCode() ) {
                    try {
                        Thread.sleep( 1000 );
                    } catch ( InterruptedException e2 ) {
                        e2.printStackTrace();
                    }
                    continue;
                } else if ( -23 == e.getErrorCode() ) { // cl not exists
                    System.out.println( clName + " is not exist" );
                    break;
                }

                Assert.assertEquals( e, -147, "drop " + clName + "failed, detail: " + e.getMessage() );
            }
        }
        System.out.println( clName + " drop success,  drop times: " + doTimes );

    }

    /**
     * 比较两个字符串的大小并重新排序
     * 
     * @param strs
     * @return void
     * @Author liuxiaoxuan
     * @Date 2018-11-15
     */
    public static void compare( List<String> strs ) {
        Collections.sort( strs, new Comparator<Object>() {
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
    }

    /**
     * 判断是否主表
     * 
     * @param sdb
     * @param clFullName
     * @return boolean
     * @Author yinzhen
     * @Date 2018-12-21
     */
    public static boolean isMainCL( Sequoiadb sdb, String clFullName ) {
        DBCursor cursor = sdb.getSnapshot( Sequoiadb.SDB_SNAP_CATALOG, "{'Name':'" + clFullName + "'}", null, null );
        Object isMainCL = cursor.getNext().get( "IsMainCL" );
        if ( isMainCL != null ) {
            boolean isMain = (Boolean) isMainCL;
            return isMain;
        }
        return false;
    }

    /**
     * 指定记录数插入记录，如: {id: 0, a: "clname0", b: "8 byte str", c: "32 byte str...",
     * d: "64 byte str...", e: "128 byte str..."}
     *
     * @param cl
     * @param insertNum
     * @return 无返回
     * @Author luweikang
     * @Date 2019-05-08
     */
    public static void insertData( DBCollection cl, int insertNum ) {
        String clName = cl.getName();
        List<BSONObject> insertObjs = new ArrayList<BSONObject>();
        int insertTimes = 100;
        int insertRecordNum = insertNum / insertTimes;
        String strB = FullTextUtils.getRandomString( 8 );
        String strC = FullTextUtils.getRandomString( 32 );
        String strD = FullTextUtils.getRandomString( 64 );
        String strE = FullTextUtils.getRandomString( 128 );
        for ( int i = 0; i < insertTimes; i++ ) {
            for ( int j = 0; j < insertRecordNum; j++ ) {
                int recordNum = i * insertRecordNum + j;
                insertObjs.add( (BSONObject) JSON.parse( "{id: " + recordNum + ", a: '" + clName + recordNum + "', b: '"
                        + strB + "', c: '" + strC + "', d: '" + strD + "', e: '" + strE + "'}" ) );
            }
            cl.insert( insertObjs, 0 );
            insertObjs.clear();
        }
    }

    /**
     * 检查固定集合空间是已清除,每秒检查一次,检测时间最长为10分钟
     * 
     * @param db
     * @param cappedCSName
     * @param rgNames
     * @return 无返回值
     * @throws InterruptedException
     * @Author luweikang
     * @Date 2019-05-09
     */
    public static void checkDropCappedCS( Sequoiadb db, String cappedCSName, List<String> rgNames )
            throws InterruptedException {
        for ( String rgName : rgNames ) {
            checkDropCappedCS( db, cappedCSName, rgName );
        }
    }

    /**
     * 检查固定集合空间是已清除,每秒检查一次,检测时间最长为10分钟
     * 
     * @param db
     * @param cappedCSName
     * @param rgName
     * @return 无返回值
     * @throws InterruptedException
     * @Author luweikang
     * @Date 2019-05-09
     */
    public static void checkDropCappedCS( Sequoiadb db, String cappedCSName, String rgName )
            throws InterruptedException {
        boolean cappedCSExist = true;
        List<String> nodeList = CommLib.getNodeAddress( db, rgName );
        for ( String nodeAddress : nodeList ) {
            cappedCSExist = true;
            try ( Sequoiadb nodeConn = new Sequoiadb( nodeAddress, "", "" ) ) {
                for ( int i = 0; i < 600; i++ ) {
                    if ( i % 60 == 0 ) {
                        System.out.println( "check capped cs on " + nodeAddress + ", time: " + ( i / 60 ) + " min" );
                    }
                    if ( !nodeConn.isCollectionSpaceExist( cappedCSName ) ) {
                        cappedCSExist = false;
                        break;
                    }
                    Thread.sleep( 1000 );
                }
            }

            Assert.assertFalse( cappedCSExist,
                    "capped cs '" + cappedCSName + "' is still on the rg: " + rgName + ", node: " + nodeAddress );
        }
    }

}
