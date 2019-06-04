package com.sequoiadb.fulltextparallel;

import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Date;
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
import com.sequoiadb.utils.StringUtils;

/**
 * FileName Fulltext15843A.java test content: 在ES正在同步原始集合中的记录时，删除全文索引与增删改记录并发
 * 
 * @author liuxiaoxuan
 * @Date 2019.05.10
 */
public class Fulltext15843A extends SdbTestBase {
    private final int TIMEOUT = 600000;
    private Sequoiadb db = null;
    private CollectionSpace cs = null;
    private DBCollection cl = null;
    private String clName = "ES_15843_A";
    private String textIndexName = "fulltext15843A";
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
        FullTextDBUtils.insertData( cl, 500000 );

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
        String cappedName = FullTextDBUtils.getCappedName( cl, textIndexName );
        String esIndexName = FullTextDBUtils.getESIndexName( cl,
                textIndexName );
        DropTextIndexThread dropTextIndexThread = new DropTextIndexThread();
        te.addWorker( dropTextIndexThread );
        te.addWorker( new InsertThread() );
        te.addWorker( new UpdateThread() );
        te.addWorker( new DeleteThread() );

        te.run();

        if ( 0 != dropTextIndexThread.getRetCode() ) {
            // 全文索引删除失败，全文检索结果正确
            DBCursor cursor = null;
            try {
                int count = 0;
                BSONObject matcher = ( BSONObject ) JSON
                        .parse( "{'':{'$Text':{'query':{'match_all':{}}}}}" );
                cursor = cl.query( matcher, null, null, null );
                while ( cursor.hasNext() ) {
                    cursor.getNext();
                    count++;
                }
                Assert.assertEquals( count, ( int ) cl.getCount() );
            } finally {
                if ( cursor != null ) {
                    cursor.close();
                }
            }
        } else {
            // 全文索引被删除，执行全文检索报错
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
                    e.printStackTrace();
                    Assert.fail( " fulltext query fail " );
                }
            } finally {
                if ( cursor != null ) {
                    cursor.close();
                }
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
            } catch ( BaseException e ) {
                if ( -147 != e.getErrorCode() ) {
                    Assert.fail( "actual exception: " + e.getErrorCode() );
                }
                saveResult( e.getErrorCode(), e );
            } finally {
                System.out.println(
                        this.getClass().getName().toString() + " end at:"
                                + new SimpleDateFormat( "yyyy-MM-dd HH:mm:ss" )
                                        .format( new Date() ) );
            }
        }
    }

    class InsertThread {
        @ExecuteOrder(step = 1, desc = "往原始集合插入数据")
        public void insert() {
            System.out.println(
                    this.getClass().getName().toString() + " insert begin at:"
                            + new SimpleDateFormat( "yyyy-MM-dd HH:mm:ss" )
                                    .format( new Date() ) );
            try ( Sequoiadb sdb = new Sequoiadb( SdbTestBase.coordUrl, "",
                    "" )) {
                DBCollection cl = sdb.getCollectionSpace( csName )
                        .getCollection( clName );
                List< BSONObject > insertObjs = new ArrayList< BSONObject >();
                int insertRecordNum = 10000;
                String strA = StringUtils.getRandomString( 64 );
                for ( int i = 0; i < insertRecordNum; i++ ) {
                    insertObjs.add( ( BSONObject ) JSON.parse( "{ a: '" + strA
                            + "', b: 'new_insert_15843A_" + i + "'}" ) );
                }
                cl.insert( insertObjs, 0 );
            } finally {
                System.out.println(
                        this.getClass().getName().toString() + " insert end at:"
                                + new SimpleDateFormat( "yyyy-MM-dd HH:mm:ss" )
                                        .format( new Date() ) );
            }
        }
    }

    class UpdateThread {
        @ExecuteOrder(step = 1, desc = "更新全文索引记录")
        public void update() {
            System.out.println(
                    this.getClass().getName().toString() + " update begin at:"
                            + new SimpleDateFormat( "yyyy-MM-dd HH:mm:ss" )
                                    .format( new Date() ) );
            try ( Sequoiadb sdb = new Sequoiadb( SdbTestBase.coordUrl, "",
                    "" )) {
                DBCollection cl = sdb.getCollectionSpace( csName )
                        .getCollection( clName );
                BSONObject modifier = new BasicBSONObject( "$set",
                        new BasicBSONObject( "a",
                                "fulltext15843A_after_update" ) );
                BSONObject matcher = new BasicBSONObject( "id",
                        new BasicBSONObject( "$lt", 500 ) );
                cl.update( matcher, modifier, null );
            } finally {
                System.out.println(
                        this.getClass().getName().toString() + " update end at:"
                                + new SimpleDateFormat( "yyyy-MM-dd HH:mm:ss" )
                                        .format( new Date() ) );
            }
        }
    }

    class DeleteThread {
        @ExecuteOrder(step = 1, desc = "删除全文索引记录")
        public void delete() {
            System.out.println(
                    this.getClass().getName().toString() + " delete begin at:"
                            + new SimpleDateFormat( "yyyy-MM-dd HH:mm:ss" )
                                    .format( new Date() ) );
            try ( Sequoiadb sdb = new Sequoiadb( SdbTestBase.coordUrl, "",
                    "" )) {
                DBCollection cl = sdb.getCollectionSpace( csName )
                        .getCollection( clName );
                BSONObject matcher = new BasicBSONObject( "id",
                        new BasicBSONObject( "$gt", 5000 ) );
                cl.delete( matcher );
            } finally {
                System.out.println(
                        this.getClass().getName().toString() + " delete end at:"
                                + new SimpleDateFormat( "yyyy-MM-dd HH:mm:ss" )
                                        .format( new Date() ) );
            }
        }
    }
}
