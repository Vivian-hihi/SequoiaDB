package com.sequoiadb.fulltext;

import java.util.ArrayList;
import java.util.List;
import org.bson.BSONObject;
import org.bson.util.JSON;
import org.elasticsearch.client.Client;
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

/**
 * @Description seqDB-12021: range切分表中创建全文索引并切分后再插入记录
 * @author xiaoni Zhao
 * @date 2018/11/23
 */
public class SplitAndInsert12021 extends SdbTestBase {
    private Sequoiadb sdb = null;
    private DBCollection cl;
    private String clName = "ES_cl_12021";
    private String fullTextIndexName = "fullIndex12021";
    private Client esClient = null;
    private String srcGroup = null;
    private String desGroup = null;
    private List< String > esIndexNames = null;

    @BeforeClass
    public void setUp() {
        sdb = new Sequoiadb( SdbTestBase.coordUrl, "", "" );
        CommLib commLib = new CommLib();
        if ( commLib.isStandAlone( sdb ) ) {
            throw new SkipException( "StandAlone environment!" );
        }
        ArrayList< String > groupsName = CommLib.getDataGroupNames( sdb );
        if ( groupsName.size() < 2 ) {
            throw new SkipException(
                    "current environment less than tow groups " );
        }
        srcGroup = groupsName.get( 0 );
        desGroup = groupsName.get( 1 );

        cl = sdb.getCollectionSpace( csName ).createCollection( clName,
                ( BSONObject ) JSON.parse(
                        "{ShardingType:'range', ShardingKey:{a:1}, Group:'"
                                + srcGroup + "'}" ) );
        esClient = FullTextESUtils.createTransportClient(
                SdbTestBase.esHostName,
                Integer.parseInt( SdbTestBase.esServiceName ) );
    }

    @Test
    public void test() {
        cl.createIndex( fullTextIndexName, ( BSONObject ) JSON.parse(
                "{a : 'text', b : 'text', c : 'text', d : 'text', e : 'text', f : 'text'}" ),
                false, false );
        esIndexNames = FullTextDBUtils.getESIndexNames( sdb, csName, clName,
                fullTextIndexName );
        cl.split( srcGroup, desGroup,
                ( BSONObject ) JSON.parse( "{a : 'test_hash12021_0'}" ),
                ( BSONObject ) JSON.parse( "{a : 'test_hash12021_1000'}" ) );
        insertData();
        FullTextUtils.checkFullSyncToES( esClient, sdb, csName, clName,
                fullTextIndexName, FullTextUtils.INSERT_NUMS );
        FullTextUtils.checkDataConsistency( sdb, csName, clName,
                fullTextIndexName );
    }

    @AfterClass
    public void tearDown() {
        CollectionSpace cs = sdb.getCollectionSpace( csName );
        if ( cs.isCollectionExist( clName ) ) {
            FullTextDBUtils.dropCollection( cs, clName );
        }
        // check fulltext deleted
        if ( esIndexNames != null ) {
            FullTextUtils.checkIndexNotExistInES( esClient, esIndexNames );
        }
        sdb.close();
        esClient.close();
    }

    public void insertData() {
        List< BSONObject > records = new ArrayList< BSONObject >();
        for ( int i = 0; i < 100; i++ ) {
            for ( int j = 0; j < FullTextUtils.INSERT_NUMS / 100; j++ ) {
                BSONObject record = ( BSONObject ) JSON
                        .parse( "{a: 'test_hash12021_" + i * j + "', b: '"
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
}
