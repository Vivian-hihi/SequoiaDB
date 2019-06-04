package com.sequoiadb.fulltextparallel;

import java.text.SimpleDateFormat;
import java.util.Date;

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
import com.sequoiadb.base.DBCursor;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.testcommon.CommLib;
import com.sequoiadb.testcommon.SdbTestBase;
import com.sequoiadb.threadexecutor.ResultStore;
import com.sequoiadb.threadexecutor.ThreadExecutor;
import com.sequoiadb.threadexecutor.annotation.ExecuteOrder;
import com.sequoiadb.utils.FullTextDBUtils;
import com.sequoiadb.utils.FullTextESUtils;
import com.sequoiadb.utils.FullTextUtils;

/**
 * @FileName seqDB-15844:删除全文索引与全文检索并发
 * @Author
 * @Date liuxiaoxuan 2019.5.10
 */
public class Fulltext15844 extends SdbTestBase {
    private final int TIMEOUT = 600000;
    private Sequoiadb db = null;
    private CollectionSpace cs = null;
    private DBCollection cl = null;
    private String clName = "ES_15844";
    private String textIndexName = "fulltext15844";
    private Client esClient = null;
    ThreadExecutor te = new ThreadExecutor( TIMEOUT );

    @BeforeClass
    public void setUp() {
        esClient = FullTextESUtils.createTransportClient( esHostName,
                Integer.parseInt( esServiceName ) );

        db = new Sequoiadb( SdbTestBase.coordUrl, "", "" );
        if ( CommLib.isStandAlone( db ) ) {
            throw new SkipException( "skip StandAlone" );
        }

        cs = db.getCollectionSpace( csName );
        cl = cs.createCollection( clName );
        FullTextDBUtils.insertData( cl, 10000 );

        BSONObject indexObj = new BasicBSONObject();
        indexObj.put( "a", "text" );
        cl.createIndex( textIndexName, indexObj, false, false );
    }

    @AfterClass
    public void tearDown() {
        try {
            FullTextDBUtils.dropCollection( cs, clName );
        } finally {
            if ( db != null ) {
                db.close();
            }
            if ( esClient != null ) {
                esClient.close();
            }
        }
    }

    @Test
    public void test() throws Exception {
        Assert.assertTrue( FullTextUtils.isIndexCreated( esClient, cl,
                textIndexName, 10000 ) );

        String cappedName = FullTextDBUtils.getCappedName( cl, textIndexName );
        String esIndexName = FullTextDBUtils.getESIndexName( cl,
                textIndexName );

        te.addWorker( new DropTextIndexThread() );
        te.addWorker( new QueryThread() );

        te.run();

        Assert.assertTrue( FullTextUtils.isIndexDeleted( db, esClient,
                esIndexName, cappedName ) );

        FullTextDBUtils.insertData( cl, 100 );

        DBCursor cursor = null;
        try {
            BSONObject matcher = ( BSONObject ) JSON
                    .parse( "{'':{'$Text':{'query':{'match_all':{}}}}}" );
            cursor = cl.query( matcher, null, null, null );
            Assert.fail( "query should fail" );
        } catch ( BaseException e ) {
            if ( -6 != e.getErrorCode() && -52 != e.getErrorCode() ) {
                Assert.fail( "actual exception: " + e.getErrorCode() );
            }
        } finally {
            if ( cursor != null ) {
                cursor.close();
            }
        }
    }

    class DropTextIndexThread extends ResultStore {

        @ExecuteOrder(step = 1, desc = "删除全文索引")
        public void dropTextIndex() {
            System.out.println(
                    this.getClass().getName().toString() + " begin at:"
                            + new SimpleDateFormat( "yyyy-MM-dd HH:mm:ss" )
                                    .format( new Date() ) );
            try ( Sequoiadb sdb = new Sequoiadb( SdbTestBase.coordUrl, "",
                    "" )) {
                DBCollection cl = sdb.getCollectionSpace( csName )
                        .getCollection( clName );
                cl.dropIndex( textIndexName );
            } finally {
                System.out.println(
                        this.getClass().getName().toString() + " end at:"
                                + new SimpleDateFormat( "yyyy-MM-dd HH:mm:ss" )
                                        .format( new Date() ) );
            }

        }
    }

    class QueryThread {

        @ExecuteOrder(step = 1, desc = "執行全文检索")
        public void query() {
            System.out.println(
                    this.getClass().getName().toString() + " begin at:"
                            + new SimpleDateFormat( "yyyy-MM-dd HH:mm:ss" )
                                    .format( new Date() ) );
            Sequoiadb sdb = null;
            DBCursor cursor = null;
            BSONObject matcher = ( BSONObject ) JSON
                    .parse( "{'':{'$Text':{'query':{'match_all':{}}}}}" );
            try {
                sdb = new Sequoiadb( SdbTestBase.coordUrl, "", "" );
                cursor = cl.query( matcher, null, null, null );
                int count = 0;
                while ( cursor.hasNext() ) {
                    cursor.getNext();
                    count++;
                }
                Assert.assertEquals( count, ( int ) cl.getCount() );
            } catch ( BaseException e ) {
                if ( -6 != e.getErrorCode() && -52 != e.getErrorCode() ) {
                    Assert.fail( "actual exception: " + e.getErrorCode() );
                }
            } finally {
                if ( cursor != null ) {
                    cursor.close();
                }
                if ( sdb != null ) {
                    sdb.close();
                }
                System.out.println(
                        this.getClass().getName().toString() + " end at:"
                                + new SimpleDateFormat( "yyyy-MM-dd HH:mm:ss" )
                                        .format( new Date() ) );
            }
        }
    }
}
