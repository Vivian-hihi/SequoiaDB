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
 * @Description seqDB-12065: 集合空间上存在全文索引，删除集合空间
 * @author yinzhen
 * @date 2018/11/19
 */
public class Fulltext12065 extends SdbTestBase {
    private Sequoiadb sdb;
    private DBCollection cl;
    private String csName12065 = "cs12065";
    private String clName = "dropCollectionSpace12065";
    private String fullIndexName = "fullIndex12065";
    private Client esClient = null;

    @BeforeClass
    public void setUp() {
        this.sdb = new Sequoiadb( SdbTestBase.coordUrl, "", "" );
        CommLib commLib = new CommLib();
        if ( commLib.isStandAlone( sdb ) ) {
            throw new SkipException( "StandAlone environment!" );
        }
        CollectionSpace cs = sdb.createCollectionSpace( csName12065 );
        this.cl = cs.createCollection( clName );
        esClient = FullTextESUtils.createTransportClient( SdbTestBase.esHostName,
                Integer.parseInt( SdbTestBase.esServiceName ) );
    }

    @Test
    public void test() {
        // 在集合上创建1个全文索引，并插入包含索引字段的数据
        this.cl.createIndex( fullIndexName,
                "{\"a\":\"text\",\"b\":\"text\",\"c\":\"text\",\"d\":\"text\",\"e\":\"text\",\"f\":\"text\"}", false,
                false );
        this.insertData( FullTextUtils.INSERT_NUMS );

        // 直连集合所在的数据节点主节点，使用游标的方式获取对应的固定集合中的一条记录
        List<DBCollection> cappedCLs = FullTextDBUtils.getCappedCLs( cl, fullIndexName );
        DBCollection cappedCL = cappedCLs.get( 0 );
        DBCursor cursor = cappedCL.query();
        BSONObject bsonObject = cursor.getNext();

        // 多次执行删除集合空间的操作
        for ( int i = 0; i < 3; i++ ) {
            try {
                sdb.dropCollectionSpace( csName12065 );
                Assert.fail( "drop cs need to return -147!" );
            } catch ( BaseException e ) {
                Assert.assertEquals( e.getErrorCode(), -147, e.getMessage() );
            }
        }

        // 关闭步骤2中打开的游标后，再次删除集合空间
        List<String> esIndexNames = FullTextDBUtils.getESIndexNames( cl, fullIndexName );
        cursor.close();
        Assert.assertTrue( FullTextUtils.isFullSyncToES( esClient, cl, fullIndexName, FullTextUtils.INSERT_NUMS ) );
        Assert.assertTrue( FullTextUtils.isDataConsistency( cl, fullIndexName ) );
        FullTextDBUtils.dropCollectionSpace( sdb, csName12065 );
        Assert.assertFalse( FullTextESUtils.isExistIndexInES( esClient, esIndexNames, false ) );
    }

    @AfterClass
    public void tearDown() {
        try {
            FullTextDBUtils.dropCollectionSpace( sdb, csName12065 );
        } catch ( BaseException e ) {
            Assert.assertEquals( e.getErrorCode(), -34, e.getMessage() );
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
                BSONObject record = (BSONObject) JSON.parse( "{a: 'test_12065_" + i * j + "', b: '"
                        + StringUtils.getRandomString( 32 ) + "', c: '" + StringUtils.getRandomString( 64 ) + "', d: '"
                        + StringUtils.getRandomString( 64 ) + "', e: '" + StringUtils.getRandomString( 128 ) + "', f: '"
                        + StringUtils.getRandomString( 128 ) + "'}" );
                records.add( record );
            }
            this.cl.insert( records );
            records.clear();
        }
    }
}
