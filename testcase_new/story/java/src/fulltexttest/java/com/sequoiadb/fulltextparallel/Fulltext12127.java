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

/**
 * @FileName seqDB-12127:创建/删除集合与创建/删除全文索引并发
 * @Author
 * @Date liuxiaoxuan 2019.5.10
 */
public class Fulltext12127 extends SdbTestBase {
    private final int TIMEOUT = 600000;
    private Sequoiadb db = null;
    private List< DBCollection > cls = new ArrayList<>();
    private String textIndexName = "fulltext12127";
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

         // 创建集合空间和集合，总共两个集合空间，每个集合空间对应2个集合
        for ( int csNo = 0; csNo < 2; csNo++ ) {
            String csName = "cs12127_" + csNo;
            if ( db.isCollectionSpaceExist( csName ) ) {
                db.dropCollectionSpace( csName );
            }
            CollectionSpace cs = db.createCollectionSpace( csName );
            for ( int clNo = 0; clNo < 2; clNo++ ) {
                String clName = "12127_cl_" + clNo;
                DBCollection cl = cs.createCollection( clName );
                FullTextDBUtils.insertData( cl, 10000 );
                if ( clNo % 2 > 0 ) {
                    BSONObject indexObj = new BasicBSONObject();
                    indexObj.put( "a", "text" );
                    cl.createIndex( textIndexName, indexObj, false, false );
                }
                cls.add( cl );
            }
        }
    }

    @AfterClass
    public void tearDown() {
        try {
            for ( int csNo = 0; csNo < 2; csNo++ ) {
                String csName = "cs12127_" + csNo;
                FullTextDBUtils.dropCollectionSpace( db, csName );
            }
            db.getCollectionSpace( csName ).dropCollection( "12127_new_cl" );
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
        List< String > cappedNames = new ArrayList<>();
        List< String > esIndexNames = new ArrayList<>();
        List< DropTextIndexThread > dropTextIndexThreads = new ArrayList<>();
        List< DropCLThread > dropCLThreads = new ArrayList<>();

        for ( int csNo = 0; csNo < 2; csNo++ ) {
            String csName = "cs12127_" + csNo;
            CollectionSpace cs = db.getCollectionSpace( csName );
            for ( int clNo = 0; clNo < 2; clNo++ ) {
                String clName = "12127_cl_" + clNo;
                DBCollection cl = cs.getCollection( clName );
                if ( clNo % 2 > 0 ) {
                    // 在已存在全文索引的集合中，获取固定集合名和全文索引名
                    cappedNames.add( FullTextDBUtils.getCappedName( cl,
                            textIndexName ) );
                    esIndexNames.add( FullTextDBUtils.getESIndexName( cl,
                            textIndexName ) );
                    DropTextIndexThread dropTextIndexThread = new DropTextIndexThread(
                            csName, clName );
                    dropTextIndexThreads.add( dropTextIndexThread );
                    DropCLThread dropCLThread = new DropCLThread( csName,
                            clName );
                    dropCLThreads.add( dropCLThread );
                    // 删除全文索引
                    te.addWorker( dropTextIndexThread );
                    // 删除集合，且集合中存在全文索引
                    te.addWorker( dropCLThread );
                } else {
                    // 不存在全文索引的集合上创建索引
                    te.addWorker( new CreateTextIndexThread( csName, clName ) );
                }
            }
        }

        // 创建新集合
        te.addWorker( new CreateCLThread( csName, "12127_new_cl" ) );

        te.run();

        for ( int i = 0; i < dropCLThreads.size(); i++ ) {
            // 集合依然存在的情况下，且全文索引删除成功，执行全文检索报错
            Assert.assertTrue( FullTextUtils.isIndexDeleted( db, esClient,
                    esIndexNames.get( i ), cappedNames.get( i ) ) );
            if ( dropCLThreads.get( i ).getRetCode() != 0
                    && dropTextIndexThreads.get( i ).getRetCode() == 0 ) {
                DBCollection cl = cls.get( i * 2 + 1 );
                FullTextDBUtils.insertData( cl, 100 );
                BSONObject matcher = ( BSONObject ) JSON
                        .parse( "{'':{'$Text':{'query':{'match_all':{}}}}}" );
                DBCursor cursor = cl.query( matcher, null, null, null );

                try {
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
            // 集合依然存在的情况下，且全文索引删除失败，全文检索结果正确
            else if ( dropCLThreads.get( i ).getRetCode() != 0
                    && dropTextIndexThreads.get( i ).getRetCode() != 0 ) {
                int count = 0;
                DBCursor cursor = null;
                try {
                    DBCollection cl = cls.get( i * 2 + 1 );
                    BSONObject matcher = ( BSONObject ) JSON.parse(
                            "{'':{'$Text':{'query':{'match_all':{}}}}}" );
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
            }
        }

        FullTextUtils.isIndexCreated( esClient, cls.get( 0 ), textIndexName,
                10000 );
        FullTextUtils.isIndexCreated( esClient, cls.get( 2 ), textIndexName,
                10000 );
    }

    class DropTextIndexThread extends ResultStore {
        private String csName;
        private String clName;

        public DropTextIndexThread( String csName, String clName ) {
            this.csName = csName;
            this.clName = clName;
        }

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
                if ( -147 != e.getErrorCode() && -23 != e.getErrorCode() ) {
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

    class CreateTextIndexThread {
        private String csName;
        private String clName;

        public CreateTextIndexThread( String csName, String clName ) {
            this.csName = csName;
            this.clName = clName;
        }

        @ExecuteOrder(step = 1, desc = "创建全文索引")
        public void createTextIndex() {
            System.out.println(
                    this.getClass().getName().toString() + " begin at:"
                            + new SimpleDateFormat( "yyyy-MM-dd HH:mm:ss" )
                                    .format( new Date() ) );

            try ( Sequoiadb sdb = new Sequoiadb( SdbTestBase.coordUrl, "",
                    "" )) {
                DBCollection cl = sdb.getCollectionSpace( csName )
                        .getCollection( clName );
                BSONObject indexObj = new BasicBSONObject();
                indexObj.put( "a", "text" );
                cl.createIndex( textIndexName, indexObj, false, false );
            } finally {
                System.out.println(
                        this.getClass().getName().toString() + " end at:"
                                + new SimpleDateFormat( "yyyy-MM-dd HH:mm:ss" )
                                        .format( new Date() ) );
            }
        }
    }

    class CreateCLThread {
        private String csName = null;
        private String clName = null;

        public CreateCLThread( String csName, String clName ) {
            this.csName = csName;
            this.clName = clName;
        }

        @ExecuteOrder(step = 1, desc = "创建集合")
        public void createCL() {
            System.out.println(
                    this.getClass().getName().toString() + " begin at:"
                            + new SimpleDateFormat( "yyyy-MM-dd HH:mm:ss" )
                                    .format( new Date() ) );
            try ( Sequoiadb sdb = new Sequoiadb( SdbTestBase.coordUrl, "",
                    "" )) {
                sdb.getCollectionSpace( csName ).createCollection( clName );
            } finally {
                System.out.println(
                        this.getClass().getName().toString() + " end at:"
                                + new SimpleDateFormat( "yyyy-MM-dd HH:mm:ss" )
                                        .format( new Date() ) );
            }
        }
    }

    class DropCLThread extends ResultStore {
        private String csName = null;
        private String clName = null;

        public DropCLThread( String csName, String clName ) {
            db = new Sequoiadb( SdbTestBase.coordUrl, "", "" );
            this.csName = csName;
            this.clName = clName;
        }

        @ExecuteOrder(step = 1, desc = "删除集合")
        public void dropCL() {
            System.out.println(
                    this.getClass().getName().toString() + " begin at:"
                            + new SimpleDateFormat( "yyyy-MM-dd HH:mm:ss" )
                                    .format( new Date() ) );
            try ( Sequoiadb sdb = new Sequoiadb( SdbTestBase.coordUrl, "",
                    "" )) {
                sdb.getCollectionSpace( csName ).dropCollection( clName );
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
}