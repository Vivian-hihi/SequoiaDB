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
import com.sequoiadb.exception.BaseException;
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
    private List<String> esIndexNames = null;

    @BeforeClass
    public void setUp() {
        this.sdb = new Sequoiadb( SdbTestBase.coordUrl, "", "" );
        if ( CommLib.isStandAlone( sdb ) ) {
            throw new SkipException( "StandAlone environment!" );
        }
        CollectionSpace cs = sdb.getCollectionSpace( SdbTestBase.csName );
        this.cl = cs.createCollection( clName );
        esClient = FullTextESUtils.createTransportClient( SdbTestBase.esHostName,
                Integer.parseInt( SdbTestBase.esServiceName ) );
        this.cl.createIndex( fullIndexName,
                "{\"a\":\"text\",\"b\":\"text\",\"c\":\"text\",\"d\":\"text\",\"e\":\"text\",\"g\":\"text\"}", false,
                false );
        esIndexNames = FullTextDBUtils.getESIndexNames( cl, fullIndexName );
    }

    @Test
    public void test() throws Exception {
        this.insertData( FullTextUtils.INSERT_NUMS );// insert >128M
        Assert.assertTrue(
                FullTextUtils.isFullSyncToES( esClient, cl, this.fullIndexName, FullTextUtils.INSERT_NUMS ) );
        Assert.assertTrue( FullTextUtils.isDataConsistency( cl, this.fullIndexName ) );
        this.insertData( FullTextUtils.INSERT_NUMS ); // insert again
        Assert.assertTrue(
                FullTextUtils.isFullSyncToES( esClient, cl, this.fullIndexName, FullTextUtils.INSERT_NUMS * 2 ) );
        Assert.assertTrue( FullTextUtils.isDataConsistency( cl, this.fullIndexName ) );
    }

    @AfterClass
    public void tearDown() {
        try {
            CollectionSpace cs = sdb.getCollectionSpace( SdbTestBase.csName );
            FullTextDBUtils.dropCollection( cs, clName );
            // check fulltext deleted
            if ( esIndexNames != null ) {
                Assert.assertFalse( FullTextESUtils.isExistIndexInES( esClient, esIndexNames, false ) );
            }
        } catch ( BaseException e ) {
            Assert.fail( e.getMessage() + "\r\n" + this.getKeyStack( e, this ) );
        } finally {
            if ( sdb != null ) {
                sdb.close();
            }
            if ( esClient != null ) {
                esClient.close();
            }
        }
    }

    public void insertData( int insertNums ) {
        List<BSONObject> records = new ArrayList<BSONObject>();
        for ( int i = 0; i < 100; i++ ) {
            for ( int j = 0; j < insertNums / 100; j++ ) {
                BSONObject record = (BSONObject) JSON.parse( "{a: 'test_14372_" + i * j + "', b: '"
                        + StringUtils.getRandomString( 64 ) + "', c: '" + StringUtils.getRandomString( 64 ) + "', d: '"
                        + StringUtils.getRandomString( 64 ) + "', e: '" + StringUtils.getRandomString( 128 ) + "', g: '"
                        + StringUtils.getRandomString( 256 ) + "'}" );
                records.add( record );
            }
            this.cl.insert( records );
            records.clear();
        }
    }

    public String getKeyStack( Exception e, Object classObj ) {
        StringBuffer stackBuffer = new StringBuffer();
        StackTraceElement[] stackElements = e.getStackTrace();
        for ( int i = 0; i < stackElements.length; i++ ) {
            if ( stackElements[i].toString().contains( classObj.getClass().getName() ) ) {
                stackBuffer.append( stackElements[i].toString() ).append( "\r\n" );
            }
        }
        String str = stackBuffer.toString();
        if ( str.length() >= 2 ) {
            return str.substring( 0, str.length() - 2 );
        } else {
            return str;
        }
    }
}
