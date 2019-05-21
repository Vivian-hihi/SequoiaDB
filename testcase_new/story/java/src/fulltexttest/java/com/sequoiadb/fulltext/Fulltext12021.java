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
 * @Description seqDB-12021: range切分表中创建全文索引并切分后再插入记录
 * @author xiaoni Zhao
 * @date 2018/11/23
 */
public class Fulltext12021 extends SdbTestBase {
    private Sequoiadb sdb = null;
    private DBCollection cl;
    private String clName = "ES_cl_12021";
    private String fullTextIndexName = "fullIndex12021";
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
        if(CommLib.OneGroupMode(sdb)){
            throw new SkipException( "current environment less than tow groups " );
        }
        
        List<String> groupsName = CommLib.getDataGroupNames( sdb );
        srcGroup = groupsName.get( 0 );
        desGroup = groupsName.get( 1 );

        cl = sdb.getCollectionSpace( csName ).createCollection( clName,
                (BSONObject) JSON.parse( "{ShardingType:'range', ShardingKey:{a:1}, Group:'" + srcGroup + "'}" ) );
        esClient = FullTextESUtils.createTransportClient( SdbTestBase.esHostName,
                Integer.parseInt( SdbTestBase.esServiceName ) );
    }

    @Test
    public void test() throws Exception {
        cl.createIndex( fullTextIndexName,
                (BSONObject) JSON.parse( "{a : 'text', b : 'text', c : 'text', d : 'text', e : 'text', f : 'text'}" ),
                false, false );
        esIndexNames = FullTextDBUtils.getESIndexNames( cl, fullTextIndexName );
        cl.split( srcGroup, desGroup, (BSONObject) JSON.parse( "{a : 'test_hash12021_0'}" ),
                (BSONObject) JSON.parse( "{a : 'test_hash12021_1000'}" ) );
        FullTextDBUtils.insertData(cl, FullTextUtils.INSERT_NUMS);
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
            Assert.assertTrue( FullTextESUtils.isIndexDeletedInES( esClient, esIndexNames ) );
        }
        sdb.close();
        esClient.close();
    }

}
