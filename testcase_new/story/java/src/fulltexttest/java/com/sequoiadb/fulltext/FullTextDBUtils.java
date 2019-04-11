package com.sequoiadb.fulltext;

import java.util.ArrayList;
import java.util.List;
import java.util.Collections;
import java.util.Comparator;

import org.bson.BSONObject;
import org.bson.BasicBSONObject;
import org.bson.types.BasicBSONList;

import com.sequoiadb.base.CollectionSpace;
import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.DBCursor;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.testcommon.CommLib;
import com.sequoiadb.exception.BaseException;
import org.testng.Assert;

public class FullTextDBUtils {

    /**
     * @param cl
     * @param textIndexName
     */
    public static String getCappedCLName( DBCollection cl,
            String textIndexName ) {
        String cappedCLName = "";
        DBCursor cur = cl.getIndex( textIndexName );
        cappedCLName = ( String ) cur.getNext().get( "ExtDataName" );

        return cappedCLName;
    }

    /**
     * @param db
     * @param csName
     * @param clName
     * @param textIndexName
     */
    public static List< DBCollection > getCappedCLs( Sequoiadb db,
            String csName, String clName, String textIndexName ) {
        DBCollection cl = db.getCollectionSpace( csName )
                .getCollection( clName );
        String cappedCLName = getCappedCLName( cl, textIndexName );
        List< String > groupNames = getCLGroups( db, csName + "." + clName );
        // get each cappedCL from each group
        List< DBCollection > cappedCLs = new ArrayList<>();
        for ( String groupName : groupNames ) {
            DBCollection cappedCL = db.getReplicaGroup( groupName ).getMaster()
                    .connect().getCollectionSpace( cappedCLName )
                    .getCollection( cappedCLName );
            cappedCLs.add( cappedCL );
        }
        return cappedCLs;
    }

    /**
     * @param db
     * @param csName
     * @param clName
     * @param textIndexName
     */
    public static List< String > getESIndexNames( Sequoiadb db, String csName,
            String clName, String textIndexName ) {
        DBCollection cl = db.getCollectionSpace( csName )
                .getCollection( clName );
        String cappedCLName = getCappedCLName( cl, textIndexName );

        // get es index names
        List< String > esIndexNames = new ArrayList<>();
        List< String > groupNames = getCLGroups( db, csName + "." + clName );

        for ( String groupName : groupNames ) {
            esIndexNames.add( cappedCLName.toLowerCase() + "_" + groupName );
        }

        // if sharding cl, return all indices
        return esIndexNames;
    }

    /**
     * @param cappedCL
     */
    public static int getLastLid( DBCollection cappedCL ) {
        long lastLogicalID = -1;
        BSONObject sortObj = new BasicBSONObject();
        sortObj.put( "_id", 1 );
        List< BSONObject > records = getRecordsFromCL( cappedCL, null, null,
                sortObj, null, 0, -1 );
        if ( records.size() > 0 ) {
            BSONObject lastMatch = records.get( records.size() - 1 );
            lastLogicalID = ( long ) lastMatch.get( "_id" );
        }
        return ( int ) lastLogicalID;
    }

    /**
     * @param cl
     * @param matcher
     * @param selector
     * @param orderBy
     * @param hint
     * @param skip
     * @param limit
     */
    public static List< BSONObject > getRecordsFromCL( DBCollection cl,
            BSONObject matcher, BSONObject selector, BSONObject orderBy,
            BSONObject hint, long skip, long limit ) {
        List< BSONObject > objs = new ArrayList<>();
        DBCursor cur = cl.query( matcher, selector, orderBy, hint, skip,
                limit );
        while ( cur.hasNext() ) {
            BSONObject obj = cur.getNext();
            objs.add( obj );
        }
        return objs;
    }

