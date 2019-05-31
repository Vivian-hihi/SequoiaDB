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
 * FileName: Fulltext14377.java test content: 已处理完固定集合中记录，插入/修改/删除/查询集合中的记录
 * 
 * @author liuxiaoxuan
 * @Date 2018.11.21
 */
public class Fulltext14377 extends SdbTestBase {

    private Sequoiadb sdb = null;
    private CollectionSpace cs = null;
    private DBCollection cl = null;
    private String clName = "ES_14377";
    private Client esClient = null;
    private String cappedName = null;
    private String esIndexName = null;

    @BeforeClass
    public void setUp() {
        esClient = FullTextESUtils.createTransportClient( esHostName,
                Integer.parseInt( esServiceName ) );
        sdb = new Sequoiadb( SdbTestBase.coordUrl, "", "" );
        if ( CommLib.isStandAlone( sdb ) ) {
            throw new SkipException( "skip StandAlone" );
        }

        cs = sdb.getCollectionSpace( csName );
        cl = cs.createCollection( clName );
    }

    @AfterClass
    public void tearDown() {
        FullTextDBUtils.dropCollection( cs, clName );
        // 检查全文索引是否残留
        if ( esIndexName != null ) {
            Assert.assertTrue( FullTextUtils.isIndexDeleted( sdb, esClient,
                    esIndexName, cappedName ) );
        }
        if ( sdb != null ) {
            sdb.close();
        }
        if ( esClient != null ) {
            esClient.close();
        }
    }

    @Test
    public void test() throws Exception {
        String textIndexName = "fulltext14377";
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

        insertData( cl, FullTextUtils.INSERT_NUMS );

        // 检查ES端索引数据是否完成同步，主备节点上主表的原始集合、固定集合数据是否一致
        Assert.assertTrue( FullTextUtils.isIndexCreated( esClient, cl,
                textIndexName, FullTextUtils.INSERT_NUMS ) );

        // 增删改
        insertData( cl, 100000 );
        updateData( cl );
        removeData( cl );

        // 检查ES端索引数据是否完成同步，主备节点上主表的原始集合、固定集合数据是否一致
        Assert.assertTrue( FullTextUtils.isIndexCreated( esClient, cl,
                textIndexName, ( int ) cl.getCount() ) );
    }

    private void insertData( DBCollection cl, int insertNums ) {
        List< BSONObject > insertObjs = new ArrayList<>();
        for ( int i = 0; i < 100; i++ ) {
            for ( int j = 0; j < insertNums / 100; j++ ) {
                insertObjs.add( ( BSONObject ) JSON.parse( "{a: 'test_14377_"
                        + i * j + "', b: '" + StringUtils.getRandomString( 32 )
                        + "', c: '" + StringUtils.getRandomString( 64 )
                        + "', d: '" + StringUtils.getRandomString( 64 )
                        + "', e: '" + StringUtils.getRandomString( 128 )
                        + "', f: '" + StringUtils.getRandomString( 128 )
                        + "', g: " + i * j + "}" ) );
            }
            cl.insert( insertObjs, 0 );
            insertObjs.clear();
        }
    }

    private void updateData( DBCollection cl ) {
        BSONObject modifier = new BasicBSONObject( "$set",
                new BasicBSONObject( "g", "-1" ) );
        BSONObject matcher = new BasicBSONObject( "g",
                new BasicBSONObject( "$lt", 100000 ) );
        cl.update( matcher, modifier, null );
    }

    private void removeData( DBCollection cl ) {
        BSONObject matcher = new BasicBSONObject( "g",
                new BasicBSONObject( "$gt", 100000 ) );
        cl.delete( matcher );
    }
}
