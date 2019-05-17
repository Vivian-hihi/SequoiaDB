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
 * FileName: DropCSAndRecreateIndex14397.java test content: 集合空间删除后重建相同的全文索引
 * 
 * @author liuxiaoxuan
 * @Date 2018.11.21
 */
public class Fulltext14397 extends SdbTestBase {

    private Sequoiadb sdb = null;
    private CollectionSpace cs = null;
    private DBCollection cl = null;
    private String csName = "ES_cs_14397";
    private String clName = "ES_cl_14397";

    private Client esClient = null;
    private List<String> esIndexNames = null;

    @BeforeClass
    public void setUp() {
        esClient = FullTextESUtils.createTransportClient( esHostName, Integer.parseInt( esServiceName ) );
        sdb = new Sequoiadb( SdbTestBase.coordUrl, "", "" );
        if ( CommLib.isStandAlone( sdb ) ) {
            throw new SkipException( "skip StandAlone" );
        }

        // create cl
        cs = sdb.createCollectionSpace( this.csName );
        cl = cs.createCollection( clName );
    }

    @AfterClass
    public void tearDown() {
        FullTextDBUtils.dropCollectionSpace( sdb, csName );
        // check fulltext deleted
        if ( esIndexNames != null ) {
            Assert.assertFalse( FullTextESUtils.isExistIndexInES( esClient, esIndexNames, false ) );
        }
        sdb.close();
        esClient.close();
    }

    @Test
    public void test() {
        // create fulltext
        String textIndexName = "fulltext14397";
        BSONObject indexObj = new BasicBSONObject();
        indexObj.put( "a", "text" );
        indexObj.put( "b", "text" );
        indexObj.put( "c", "text" );
        indexObj.put( "d", "text" );
        indexObj.put( "e", "text" );
        indexObj.put( "f", "text" );
        cl.createIndex( textIndexName, indexObj, false, false );

        esIndexNames = FullTextDBUtils.getESIndexNames( cl, textIndexName );

        // check drop cs and recreate index after index clear in ES
        insertData( cl, FullTextUtils.INSERT_NUMS );

        Assert.assertTrue( FullTextUtils.isFullSyncToES( esClient, cl, textIndexName, FullTextUtils.INSERT_NUMS ) );
        Assert.assertTrue( FullTextUtils.isDataConsistency( cl, textIndexName ) );

        FullTextDBUtils.dropCollectionSpace( sdb, csName );

        Assert.assertFalse( FullTextESUtils.isExistIndexInES( esClient, esIndexNames, false ) );

        // recreate after ES index clear
        cs = sdb.createCollectionSpace( this.csName );
        cl = cs.createCollection( clName );
        cl.createIndex( textIndexName, indexObj, false, false );

        // insert new datas
        int newInsertNums = 210000;
        insertData( cl, newInsertNums );

        // check consistencty
        Assert.assertTrue( FullTextUtils.isFullSyncToES( esClient, cl, textIndexName, newInsertNums ) );
        Assert.assertTrue( FullTextUtils.isDataConsistency( cl, textIndexName ) );

        System.out.println( "----------success check drop cs after index clear in ES----------" );

        // check drop cs and recreate index while index processing to clear in
        // ES
        FullTextDBUtils.dropFullTextIndex( cl, textIndexName );// init env
        cl.truncate();
        Assert.assertFalse( FullTextESUtils.isExistIndexInES( esClient, esIndexNames, false ) );

        cl.createIndex( textIndexName, indexObj, false, false );

        // init insert datas
        insertData( cl, FullTextUtils.INSERT_NUMS );

        Assert.assertTrue( FullTextUtils.isFullSyncToES( esClient, cl, textIndexName, FullTextUtils.INSERT_NUMS ) );
        Assert.assertTrue( FullTextUtils.isDataConsistency( cl, textIndexName ) );

        FullTextDBUtils.dropCollectionSpace( sdb, csName );

        // recreate cs and index while index processing to clear in ES
        cs = sdb.createCollectionSpace( this.csName );
        cl = cs.createCollection( clName );
        cl.createIndex( textIndexName, indexObj, false, false );

        // insert new datas
        insertData( cl, newInsertNums );

        // check result after index recreate
        Assert.assertTrue( FullTextUtils.isFullSyncToES( esClient, cl, textIndexName, newInsertNums ) );
        Assert.assertTrue( FullTextUtils.isDataConsistency( cl, textIndexName ) );

        System.out.println( "----------success check drop cs while index processing to clear in ES----------" );
    }

    public void insertData( DBCollection cl, int insertNums ) {
        List<BSONObject> insertObjs = new ArrayList<>();
        for ( int i = 0; i < 100; i++ ) {
            for ( int j = 0; j < insertNums / 100; j++ ) {
                insertObjs.add( (BSONObject) JSON.parse( "{a: 'test_14397_" + StringUtils.getRandomString( 10 )
                        + "', b: '" + StringUtils.getRandomString( 32 ) + "', c: '" + StringUtils.getRandomString( 64 )
                        + "', d: '" + StringUtils.getRandomString( 64 ) + "', e: '" + StringUtils.getRandomString( 128 )
                        + "', f: '" + StringUtils.getRandomString( 128 ) + "'}" ) );

            }
            cl.insert( insertObjs, 0 );
            insertObjs.clear();
        }
    }

}
