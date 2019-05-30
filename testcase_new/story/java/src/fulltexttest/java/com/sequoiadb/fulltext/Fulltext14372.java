package com.sequoiadb.fulltext;

import java.util.ArrayList;
import java.util.List;

import org.bson.BSONObject;
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
 * @Description seqDB-14372:无存量数据，插入记录
 * @author yinzhen
 * @date 2018/11/19
 */
public class Fulltext14372 extends SdbTestBase {
    private Sequoiadb sdb;
    private DBCollection cl;
    private String clName = "insertRecords14372";
    private String fullIndexName = "fullIndex14372";
    private Client esClient = null;
    private String cappedName = null;
    private String esIndexName = null;

    @BeforeClass
    public void setUp() {
        sdb = new Sequoiadb( SdbTestBase.coordUrl, "", "" );
        if ( CommLib.isStandAlone( sdb ) ) {
            throw new SkipException( "StandAlone environment!" );
        }
        CollectionSpace cs = sdb.getCollectionSpace( SdbTestBase.csName );
        cl = cs.createCollection( clName );
        esClient = FullTextESUtils.createTransportClient( SdbTestBase.esHostName,
                Integer.parseInt( SdbTestBase.esServiceName ) );
        cl.createIndex( fullIndexName,
                "{\"a\":\"text\",\"b\":\"text\",\"c\":\"text\",\"d\":\"text\",\"e\":\"text\",\"g\":\"text\"}", false,
                false );
        cappedName = FullTextDBUtils.getCappedName( cl, fullIndexName );
        esIndexName = FullTextDBUtils.getESIndexName( cl, fullIndexName );
    }

    @Test
    public void test() throws Exception {
        insertData( FullTextUtils.INSERT_NUMS );// insert >128M
        Assert.assertTrue( FullTextUtils.isIndexCreated( esClient, cl, fullIndexName, FullTextUtils.INSERT_NUMS ) );
        insertData( FullTextUtils.INSERT_NUMS ); // insert again
        Assert.assertTrue( FullTextUtils.isIndexCreated( esClient, cl, fullIndexName, FullTextUtils.INSERT_NUMS * 2 ) );
    }

    @AfterClass
    public void tearDown() throws Exception {
        try {
            CollectionSpace cs = sdb.getCollectionSpace( SdbTestBase.csName );
            FullTextDBUtils.dropCollection( cs, clName );
            // check fulltext deleted
            if ( esIndexName != null ) {
                Assert.assertTrue( FullTextUtils.isIndexDeleted( sdb, esClient, esIndexName, cappedName ) );
            }
        } finally {
            if ( sdb != null ) {
                sdb.close();
            }
            if ( esClient != null ) {
                esClient.close();
            }
        }
    }

    private void insertData( int insertNums ) {
        List<BSONObject> records = new ArrayList<BSONObject>();
        for ( int i = 0; i < 100; i++ ) {
            for ( int j = 0; j < insertNums / 100; j++ ) {
                BSONObject record = (BSONObject) JSON.parse( "{a: 'test_14372_" + i * j + "', b: '"
                        + StringUtils.getRandomString( 64 ) + "', c: '" + StringUtils.getRandomString( 64 ) + "', d: '"
                        + StringUtils.getRandomString( 64 ) + "', e: '" + StringUtils.getRandomString( 128 ) + "', g: '"
                        + StringUtils.getRandomString( 256 ) + "'}" );
                records.add( record );
            }
            cl.insert( records );
            records.clear();
        }
    }

}
