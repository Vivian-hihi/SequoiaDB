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
 * @Description seqDB-12020:hash切分表中创建全文索引并切分后再插入记录
 * @author xiaoni Zhao
 * @date 2018/11/23
 */
public class Fulltext12020 extends SdbTestBase {
    private Sequoiadb sdb = null;
    private DBCollection cl;
    private String clName = "ES_cl_12020";
    private String fullTextIndexName = "fullIndex12020";
    private Client esClient = null;
    private String srcGroup = null;
    private String desGroup = null;
    private List<String> esIndexNames = null;

    @BeforeClass
    public void setUp() {

        sdb = new Sequoiadb( SdbTestBase.coordUrl, "", "" );
        if ( CommLib.isStandAlone( sdb ) ) {
            throw new SkipException( "StandAlone environment!" );
        }
        ArrayList<String> groupsName = CommLib.getDataGroupNames( sdb );
        if ( groupsName.size() < 2 ) {
            throw new SkipException( "current environment less than tow groups " );
        }
        srcGroup = groupsName.get( 0 );
        desGroup = groupsName.get( 1 );

        cl = sdb.getCollectionSpace( csName ).createCollection( clName,
                (BSONObject) JSON.parse( "{ShardingType:'hash', ShardingKey:{a:1}, Group:'" + srcGroup + "'}" ) );
        esClient = FullTextESUtils.createTransportClient( SdbTestBase.esHostName,
                Integer.parseInt( SdbTestBase.esServiceName ) );
    }

    @Test
    public void test() throws Exception {
        cl.createIndex( fullTextIndexName,
                (BSONObject) JSON.parse( "{a : 'text', b : 'text', c : 'text', d : 'text', e : 'text', f : 'text'}" ),
                false, false );
        esIndexNames = FullTextDBUtils.getESIndexNames( cl, fullTextIndexName );
        cl.split( srcGroup, desGroup, 50 );
        insertData();
        Assert.assertTrue( FullTextUtils.isFullSyncToES( esClient, cl, fullTextIndexName, FullTextUtils.INSERT_NUMS ) );
        Assert.assertTrue( FullTextUtils.isDataConsistency( cl, fullTextIndexName ) );
    }

    @AfterClass
    public void tearDown() {
        CollectionSpace cs = sdb.getCollectionSpace( csName );
        if ( cs.isCollectionExist( clName ) ) {
            FullTextDBUtils.dropCollection( cs, clName );
        }
        // check fulltext deleted
        if ( esIndexNames != null ) {
            Assert.assertFalse( FullTextESUtils.isExistIndexInES( esClient, esIndexNames, false )  );
        }
        sdb.close();
        esClient.close();
    }

    public void insertData() {
        List<BSONObject> records = new ArrayList<BSONObject>();
        for ( int i = 0; i < 100; i++ ) {
            for ( int j = 0; j < FullTextUtils.INSERT_NUMS / 100; j++ ) {
                BSONObject record = (BSONObject) JSON.parse( "{a: 'test_hash12020_" + i * j + "', b: '"
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
