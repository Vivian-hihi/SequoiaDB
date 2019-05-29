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
    private List<String> cappedNames = null;
    private List<String> esIndexNames = null;

    @BeforeClass
    public void setUp() {
        sdb = new Sequoiadb( SdbTestBase.coordUrl, "", "" );
        if ( CommLib.isStandAlone( sdb ) ) {
            throw new SkipException( "StandAlone environment!" );
        }
        if ( CommLib.OneGroupMode( sdb ) ) {
            throw new SkipException( "current environment less than tow groups " );
        }

        List<String> groupsName = CommLib.getDataGroupNames( sdb );
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
        cappedNames = new ArrayList<String>();
        cappedNames.add( FullTextDBUtils.getCappedName( cl, fullTextIndexName ) );
        esIndexNames = FullTextDBUtils.getESIndexNames( cl, fullTextIndexName );
        cl.split( srcGroup, desGroup, 50 );// TODO 建议补充检查原始集合切分结果，如切分后集合数据组个数、数据总数
        FullTextDBUtils.insertData( cl, FullTextUtils.INSERT_NUMS );
        Assert.assertTrue( FullTextUtils.isIndexCreated( esClient, cl, fullTextIndexName, FullTextUtils.INSERT_NUMS ) );
    }

    @AfterClass
    public void tearDown() {
        CollectionSpace cs = sdb.getCollectionSpace( csName );
        if ( cs.isCollectionExist( clName ) ) {// TODO 前面没有删表的操作不需要判断吧？
            FullTextDBUtils.dropCollection( cs, clName );
        }
        // check fulltext deleted
        if ( esIndexNames != null ) {// TODO 不用判断为null
            Assert.assertTrue( FullTextUtils.isIndexDeleted( sdb, esClient, esIndexNames, cappedNames ) );
        }
        sdb.close(); // TODO 关闭sdb和esClient连接需要放到finally，如果teardown前面步骤失败连接可能残留
        esClient.close();
    }

}
