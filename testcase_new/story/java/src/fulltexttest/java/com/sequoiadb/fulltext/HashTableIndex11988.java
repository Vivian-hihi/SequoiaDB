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

/**
 * @Description seqDB-11988:hash切分表加入域使用自动切分，创建/删除全文索引
 * @author yinzhen
 * @date 2018/11/13
 */
public class HashTableIndex11988 extends SdbTestBase {
    private Sequoiadb sdb;
    private DBCollection cl;
    private String clName = "hashTableIndex11988";
    private String fullIndexName = "fullIndex11988";
    private List< String > groupNames;
    private Client esClient = null;

    @BeforeClass
    public void setUp() {
        this.sdb = new Sequoiadb( SdbTestBase.coordUrl, "", "" );
        if ( CommLib.isStandAlone( sdb ) ) {
            throw new SkipException( "StandAlone environment!" );
        }
        this.groupNames = CommLib.getDataGroupNames( sdb );
        if ( groupNames.size() < 2 ) {
            throw new SkipException( "Less than two groups!" );
        }
        CollectionSpace cs = sdb.getCollectionSpace( SdbTestBase.csName );
        this.cl = cs.createCollection( clName, ( BSONObject ) JSON.parse(
                "{ShardingKey:{a:1},ShardingType:'hash',AutoSplit:true}" ) );
        esClient = FullTextESUtils.createTransportClient(
                SdbTestBase.esHostName,
                Integer.parseInt( SdbTestBase.esServiceName ) );
    }

    @Test
    public void test() {
        this.insertData( FullTextUtils.INSERT_NUMS );

        // 创建全文索引，索引字段覆盖：分区键和非分区键
        this.cl.createIndex( fullIndexName,
                "{\"a\":\"text\",\"b\":\"text\",\"c\":\"text\",\"d\":\"text\",\"e\":\"text\",\"g\":\"text\"}",
                false, false );
        FullTextUtils.checkFullSyncToES( esClient, sdb, SdbTestBase.csName,
                this.clName, this.fullIndexName, FullTextUtils.INSERT_NUMS );
        FullTextUtils.checkConsistency( sdb, csName, clName );
        List< String > esIndexNames = FullTextDBUtils.getESIndexNames( sdb,
                SdbTestBase.csName, this.clName, this.fullIndexName );

        FullTextDBUtils.dropFullTextIndex( cl, fullIndexName );

        FullTextUtils.checkIndexNotExistInES( esClient, esIndexNames );
    }

    @AfterClass
    public void tearDown() {
        try {
            CollectionSpace cs = sdb.getCollectionSpace( csName );
            FullTextDBUtils.dropCollection( cs, clName );
        } catch ( BaseException e ) {
            Assert.fail(
                    e.getMessage() + "\r\n" + this.getKeyStack( e, this ) );
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
        List< BSONObject > records = new ArrayList< BSONObject >();
        for ( int i = 0; i < 100; i++ ) {
            for ( int j = 0; j < insertNums / 100; j++ ) {
                BSONObject record = ( BSONObject ) JSON
                        .parse( "{a: 'test_hash11988_" + i * j + "', b: '"
                                + FullTextUtils.getRandomString( 32 )
                                + "', c: '"
                                + FullTextUtils.getRandomString( 64 )
                                + "', d: '"
                                + FullTextUtils.getRandomString( 64 )
                                + "', e: '"
                                + FullTextUtils.getRandomString( 128 )
                                + "', g: '"
                                + FullTextUtils.getRandomString( 128 ) + "'}" );
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
            if ( stackElements[ i ].toString()
                    .contains( classObj.getClass().getName() ) ) {
                stackBuffer.append( stackElements[ i ].toString() )
                        .append( "\r\n" );
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
