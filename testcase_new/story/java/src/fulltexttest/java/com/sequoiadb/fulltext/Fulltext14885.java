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
import com.sequoiadb.utils.StringUtils;

/**
 * @Description seqDB-14885: 正在查询固定集合时删除全文索引
 * @author yinzhen
 * @date 2018/11/20
 */
public class Fulltext14885 extends SdbTestBase {
    private Sequoiadb sdb;
    private CollectionSpace cs;
    private DBCollection cl;
    private String clName = "dropCollection14885";
    private String fullIndexName = "fullIndex14885";
    private Client esClient = null;

    @BeforeClass
    public void setUp() {
        this.sdb = new Sequoiadb( SdbTestBase.coordUrl, "", "" );
        if ( CommLib.isStandAlone( sdb ) ) {
            throw new SkipException( "StandAlone environment!" );
        }
        this.cs = sdb.getCollectionSpace( csName );
        this.cl = cs.createCollection( clName );
        esClient = FullTextESUtils.createTransportClient( SdbTestBase.esHostName,
                Integer.parseInt( SdbTestBase.esServiceName ) );
    }

    @Test
    public void test() throws Exception {
        // 在集合上创建1个全文索引，并插入包含索引字段的数据
        this.cl.createIndex( fullIndexName,
                "{\"a\":\"text\",\"b\":\"text\",\"c\":\"text\",\"d\":\"text\",\"e\":\"text\",\"f\":\"text\"}", false,
                false );
        this.insertData( FullTextUtils.INSERT_NUMS );

        // 直连主数据节点使用游标的方式获取固定集合中的一条记录
        List<DBCollection> cappedCLs = FullTextDBUtils.getCappedCLs( cl, fullIndexName );
        DBCollection cappedCL = cappedCLs.get( 0 );
        DBCursor cursor = cappedCL.query();
        cursor.getNext();

        // 多次执行删除全文索引的操作，检查结果
        if ( cappedCL.getCount() > 2 ) {
            for ( int i = 0; i < 3; i++ ) {
                try {
                    this.cl.dropIndex( fullIndexName );
                    Assert.fail( "drop textIndex need to return -147!" );
                } catch ( BaseException e ) {
                    Assert.assertEquals( e.getErrorCode(), -147, e.getMessage() );
                }
            }
        }

        // 关闭步骤2中的游标，再次删除集合
        String cappedName = FullTextDBUtils.getCappedName( cl, fullIndexName );
        String esIndexName = FullTextDBUtils.getESIndexName( cl, fullIndexName );

        // 关闭打开的游标
        if ( cursor != null ) {
            cursor.close();
        }

        Assert.assertTrue( FullTextUtils.isIndexCreated( esClient, cl, fullIndexName, FullTextUtils.INSERT_NUMS ) );
        FullTextDBUtils.dropFullTextIndex( this.cl, fullIndexName );
        Assert.assertTrue( FullTextUtils.isIndexDeleted( sdb, esClient, esIndexName, cappedName ) );
    }

    @AfterClass
    public void tearDown() {
        try {
            FullTextDBUtils.dropCollection( cs, clName );
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
                BSONObject record = (BSONObject) JSON.parse( "{a: 'test_14885_" + i * j + "', b: '"
                        + StringUtils.getRandomString( 32 ) + "', c: '" + StringUtils.getRandomString( 64 ) + "', d: '"
                        + StringUtils.getRandomString( 64 ) + "', e: '" + StringUtils.getRandomString( 128 ) + "', f: '"
                        + StringUtils.getRandomString( 128 ) + "'}" );
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
