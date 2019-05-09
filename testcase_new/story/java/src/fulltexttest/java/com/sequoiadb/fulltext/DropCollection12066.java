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
import com.sequoiadb.base.DBCursor;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.testcommon.CommLib;
import com.sequoiadb.testcommon.SdbTestBase;
import com.sequoiadb.utils.FullTextDBUtils;
import com.sequoiadb.utils.FullTextESUtils;
import com.sequoiadb.utils.FullTextUtils;

/**
 * @Description seqDB-12066: 集合上存在全文索引，删除集合
 * @author yinzhen
 * @date 2018/11/20
 */
public class DropCollection12066 extends SdbTestBase {
    private Sequoiadb sdb;
    private CollectionSpace cs;
    private DBCollection cl;
    private String csName12066 = "cs12066";
    private String clName = "dropCollection12066";
    private String fullIndexName = "fullIndex12066";
    private Client esClient = null;

    @BeforeClass
    public void setUp() {
        this.sdb = new Sequoiadb( SdbTestBase.coordUrl, "", "" );
        CommLib commLib = new CommLib();
        if ( commLib.isStandAlone( sdb ) ) {
            throw new SkipException( "StandAlone environment!" );
        }
        this.cs = sdb.createCollectionSpace( csName12066 );
        this.cl = cs.createCollection( clName );
        esClient = FullTextESUtils.createTransportClient(
                SdbTestBase.esHostName,
                Integer.parseInt( SdbTestBase.esServiceName ) );
    }

    @Test
    public void test() {
        // 在集合上创建1个全文索引，并插入大量包含索引字段的数据
        this.cl.createIndex( fullIndexName,
                "{\"a\":\"text\",\"b\":\"text\",\"c\":\"text\",\"d\":\"text\",\"e\":\"text\",\"f\":\"text\"}",
                false, false );
        this.insertData( FullTextUtils.INSERT_NUMS );

        // 直连主数据节点使用游标的方式获取固定集合中的一条记录
        List< DBCollection > cappedCLs = FullTextDBUtils.getCappedCLs( sdb,
                csName12066, clName, fullIndexName );
        DBCollection cappedCL = cappedCLs.get( 0 );
        DBCursor cursor = cappedCL.query();
        BSONObject bsonObject = cursor.getNext();

        // 多次执行删除集合的操作
        for ( int i = 0; i < 3; i++ ) {
            try {
                cs.dropCollection( clName );
                Assert.fail( "drop collection need to return -147!" );
            } catch ( BaseException e ) {
                Assert.assertEquals( e.getErrorCode(), -147, e.getMessage() );
            }
        }
        // 关闭步骤2中的游标，再次删除集合
        List< String > esIndexNames = FullTextDBUtils.getESIndexNames( sdb,
                csName12066, clName, fullIndexName );
        cursor.close();
        FullTextUtils.checkFullSyncToES( esClient, sdb, csName12066, clName,
                fullIndexName, FullTextUtils.INSERT_NUMS );
        FullTextUtils.checkDataConsistency( sdb, csName12066, clName,
                fullIndexName );
        FullTextDBUtils.dropCollection( this.cs, clName );
        FullTextUtils.checkIndexNotExistInES( esClient, esIndexNames );
    }

    @AfterClass
    public void tearDown() {
        try {
            FullTextDBUtils.dropCollectionSpace( sdb, csName12066 );
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
                        .parse( "{a: 'test_12066_" + i * j + "', b: '"
                                + FullTextUtils.getRandomString( 32 )
                                + "', c: '"
                                + FullTextUtils.getRandomString( 64 )
                                + "', d: '"
                                + FullTextUtils.getRandomString( 64 )
                                + "', e: '"
                                + FullTextUtils.getRandomString( 128 )
                                + "', f: '"
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
