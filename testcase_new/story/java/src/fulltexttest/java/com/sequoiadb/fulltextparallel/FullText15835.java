package com.sequoiadb.fulltextparallel;

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
import com.sequoiadb.base.DBCursor;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.testcommon.CommLib;
import com.sequoiadb.testcommon.SdbTestBase;
import com.sequoiadb.threadexecutor.ThreadExecutor;
import com.sequoiadb.threadexecutor.annotation.ExecuteOrder;
import com.sequoiadb.utils.FullTextDBUtils;
import com.sequoiadb.utils.FullTextESUtils;
import com.sequoiadb.utils.FullTextUtils;

/**
 * @FileName FullText15835.java 删除全文索引与alter操作并发
 * @Author luweikang
 * @Date 2019年5月6日
 */
public class FullText15835 extends SdbTestBase {

    private Sequoiadb sdb = null;
    private CollectionSpace cs = null;
    private DBCollection cl = null;
    private String clName = "es_15835";
    private Client esClient = null;
    private String indexName = "fulltextIndex15835";
    private String cappedName = null;
    private String esIndexName = null;
    private int insertNum = 100000;

    @BeforeClass
    public void setUp() {
        esClient = FullTextESUtils.createTransportClient( esHostName, Integer.parseInt( esServiceName ) );
        sdb = new Sequoiadb( SdbTestBase.coordUrl, "", "" );
        if ( CommLib.isStandAlone( sdb ) ) {
            throw new SkipException( "skip StandAlone" );
        }

        cs = sdb.getCollectionSpace( csName );
        cl = cs.createCollection( clName );

        FullTextDBUtils.insertData( cl, insertNum );
    }

    @Test
    public void test() throws Exception {

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

        ThreadExecutor thread = new ThreadExecutor();
        thread.addWorker( new DropIndexThread() );
        thread.addWorker( new AlterTableThread() );
        thread.run();

        Assert.assertTrue( FullTextUtils.isIndexDeleted( sdb, esClient, esIndexName, cappedName ) );

        checkSnapshotResult();

        // TODO:需要校验集合的插入及全文检索功能

    }

    @AfterClass
    public void tearDown() throws Exception {
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

    private class AlterTableThread {

        @ExecuteOrder(step = 1)
        private void alterTable() {
            try ( Sequoiadb db = new Sequoiadb( SdbTestBase.coordUrl, "", "" ) ) {
                DBCollection cl = db.getCollectionSpace( csName ).getCollection( clName );
                BSONObject options = new BasicBSONObject();
                options.put( "ShardingType", "hash" );
                options.put( "ShardingKey", new BasicBSONObject( "a", 1 ) );
                cl.alterCollection( options );
            }
        }
    }

    private void checkSnapshotResult() {
        DBCursor snap = sdb.getSnapshot( Sequoiadb.SDB_SNAP_CATALOG,
                new BasicBSONObject( "Name", csName + "." + clName ), null, null );
        BSONObject clOption = snap.getNext();
        String shardingType = (String) clOption.get( "ShardingType" );
        BSONObject shardingKey = (BSONObject) clOption.get( "ShardingKey" );
        snap.close();

        Assert.assertEquals( shardingType, "hash" );
        Assert.assertEquals( shardingKey, new BasicBSONObject( "a", 1 ) );
    }
}
