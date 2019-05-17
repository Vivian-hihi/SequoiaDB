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
 * FileName: Hash12018.java test content: 插入记录并执行hash切分再创建全文索引
 * 
 * @author liuxiaoxuan
 * @Date 2018.11.20
 */
public class Fulltext12018 extends SdbTestBase {

    private Sequoiadb sdb = null;
    private CollectionSpace cs = null;
    private DBCollection cl = null;
    private String clName = "ES_hash_12018";
    private String srcGroupName = "";
    private String destGroupName = "";

    private Client esClient = null;
    private List<String> esIndexNames = null;

    @BeforeClass
    public void setUp() {
        esClient = FullTextESUtils.createTransportClient( esHostName, Integer.parseInt( esServiceName ) );
        sdb = new Sequoiadb( SdbTestBase.coordUrl, "", "" );
        if ( CommLib.isStandAlone( sdb ) ) {
            throw new SkipException( "skip StandAlone" );
        }
        ArrayList<String> groupsName = CommLib.getDataGroupNames( sdb );
        if ( groupsName.size() < 2 ) {
            throw new SkipException( "current environment less than tow groups " );
        }

        // create hash cl
        srcGroupName = groupsName.get( 0 );
        destGroupName = groupsName.get( 1 );
        cs = sdb.getCollectionSpace( csName );
        cl = cs.createCollection( clName,
                (BSONObject) JSON.parse( "{ShardingKey:{a:1},ShardingType:'hash',Group:'" + srcGroupName + "'}" ) );
    }

    @AfterClass
    public void tearDown() {
        FullTextDBUtils.dropCollection( cs, clName );
        // check fulltext deleted
        if ( esIndexNames != null ) {
            Assert.assertFalse( FullTextESUtils.isExistIndexInES( esClient, esIndexNames, false ) );
        }
        sdb.close();
        esClient.close();
    }

    @Test
    public void test() throws Exception {
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

        esIndexNames = FullTextDBUtils.getESIndexNames( cl, textIndexName );

        // check consistency
        Assert.assertTrue( FullTextUtils.isFullSyncToES( esClient, cl, textIndexName, FullTextUtils.INSERT_NUMS ) );
        Assert.assertTrue( FullTextUtils.isDataConsistency( cl, textIndexName ) );
    }

    public void insertData( DBCollection cl, int insertNums ) {
        List<BSONObject> insertObjs = new ArrayList<>();
        for ( int i = 0; i < 100; i++ ) {
            for ( int j = 0; j < insertNums / 100; j++ ) {
                insertObjs.add( (BSONObject) JSON.parse( "{a: 'test_hash12018_" + i * j + "', b: '"
                        + StringUtils.getRandomString( 32 ) + "', c: '" + StringUtils.getRandomString( 64 ) + "', d: '"
                        + StringUtils.getRandomString( 64 ) + "', e: '" + StringUtils.getRandomString( 128 ) + "', f: '"
                        + StringUtils.getRandomString( 128 ) + "'}" ) );
            }
            cl.insert( insertObjs, 0 );
            insertObjs.clear();
        }
    }

}
