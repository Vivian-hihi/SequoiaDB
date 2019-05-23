package com.sequoiadb.fulltextparallel;

import java.util.ArrayList;
import java.util.List;
import java.util.Random;

import org.bson.BSONObject;
import org.bson.BasicBSONObject;
import org.bson.types.ObjectId;
import org.elasticsearch.client.Client;
import org.testng.Assert;
import org.testng.SkipException;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import com.sequoiadb.base.CollectionSpace;
import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.DBLob;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.testcommon.CommLib;
import com.sequoiadb.testcommon.SdbTestBase;
import com.sequoiadb.threadexecutor.ThreadExecutor;
import com.sequoiadb.threadexecutor.annotation.ExecuteOrder;
import com.sequoiadb.utils.FullTextDBUtils;
import com.sequoiadb.utils.FullTextESUtils;
import com.sequoiadb.utils.FullTextUtils;

/**
 * @FileName FullText15846.java 删除全文索引与lob操作并发
 * @Author luweikang
 * @Date 2019年5月10日
 */
public class FullText15846 extends SdbTestBase {

    private Sequoiadb sdb = null;
    private CollectionSpace cs = null;
    private DBCollection cl = null;
    private String clName = "es_15846";
    private Client esClient = null;
    private String indexName = "fulltextIndex15846";
    private String cappedName = null;
    private String esIndexName = null;
    private int insertNum = 100000;
    private long lobSize = 1024 * 1024 * 10;
    private List<ObjectId> lobTruncateList = new ArrayList<ObjectId>();
    private List<ObjectId> lobRemoveList = new ArrayList<ObjectId>();
    private List<ObjectId> lobReadList = new ArrayList<ObjectId>();

    @BeforeClass
    public void setUp() {
        esClient = FullTextESUtils.createTransportClient( esHostName, Integer.parseInt( esServiceName ) );
        sdb = new Sequoiadb( SdbTestBase.coordUrl, "", "" );
        if ( CommLib.isStandAlone( sdb ) ) {
            throw new SkipException( "skip StandAlone" );
        }
        cs = sdb.getCollectionSpace( csName );
        cl = cs.createCollection( clName );

        List<ObjectId> lobList = writeLob( cl, 100 );
        lobTruncateList.addAll( lobList.subList( 0, 49 ) );
        lobRemoveList.addAll( lobList.subList( 50, 69 ) );
        lobReadList.addAll( lobList.subList( 70, 99 ) );
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

        ThreadExecutor thread = new ThreadExecutor();
        thread.addWorker( new DropIndexThread() );
        thread.addWorker( new TruncateLobThread() );
        thread.addWorker( new PutLobThread() );
        thread.addWorker( new RemoveLobThread() );
        thread.addWorker( new GetLoBThread() );
        thread.run();

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
            } catch ( BaseException e ) {
                e.printStackTrace();
                Assert.assertEquals( e.getErrorCode(), -321, e.getMessage() );
            }
        }
    }

    private class TruncateLobThread {

        @ExecuteOrder(step = 1)
        private void truncateLob() throws InterruptedException {
            try ( Sequoiadb db = new Sequoiadb( SdbTestBase.coordUrl, "", "" ) ) {
                Thread.sleep( 1000 + new Random().nextInt( 100 ) );
                DBCollection cl = db.getCollectionSpace( csName ).getCollection( clName );
                for ( ObjectId lobId : lobTruncateList ) {
                    cl.truncateLob( lobId, lobSize );
                }
            }
        }
    }

    private class PutLobThread {

        @ExecuteOrder(step = 1)
        private void putLob() throws InterruptedException {
            try ( Sequoiadb db = new Sequoiadb( SdbTestBase.coordUrl, "", "" ) ) {
                Thread.sleep( 1000 + new Random().nextInt( 100 ) );
                DBCollection cl = db.getCollectionSpace( csName ).getCollection( clName );
                writeLob( cl, 100 );
            }
        }
    }

    private class RemoveLobThread {

        @ExecuteOrder(step = 1)
        private void removeLob() throws InterruptedException {
            try ( Sequoiadb db = new Sequoiadb( SdbTestBase.coordUrl, "", "" ) ) {
                Thread.sleep( 1000 + new Random().nextInt( 100 ) );
                DBCollection cl = db.getCollectionSpace( csName ).getCollection( clName );
                for ( ObjectId lobId : lobRemoveList ) {
                    cl.removeLob( lobId );
                }
            } catch ( BaseException e ) {
                e.printStackTrace();
            }
        }
    }

    private class GetLoBThread {

        @ExecuteOrder(step = 1)
        private void getLob() throws InterruptedException {
            try ( Sequoiadb db = new Sequoiadb( SdbTestBase.coordUrl, "", "" ) ) {
                Thread.sleep( 1000 + new Random().nextInt( 100 ) );
                DBCollection cl = db.getCollectionSpace( csName ).getCollection( clName );
                for ( ObjectId lobId : lobReadList ) {
                    DBLob lob = cl.openLob( lobId );
                    byte[] data = new byte[(int) lobSize];
                    lob.read( data );
                }
            } catch ( BaseException e ) {
                e.printStackTrace();
            }
        }
    }

    private List<ObjectId> writeLob( DBCollection cl, int lobNum ) {

        List<ObjectId> lobIdList = new ArrayList<ObjectId>();
        byte[] data = new byte[(int) lobSize];
        new Random().nextBytes( data );

        for ( int i = 0; i < lobNum; i++ ) {
            DBLob lob = cl.createLob();
            lob.write( data );
            lob.close();
            lobIdList.add( lob.getID() );
        }

        return lobIdList;
    }

}
