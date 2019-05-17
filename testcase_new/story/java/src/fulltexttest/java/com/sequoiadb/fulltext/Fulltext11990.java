package com.sequoiadb.fulltext;

import java.util.ArrayList;
import java.util.List;

import org.bson.BSONObject;
import org.bson.BasicBSONObject;
import org.bson.util.JSON;
import org.elasticsearch.client.Client;
import org.testng.Assert;
import org.testng.SkipException;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import com.sequoiadb.base.CollectionSpace;
import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.testcommon.CommLib;
import com.sequoiadb.testcommon.SdbTestBase;
import com.sequoiadb.utils.FullTextDBUtils;
import com.sequoiadb.utils.FullTextESUtils;
import com.sequoiadb.utils.FullTextUtils;
import com.sequoiadb.utils.StringUtils;

/**
 * FileName: seqDB-11990:主子表中创建/删除全文索引
 * 
 * @author liuxiaoxuan
 * @Date 2018.11.27
 */
public class Fulltext11990 extends SdbTestBase {

    private Sequoiadb sdb = null;
    private CollectionSpace cs = null;
    private DBCollection maincl = null;
    private String mainCLName = "ES_11990_maincl";
    private String subCLName1 = "ES_11990_subcl_1";
    private String subCLName2 = "ES_11990_subcl_2";
    private String srcGroupName = "";
    private String destGroupName = "";
    private Client esClient = null;

    @BeforeClass
    public void setUp() {
        esClient = FullTextESUtils.createTransportClient( esHostName, Integer.parseInt( esServiceName ) );
        sdb = new Sequoiadb( SdbTestBase.coordUrl, "", "" );
        if ( CommLib.isStandAlone( sdb ) ) {
            throw new SkipException( "skip StandAlone" );
        }
        ArrayList<String> groupsName = CommLib.getDataGroupNames( sdb );
        if ( groupsName.size() < 2 ) {
            throw new SkipException( "current environment less than tow groups " );
        }

        srcGroupName = groupsName.get( 0 );
        destGroupName = groupsName.get( 1 );
        // create maincl and subcls
        cs = sdb.getCollectionSpace( csName );
        maincl = cs.createCollection( mainCLName,
                (BSONObject) JSON.parse( "{ShardingKey:{a:1}, ShardingType:'range', IsMainCL:true}" ) );
        cs.createCollection( subCLName1 );
        DBCollection subcl = cs.createCollection( subCLName2,
                (BSONObject) JSON.parse( "{ShardingKey:{a0:1}, ShardingType:'hash', Group:'" + srcGroupName + "'}" ) );
        subcl.split( srcGroupName, destGroupName, 50 );
    }

    @AfterClass
    public void tearDown() {
        FullTextDBUtils.dropCollection( cs, subCLName1 );
        FullTextDBUtils.dropCollection( cs, subCLName2 );
        FullTextDBUtils.dropCollection( cs, mainCLName );
        sdb.close();
        esClient.close();
    }

