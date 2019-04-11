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
import org.elasticsearch.client.*;

/**
 * FileName: Hash12018.java test content: 插入记录并执行hash切分再创建全文索引
 * 
 * @author liuxiaoxuan
 * @Date 2018.11.20
 */
public class Hash12018 extends SdbTestBase {

    private Sequoiadb sdb = null;
    private CollectionSpace cs = null;
    private DBCollection cl = null;
    private String clName = "ES_hash_12018";
    private String srcGroupName = "";
    private String destGroupName = "";

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
        ArrayList< String > groupsName = CommLib.getDataGroupNames( sdb );
        if ( groupsName.size() < 2 ) {
            throw new SkipException(
                    "current environment less than tow groups " );
        }

        // create hash cl
        srcGroupName = groupsName.get( 0 );
        destGroupName = groupsName.get( 1 );
        cs = sdb.getCollectionSpace( csName );
        cl = cs.createCollection( clName,
                ( BSONObject ) JSON
                        .parse( "{ShardingKey:{a:1},ShardingType:'hash',Group:'"
                                + srcGroupName + "'}" ) );
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
        // insert big datas
        insertData( cl, FullTextUtils.INSERT_NUMS );

        // split
        cl.split( srcGroupName, destGroupName, 50 );

        // create fulltext, with shardingkey and non-shardingkey
        String textIndexName = "fulltext12018";
        BSONObject indexObj = new BasicBSONObject();
        indexObj.put( "a", "text" );
        indexObj.put( "b", "text" );
        indexObj.put( "c", "text" );
        indexObj.put( "d", "text" );
        indexObj.put( "e", "text" );
        indexObj.put( "f", "text" );
        cl.createIndex( textIndexName, indexObj, false, false );

        esIndexNames = FullTextDBUtils.getESIndexNames( sdb, csName, clName,
                textIndexName );

        // check consistency
        FullTextUtils.checkFullSyncToES( esClient, sdb, csName, clName,
                textIndexName, FullTextUtils.INSERT_NUMS );
        FullTextUtils.checkConsistency( sdb, csName, clName );
    }

    public void insertData( DBCollection cl, int insertNums ) {
        List< BSONObject > insertObjs = new ArrayList<>();
        for ( int i = 0; i < 100; i++ ) {
            for ( int j = 0; j < insertNums / 100; j++ ) {
                insertObjs.add( ( BSONObject ) JSON
                        .parse( "{a: 'test_hash12018_" + i * j + "', b: '"
                                + FullTextUtils.getRandomString( 32 )
                                + "', c: '"
                                + FullTextUtils.getRandomString( 64 )
                                + "', d: '"
                                + FullTextUtils.getRandomString( 64 )
                                + "', e: '"
                                + FullTextUtils.getRandomString( 128 )
                                + "', f: '"
                                + FullTextUtils.getRandomString( 128 )
                                + "'}" ) );
            }
            cl.insert( insertObjs, 0 );
            insertObjs.clear();
        }
    }

}