    /**
     * @param db
     * @param clFullName
     */
    public static List< String > getCLGroups( Sequoiadb db,
            String clFullName ) {
        if ( CommLib.isStandAlone( db ) ) {
            return new ArrayList<>();
        }
        List< String > groupNames = new ArrayList<>();
        BSONObject matcher = new BasicBSONObject();
        matcher.put( "Name", clFullName );
        DBCursor cur = db.getSnapshot( Sequoiadb.SDB_SNAP_CATALOG, matcher,
                null, null );
        while ( cur.hasNext() ) {
            BasicBSONList bsonLists = ( BasicBSONList ) cur.getNext()
                    .get( "CataInfo" );
            for ( int i = 0; i < bsonLists.size(); i++ ) {
                BasicBSONObject obj = ( BasicBSONObject ) bsonLists.get( i );
                groupNames.add( obj.getString( "GroupName" ) );
            }
        }

        groupNames = FullTextUtils.removeDuplicateItems( groupNames );
        compare( groupNames );

        return groupNames;
    }

    /**
     * @param db
     * @param mainCLFullName
     */
    public static List< String > getSubCLNames( Sequoiadb db,
            String mainCLFullName ) {
        if ( CommLib.isStandAlone( db ) ) {
            return new ArrayList<>();
        }
        List< String > subCLNames = new ArrayList<>();
        BSONObject matcher = new BasicBSONObject();
        matcher.put( "Name", mainCLFullName );
        DBCursor cur = db.getSnapshot( Sequoiadb.SDB_SNAP_CATALOG, matcher,
                null, null );
        while ( cur.hasNext() ) {
            BasicBSONList bsonLists = ( BasicBSONList ) cur.getNext()
                    .get( "CataInfo" );
            for ( int i = 0; i < bsonLists.size(); i++ ) {
                BasicBSONObject obj = ( BasicBSONObject ) bsonLists.get( i );
                subCLNames.add( obj.getString( "SubCLName" ) );
            }
        }

        return subCLNames;
    }

    /**
     * @param cl
     * @param textIndexName
     *            bug#SEQUOIADBMAINSTAREM-3778：drop full index will cause error
     *            -147 sometimes
     */
    public static void dropFullTextIndex( DBCollection cl,
            String textIndexName ) {
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

                Assert.assertEquals( e, -147, "drop " + textIndexName
                        + "failed, detail: " + e.getMessage() );
            }
        }
        System.out.println(
                textIndexName + " drop success,  drop times: " + doTimes );
    }

    /**
     * @param db
     * @param csName
     *            bug#SEQUOIADBMAINSTAREM-3778：drop cs contains full index will
     *            cause error -147 sometimes
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

                Assert.assertEquals( e, -147, "drop " + csName
                        + "failed, detail: " + e.getMessage() );
            }
        }
        System.out.println( csName + " drop success,  drop times: " + doTimes );

    }

    /**
     * @param cs
     * @param clName
     *            bug#SEQUOIADBMAINSTAREM-3778：drop cl contains full index will
     *            cause error -147 sometimes
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

                Assert.assertEquals( e, -147, "drop " + clName
                        + "failed, detail: " + e.getMessage() );
            }
        }
        System.out.println( clName + " drop success,  drop times: " + doTimes );

    }

    /**
     * @param strs
     */
    public static void compare( List< String > strs ) {
        Collections.sort( strs, new Comparator() {
            @Override
            public int compare( Object o1, Object o2 ) {
                String str1 = ( String ) o1;
                String str2 = ( String ) o2;
                if ( str1.compareToIgnoreCase( str2 ) < 0 ) {
                    return -1;
                }
                return 1;
            }
        } );
    }

    public static boolean isMainCL( Sequoiadb sdb, String clFullName ) {
        DBCursor cursor = sdb.getSnapshot( Sequoiadb.SDB_SNAP_CATALOG,
                "{'Name':'" + clFullName + "'}", null, null );
        Object isMainCL = cursor.getNext().get( "IsMainCL" );
        if ( isMainCL != null ) {
            boolean isMain = ( Boolean ) isMainCL;
            return isMain;
        }
        return false;
    }
}
