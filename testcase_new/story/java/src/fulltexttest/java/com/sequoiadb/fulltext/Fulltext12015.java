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
 * FileName: MainCLCurdFullIndex12015.java test content: 主子表中插入/更新/删除包含全文索引字段的记录
 * 
 * @author liuxiaoxuan
 * @Date 2018.11.27
 */
public class Fulltext12015 extends SdbTestBase {

    private Sequoiadb sdb = null;
    private CollectionSpace cs = null;
    private DBCollection maincl = null;
    private String mainCLName = "ES_12015_maincl";
    private String subCLName1 = "ES_12015_subcl_1";
    private String subCLName2 = "ES_12015_subcl_2";
    String textIndexName = "fulltext12015";

    private Client esClient = null;
    List<String> esIndexNames = new ArrayList<>();

    @BeforeClass
    public void setUp() {
        esClient = FullTextESUtils.createTransportClient( esHostName, Integer.parseInt( esServiceName ) );
        sdb = new Sequoiadb( SdbTestBase.coordUrl, "", "" );
        if ( CommLib.isStandAlone( sdb ) ) {
            throw new SkipException( "skip StandAlone" );
        }

        // create maincl and subcls
        cs = sdb.getCollectionSpace( csName );
        maincl = cs.createCollection( mainCLName,
                (BSONObject) JSON.parse( "{ShardingKey:{a:1}, ShardingType:'range', IsMainCL:true}" ) );
        cs.createCollection( subCLName1 );
        cs.createCollection( subCLName2, (BSONObject) JSON.parse( "{ShardingKey:{a0:1}, ShardingType:'hash'}" ) );
    }

    @AfterClass
    public void tearDown() {
        List<String> cappedNames = FullTextDBUtils.getESIndexNames( maincl, textIndexName );
        FullTextDBUtils.dropCollection( cs, subCLName1 );
        FullTextDBUtils.dropCollection( cs, subCLName2 );
        FullTextDBUtils.dropCollection( cs, mainCLName );
        // check fulltext deleted
        if ( esIndexNames != null ) {
            Assert.assertTrue( FullTextUtils.isIndexDeleted( sdb, esClient, esIndexNames, cappedNames ) );
        }
        sdb.close();
        esClient.close();
    }

    @Test
    public void test() throws Exception {
        // attach CL
        BSONObject options1 = (BSONObject) JSON.parse( "{LowBound:{a:'testa'}, UpBound:{a:'testa 999999'}}" );
        BSONObject options2 = (BSONObject) JSON.parse( "{LowBound:{a:'zzza'}, UpBound:{a:'zzza 999999'}}" );
        maincl.attachCollection( csName + "." + subCLName1, options1 );
        maincl.attachCollection( csName + "." + subCLName2, options2 );

        // create fulltext of maincl shardingkey and non-shardingkey
        BSONObject indexObj = new BasicBSONObject();
        indexObj.put( "a", "text" );
        indexObj.put( "a0", "text" );
        indexObj.put( "b", "text" );
        indexObj.put( "c", "text" );
        indexObj.put( "d", "text" );
        indexObj.put( "e", "text" );
        indexObj.put( "f", "text" );
        maincl.createIndex( textIndexName, indexObj, false, false );

        // get esIndexNames of each subcl
        List<String> subCLFullNames = FullTextDBUtils.getSubCLNames( sdb, csName + "." + mainCLName );
        for ( String subCLFullName : subCLFullNames ) {
            String subCSName = subCLFullName.split( "\\." )[0];
            String subCLName = subCLFullName.split( "\\." )[1];
            DBCollection subCL = sdb.getCollectionSpace( subCSName ).getCollection( subCLName );
            esIndexNames.addAll( FullTextDBUtils.getESIndexNames( subCL, textIndexName ) );
        }

        // insert
        insertData( maincl, FullTextUtils.INSERT_NUMS );
        Assert.assertTrue(
                FullTextUtils.isMainCLIndexCreated( esClient, maincl, textIndexName, FullTextUtils.INSERT_NUMS ) );

        // update, should change cl count
        update( maincl );
        insertData( maincl, 10000 );
        Assert.assertTrue( FullTextUtils.isMainCLIndexCreated( esClient, maincl, textIndexName,
                FullTextUtils.INSERT_NUMS + 10000 ) );

        // delete
        remove( maincl );
        Assert.assertTrue(
                FullTextUtils.isMainCLIndexCreated( esClient, maincl, textIndexName, (int) maincl.getCount() ) );

        System.out.println( "check fulltext of maincl shardingkey and non-shardingkey success!" );
    }

    public void insertData( DBCollection cl, int insertNums ) {
        List<BSONObject> insertObjs = new ArrayList<>();
        for ( int i = 0; i < 100; i++ ) {
            for ( int j = 0; j < insertNums / 2 / 100; j++ ) {
                insertObjs.add( (BSONObject) JSON.parse( "{a: 'testa " + i * j + "', a0:" + "'test_12051 " + i * j
                        + "', b: '" + StringUtils.getRandomString( 32 ) + "', c: '" + StringUtils.getRandomString( 64 )
                        + "', d: '" + StringUtils.getRandomString( 64 ) + "', e: '" + StringUtils.getRandomString( 128 )
                        + "', f: '" + StringUtils.getRandomString( 128 ) + "'}" ) );
            }
            for ( int j = 0; j < insertNums / 2 / 100; j++ ) {
                insertObjs.add( (BSONObject) JSON.parse( "{a: 'zzza " + i * j + "', a0:" + "'test_12051 " + i * j
                        + "', b: '" + StringUtils.getRandomString( 32 ) + "', c: '" + StringUtils.getRandomString( 64 )
                        + "', d: '" + StringUtils.getRandomString( 64 ) + "', e: '" + StringUtils.getRandomString( 128 )
                        + "', f: '" + StringUtils.getRandomString( 128 ) + "'}" ) );
            }
            cl.insert( insertObjs, 0 );
            insertObjs.clear();
        }
    }

    private void update( DBCollection cl ) {
        BSONObject modifier = new BasicBSONObject();
        BSONObject value = new BasicBSONObject();
        BSONObject matcher = new BasicBSONObject();
        BSONObject subMatcher = new BasicBSONObject();
        value.put( "a", "testa 99999" );
        modifier.put( "$set", value );
        subMatcher.put( "$lt", "testa 10000" );
        matcher.put( "a", subMatcher );
        cl.update( matcher, modifier, null );
    }

    private void remove( DBCollection cl ) {
        BSONObject matcher = new BasicBSONObject();
        BSONObject subMatcher = new BasicBSONObject();
        subMatcher.put( "$et", "testa 99999" );
        matcher.put( "a", subMatcher );
        cl.delete( matcher );
    }
}
