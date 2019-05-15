package com.sequoiadb.fulltext;

import java.util.ArrayList;
import java.util.Date;
import java.util.List;

import org.bson.BSONObject;
import org.bson.BasicBSONObject;
import org.bson.util.JSON;
import org.testng.SkipException;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import com.sequoiadb.base.CollectionSpace;
import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.testcommon.CommLib;
import com.sequoiadb.testcommon.SdbTestBase;
import com.sequoiadb.utils.FullTextDBUtils;
import com.sequoiadb.utils.FullTextESUtils;
import com.sequoiadb.utils.FullTextUtils;
import com.sequoiadb.utils.StringUtils;

import org.elasticsearch.client.*;

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
    private List< String > esIndexNames = null;

    @BeforeClass
    public void setUp() {
        esClient = FullTextESUtils.createTransportClient( esHostName,
                Integer.parseInt( esServiceName ) );
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
        if ( esIndexNames != null ) {
            FullTextUtils.checkIndexNotExistInES( esClient, esIndexNames );
        }
        sdb.close();
        esClient.close();
    }

    @Test
    public void test() {
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

        esIndexNames = FullTextDBUtils.getESIndexNames( sdb, csName, clName,
                textIndexName );

        // check drop cl and recreate index after index clear in ES
        insertData( cl, FullTextUtils.INSERT_NUMS );

        FullTextUtils.checkFullSyncToES( esClient, sdb, csName, clName,
                textIndexName, FullTextUtils.INSERT_NUMS );
        FullTextUtils.checkDataConsistency( sdb, csName, clName,
                textIndexName );

        FullTextDBUtils.dropCollection( cs, clName );

        FullTextUtils.checkIndexNotExistInES( esClient, esIndexNames );

        // recreate after ES index clear
        cl = cs.createCollection( clName );
        cl.createIndex( textIndexName, indexObj, false, false );

        // insert new datas
        int newInsertNums = 210000;
        insertData( cl, newInsertNums );

        // check consistency
        FullTextUtils.checkFullSyncToES( esClient, sdb, csName, clName,
                textIndexName, newInsertNums );
        FullTextUtils.checkDataConsistency( sdb, csName, clName,
                textIndexName );

        System.out.println(
                "----------success check drop cl after index clear in ES----------" );

        // check drop cl and recreate index while index processing to clear in
        // ES
        FullTextDBUtils.dropFullTextIndex( cl, textIndexName );// init env
        cl.truncate();
        FullTextUtils.checkIndexNotExistInES( esClient, esIndexNames );
        cl.createIndex( textIndexName, indexObj, false, false );

        // init insert datas
        insertData( cl, FullTextUtils.INSERT_NUMS );

        FullTextUtils.checkFullSyncToES( esClient, sdb, csName, clName,
                textIndexName, FullTextUtils.INSERT_NUMS );
        FullTextUtils.checkDataConsistency( sdb, csName, clName,
                textIndexName );

        FullTextDBUtils.dropCollection( cs, clName );

        // recreate while index processing to clear
        cl = cs.createCollection( clName );
        cl.createIndex( textIndexName, indexObj, false, false );

        // insert new datas
        insertData( cl, newInsertNums );

        // check consistency
        FullTextUtils.checkFullSyncToES( esClient, sdb, csName, clName,
                textIndexName, newInsertNums );
        FullTextUtils.checkDataConsistency( sdb, csName, clName,
                textIndexName );

        System.out.println(
                "----------success check drop cl while index processing to clear in ES----------" );
    }

    public void insertData( DBCollection cl, int insertNums ) {
        List< BSONObject > insertObjs = new ArrayList<>();
        for ( int i = 0; i < 100; i++ ) {
            for ( int j = 0; j < insertNums / 100; j++ ) {
                insertObjs.add( ( BSONObject ) JSON.parse( "{a: 'test_14398_"
                        + StringUtils.getRandomString( 10 ) + "', b: '"
                        + StringUtils.getRandomString( 32 ) + "', c: '"
                        + StringUtils.getRandomString( 64 ) + "', d: '"
                        + StringUtils.getRandomString( 64 ) + "', e: '"
                        + StringUtils.getRandomString( 128 ) + "', f: '"
                        + StringUtils.getRandomString( 128 ) + "', g: "
                        + i * j + "}" ) );

            }
            cl.insert( insertObjs, 0 );
            insertObjs.clear();
        }
    }

}
