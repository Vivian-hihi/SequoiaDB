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
 * FileName: DropCLAndRecreateIndex14398.java test content: 集合空间删除后重建相同的全文索引
 * 
 * @author liuxiaoxuan
 * @Date 2018.11.21
 */
public class Fulltext14398 extends SdbTestBase {

    private Sequoiadb sdb = null;
    private CollectionSpace cs = null;
    private DBCollection cl = null;
    private String clName = "ES_cl_14398";

    private Client esClient = null;
    private String cappedName = null;
    private String esIndexName = null;

    @BeforeClass
    public void setUp() {
        esClient = FullTextESUtils.createTransportClient( esHostName, Integer.parseInt( esServiceName ) );
        sdb = new Sequoiadb( SdbTestBase.coordUrl, "", "" );
        if ( CommLib.isStandAlone( sdb ) ) {
            throw new SkipException( "skip StandAlone" );
        }

        // create cl
        cs = sdb.getCollectionSpace( csName );
        cl = cs.createCollection( clName );
    }

    @AfterClass
    public void tearDown() {
        FullTextDBUtils.dropCollection( cs, clName );
        // check fulltext deleted
        if ( esIndexName != null ) {
            Assert.assertTrue( FullTextUtils.isIndexDeleted( sdb, esClient, esIndexName, cappedName ) );
        }
        sdb.close();
        esClient.close();
    }

    @Test
    public void test() throws Exception {
        // create fulltext
        String textIndexName = "fulltext14398";
        BSONObject indexObj = new BasicBSONObject();
        indexObj.put( "a", "text" );
        indexObj.put( "b", "text" );
        indexObj.put( "c", "text" );
        indexObj.put( "d", "text" );
        indexObj.put( "e", "text" );
        indexObj.put( "f", "text" );
        indexObj.put( "g", "text" );
        cl.createIndex( textIndexName, indexObj, false, false );

        cappedName = FullTextDBUtils.getCappedName( cl, textIndexName );
        esIndexName = FullTextDBUtils.getESIndexName( cl, textIndexName );

        // check drop cl and recreate index after index clear in ES
        insertData( cl, FullTextUtils.INSERT_NUMS );

        Assert.assertTrue( FullTextUtils.isIndexCreated( esClient, cl, textIndexName, FullTextUtils.INSERT_NUMS ) );

        FullTextDBUtils.dropCollection( cs, clName );

        Assert.assertTrue( FullTextUtils.isIndexDeleted( sdb, esClient, esIndexName, cappedName ) );

        // recreate after ES index clear
        cl = cs.createCollection( clName );
        cl.createIndex( textIndexName, indexObj, false, false );

        // insert new datas
        int newInsertNums = 210000;
        insertData( cl, newInsertNums );

        // check consistency
        FullTextUtils.isIndexCreated( esClient, cl, textIndexName, newInsertNums );

        System.out.println( "----------success check drop cl after index clear in ES----------" );

        // check drop cl and recreate index while index processing to clear in
        // ES
        FullTextDBUtils.dropFullTextIndex( cl, textIndexName );// init env
        cl.truncate();
        Assert.assertTrue( FullTextUtils.isIndexDeleted( sdb, esClient, esIndexName, cappedName ) );
        cl.createIndex( textIndexName, indexObj, false, false );

        // init insert datas
        insertData( cl, FullTextUtils.INSERT_NUMS );

        Assert.assertTrue( FullTextUtils.isIndexCreated( esClient, cl, textIndexName, FullTextUtils.INSERT_NUMS ) );

        FullTextDBUtils.dropCollection( cs, clName );

        // recreate while index processing to clear
        cl = cs.createCollection( clName );
        cl.createIndex( textIndexName, indexObj, false, false );

        // insert new datas
        insertData( cl, newInsertNums );

        // check consistency
        Assert.assertTrue( FullTextUtils.isIndexCreated( esClient, cl, textIndexName, newInsertNums ) );

        System.out.println( "----------success check drop cl while index processing to clear in ES----------" );
    }

    public void insertData( DBCollection cl, int insertNums ) {
        List<BSONObject> insertObjs = new ArrayList<>();
        for ( int i = 0; i < 100; i++ ) {
            for ( int j = 0; j < insertNums / 100; j++ ) {
                insertObjs.add( (BSONObject) JSON.parse( "{a: 'test_14398_" + StringUtils.getRandomString( 10 )
                        + "', b: '" + StringUtils.getRandomString( 32 ) + "', c: '" + StringUtils.getRandomString( 64 )
                        + "', d: '" + StringUtils.getRandomString( 64 ) + "', e: '" + StringUtils.getRandomString( 128 )
                        + "', f: '" + StringUtils.getRandomString( 128 ) + "', g: " + i * j + "}" ) );

            }
            cl.insert( insertObjs, 0 );
            insertObjs.clear();
        }
    }

}
