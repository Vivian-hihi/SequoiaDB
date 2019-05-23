package com.sequoiadb.fulltextparallel;

import java.util.List;

import org.bson.BSONObject;
import org.bson.BasicBSONObject;
import org.elasticsearch.client.Client;
import org.testng.Assert;
import org.testng.SkipException;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import com.sequoiadb.base.CollectionSpace;
import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.ReplicaGroup;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.testcommon.CommLib;
import com.sequoiadb.testcommon.SdbTestBase;
import com.sequoiadb.threadexecutor.ThreadExecutor;
import com.sequoiadb.threadexecutor.annotation.ExecuteOrder;
import com.sequoiadb.utils.FullTextDBUtils;
import com.sequoiadb.utils.FullTextESUtils;
import com.sequoiadb.utils.FullTextUtils;

/**
 * @FileName FullText15839.java 删除全文索引与split操作并发
 * @Author luweikang
 * @Date 2019年5月6日
 */
public class FullText15839 extends SdbTestBase {

    private Sequoiadb sdb = null;
    private CollectionSpace cs = null;
    private DBCollection cl = null;
    private String clName = "es_15839";
    private Client esClient = null;
    private String indexName = "fulltextIndex15839";
    private String cappedName = null;
    private String esIndexName = null;
    private String sourceGruop = null;
    private String targetGruop = null;
    private int insertNum = 100000;

    @BeforeClass
    public void setUp() {
        esClient = FullTextESUtils.createTransportClient( esHostName, Integer.parseInt( esServiceName ) );
        sdb = new Sequoiadb( SdbTestBase.coordUrl, "", "" );
        if ( CommLib.isStandAlone( sdb ) ) {
            throw new SkipException( "skip StandAlone" );
        }
        List<String> groupNames = CommLib.getDataGroupNames( sdb );
        if ( groupNames.size() < 2 ) {
            throw new SkipException( "group less 2" );
        }
        sourceGruop = groupNames.get( 0 );
        targetGruop = groupNames.get( 1 );
        cs = sdb.getCollectionSpace( csName );
        BSONObject options = new BasicBSONObject();
        options.put( "ShardingType", "range" );
        options.put( "ShardingKey", new BasicBSONObject( "recordId", 1 ) );
        options.put( "Group", sourceGruop );
        cl = cs.createCollection( clName, options );
    }

    @Test
    public void test() throws Exception {

        FullTextDBUtils.insertData( cl, insertNum );

        BSONObject indexObj = new BasicBSONObject();
        indexObj.put( "a", "text" );
        indexObj.put( "b", "text" );
        indexObj.put( "c", "text" );
        indexObj.put( "d", "text" );
        indexObj.put( "e", "text" );
        cl.createIndex( indexName, indexObj, false, false );

        Assert.assertTrue( FullTextUtils.isIndexCreated( esClient, cl, indexName, insertNum ) );

        cappedName = FullTextDBUtils.getCappedName( cl, indexName );
        esIndexName = FullTextDBUtils.getESIndexName( cl, indexName );
//TODO:以上步骤均为并发前的准备工作，建议放到 setUp
        ThreadExecutor thread = new ThreadExecutor();
        thread.addWorker( new DropIndexThread() );
        thread.addWorker( new SplitThread() );
        thread.run();

        checkSplitResult();

        Assert.assertTrue( FullTextUtils.isIndexDeleted( sdb, esClient, esIndexName, cappedName ) );

    }

    @AfterClass
    public void tearDown() {
        try {
            FullTextDBUtils.dropCollection( cs, clName );
            Assert.assertTrue( FullTextUtils.isIndexDeleted( sdb, esClient, esIndexName, cappedName ) );
        } finally {
            if ( sdb != null ) {
                sdb.close();
            }
            if ( esClient != null ) {
                esClient.close();
            }
        }
    }

    private class DropIndexThread {

        @ExecuteOrder(step = 1)
        private void createIndex() {
            try ( Sequoiadb db = new Sequoiadb( SdbTestBase.coordUrl, "", "" ) ) {
                DBCollection cl = db.getCollectionSpace( csName ).getCollection( clName );
                cl.dropIndex( indexName );
            }
        }
    }

    private class SplitThread {

        @ExecuteOrder(step = 1)
        private void splitTable() {
            try ( Sequoiadb db = new Sequoiadb( SdbTestBase.coordUrl, "", "" ) ) {
                DBCollection cl = db.getCollectionSpace( csName ).getCollection( clName );
                cl.split( sourceGruop, targetGruop, 50 );
            }
        }
    }

    private void checkSplitResult() {
        ReplicaGroup sourceRG = sdb.getReplicaGroup( sourceGruop );
        ReplicaGroup targetRG = sdb.getReplicaGroup( targetGruop );
        Sequoiadb sdb1 = sourceRG.getMaster().connect();
        Sequoiadb sdb2 = targetRG.getMaster().connect();
        DBCollection cl1 = sdb1.getCollectionSpace( csName ).getCollection( clName );
        DBCollection cl2 = sdb2.getCollectionSpace( csName ).getCollection( clName );

        Assert.assertEquals( cl1.getCount(), insertNum / 2 );
        Assert.assertEquals( cl2.getCount(), insertNum / 2 );
    }

}