    @Test
    public void test() {
        // attach CL
        BSONObject options1 = (BSONObject) JSON.parse( "{LowBound:{a:'testa'}, UpBound:{a:'testa 999999'}}" );
        BSONObject options2 = (BSONObject) JSON.parse( "{LowBound:{a:'zzza'}, UpBound:{a:'zzza 999999'}}" );
        maincl.attachCollection( csName + "." + subCLName1, options1 );
        maincl.attachCollection( csName + "." + subCLName2, options2 );

        // insert one group
        insertDataOneGroup( maincl, FullTextUtils.INSERT_NUMS );

        // create fulltext of maincl shardingkey
        String textIndexName = "fulltext11990";
        BSONObject indexObj = new BasicBSONObject();
        indexObj.put( "a", "text" );
        maincl.createIndex( textIndexName, indexObj, false, false );

        // get esIndexNames of each subcl
        List<String> subCLFullNames = FullTextDBUtils.getSubCLNames( sdb, csName + "." + mainCLName );
        List<String> esIndexNames = new ArrayList<>();
        for ( String subCLFullName : subCLFullNames ) {
            String subCSName = subCLFullName.split( "\\." )[0];
            String subCLName = subCLFullName.split( "\\." )[1];
            DBCollection subCL = sdb.getCollectionSpace( subCSName ).getCollection( subCLName );
            esIndexNames.addAll( FullTextDBUtils.getESIndexNames( subCL, textIndexName ) );
        }

        Assert.assertTrue(
                FullTextUtils.isMainCLFullSyncToES( esClient, maincl, textIndexName, FullTextUtils.INSERT_NUMS ) );
        Assert.assertTrue( FullTextUtils.isMainCLDataConsistency( maincl, textIndexName ) );
        System.out.println( "check fulltext of maincl shardingkey success when datas in one group!" );

        // create fulltext of subcl shardingkey and non-shardingkey
        FullTextDBUtils.dropFullTextIndex( maincl, textIndexName );
        Assert.assertFalse( FullTextESUtils.isExistIndexInES( esClient, esIndexNames, false ) );

        indexObj = new BasicBSONObject();
        indexObj.put( "a0", "text" );
        indexObj.put( "b", "text" );
        indexObj.put( "c", "text" );
        indexObj.put( "d", "text" );
        indexObj.put( "e", "text" );
        indexObj.put( "f", "text" );
        maincl.createIndex( textIndexName, indexObj, false, false );

        Assert.assertTrue(
                FullTextUtils.isMainCLFullSyncToES( esClient, maincl, textIndexName, FullTextUtils.INSERT_NUMS ) );
        Assert.assertTrue( FullTextUtils.isMainCLDataConsistency( maincl, textIndexName ) );
        System.out
                .println( "check fulltext of subcl shardingkey and non-shardingkey success when datas in one group!" );

        FullTextDBUtils.dropFullTextIndex( maincl, textIndexName );
        Assert.assertFalse( FullTextESUtils.isExistIndexInES( esClient, esIndexNames, false ) );
        maincl.truncate();

        // insert more groups
        insertDataMoreGroups( maincl, FullTextUtils.INSERT_NUMS );

        // create fulltext of maincl shardingkey
        indexObj = new BasicBSONObject();
        indexObj.put( "a", "text" );
        maincl.createIndex( textIndexName, indexObj, false, false );

        Assert.assertTrue(
                FullTextUtils.isMainCLFullSyncToES( esClient, maincl, textIndexName, FullTextUtils.INSERT_NUMS ) );
        Assert.assertTrue( FullTextUtils.isMainCLDataConsistency( maincl, textIndexName ) );
        System.out.println( "check fulltext of maincl shardingkey success when datas in more groups!" );

        // create fulltext of subcl shardingkey and non-shardingkey
        FullTextDBUtils.dropFullTextIndex( maincl, textIndexName );
        Assert.assertFalse( FullTextESUtils.isExistIndexInES( esClient, esIndexNames, false ) );

        indexObj = new BasicBSONObject();
        indexObj.put( "a0", "text" );
        indexObj.put( "b", "text" );
        indexObj.put( "c", "text" );
        indexObj.put( "d", "text" );
        indexObj.put( "e", "text" );
        indexObj.put( "f", "text" );
        maincl.createIndex( textIndexName, indexObj, false, false );

        Assert.assertTrue(
                FullTextUtils.isMainCLFullSyncToES( esClient, maincl, textIndexName, FullTextUtils.INSERT_NUMS ) );
        Assert.assertTrue( FullTextUtils.isMainCLDataConsistency( maincl, textIndexName ) );
        System.out.println(
                "check fulltext of subcl shardingkey and non-shardingkey success when datas in more groups!" );

        FullTextDBUtils.dropFullTextIndex( maincl, textIndexName );
        Assert.assertFalse( FullTextESUtils.isExistIndexInES( esClient, esIndexNames, false ) );
    }

    public void insertDataOneGroup( DBCollection cl, int insertNums ) {
        List<BSONObject> insertObjs = new ArrayList<>();
        for ( int i = 0; i < 100; i++ ) {
            for ( int j = 0; j < insertNums / 100; j++ ) {
                insertObjs.add( (BSONObject) JSON.parse( "{a: 'testa " + i * j + "', a0:" + "'test_11990 " + i * j
                        + "', b: '" + StringUtils.getRandomString( 32 ) + "', c: '" + StringUtils.getRandomString( 64 )
                        + "', d: '" + StringUtils.getRandomString( 64 ) + "', e: '" + StringUtils.getRandomString( 128 )
                        + "', f: '" + StringUtils.getRandomString( 128 ) + "'}" ) );
            }
            cl.insert( insertObjs, 0 );
            insertObjs.clear();
        }
    }

    public void insertDataMoreGroups( DBCollection cl, int insertNums ) {
        List<BSONObject> insertObjs = new ArrayList<>();
        for ( int i = 0; i < 100; i++ ) {
            for ( int j = 0; j < insertNums / 2 / 100; j++ ) {
                insertObjs.add( (BSONObject) JSON.parse( "{a: 'testa " + i * j + "', a0:" + "'test_11990 " + i * j
                        + "', b: '" + StringUtils.getRandomString( 32 ) + "', c: '" + StringUtils.getRandomString( 64 )
                        + "', d: '" + StringUtils.getRandomString( 64 ) + "', e: '" + StringUtils.getRandomString( 128 )
                        + "', f: '" + StringUtils.getRandomString( 128 ) + "'}" ) );
            }
            for ( int j = 0; j < insertNums / 2 / 100; j++ ) {
                insertObjs.add( (BSONObject) JSON.parse( "{a: 'zzza " + i * j + "', a0:" + "'test_11990 " + i * j
                        + "', b: '" + StringUtils.getRandomString( 32 ) + "', c: '" + StringUtils.getRandomString( 64 )
                        + "', d: '" + StringUtils.getRandomString( 64 ) + "', e: '" + StringUtils.getRandomString( 128 )
                        + "', f: '" + StringUtils.getRandomString( 128 ) + "'}" ) );
            }
            cl.insert( insertObjs, 0 );
            insertObjs.clear();
        }
    }
}
