package com.sequoiadb.fulltextparallel;

import java.util.ArrayList;
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
 * @FileName FullText15838.java 创建全文索引与split并发
 * @Author luweikang
 * @Date 2019年5月6日
 */
public class FullText15838 extends SdbTestBase {

    private Sequoiadb sdb = null;
    private CollectionSpace cs = null;
    private DBCollection cl = null;
    private String clName = "es_15838";
    private Client esClient = null;
    private String indexName = "fulltextIndex15838";
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

        FullTextDBUtils.insertData( cl, insertNum );
    }

    @Test
    public void test() throws Exception {

        ThreadExecutor thread = new ThreadExecutor();
        thread.addWorker( new CreateIndexThread() );
        thread.addWorker( new SplitThread() );
        thread.run();

        checkSplitResult();

        Assert.assertTrue( FullTextUtils.isIndexCreated( esClient, cl, indexName, insertNum ) );
    }

    @AfterClass
    public void tearDown() throws Exception {
        try {
            List<String> cappedNames = new ArrayList<String>();
            cappedNames.add( FullTextDBUtils.getCappedName( cl, indexName ) );
            List<String> esIndexNames = FullTextDBUtils.getESIndexNames( cl, indexName );
            FullTextDBUtils.dropCollection( cs, clName );
            Assert.assertTrue( FullTextUtils.isIndexDeleted( sdb, esClient, esIndexNames, cappedNames ) );
        } finally {
            if ( sdb != null ) {
                sdb.close();
            }
            if ( esClient != null ) {
                esClient.close();
            }
        }
    }

    private class CreateIndexThread {

        @ExecuteOrder(step = 1)
        private void createIndex() {
            try ( Sequoiadb db = new Sequoiadb( SdbTestBase.coordUrl, "", "" ) ) {
                DBCollection cl = db.getCollectionSpace( csName ).getCollection( clName );
                BSONObject indexObj = new BasicBSONObject();
                indexObj.put( "a", "text" );
                indexObj.put( "b", "text" );
                indexObj.put( "c", "text" );
                indexObj.put( "d", "text" );
                indexObj.put( "e", "text" );
                cl.createIndex( indexName, indexObj, false, false );
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
