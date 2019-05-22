/**
 * Copyright (c) 2019, SequoiaDB Ltd.
 * File Fulltext12122.java
 * 部分集合上存在全文索引，多个集合同时执行增删改/truncate/lob操作
 *
 *  @author liuxiaoxuan
 * Date:2019年5月10日上午11:33:44
 *  @version 1.00
 */
package com.sequoiadb.fulltextparallel;
// TODO:用例批注不需要符合规范
import java.util.ArrayList;
import java.util.List;
import java.util.Random;

import org.bson.BSONObject;
import org.bson.BasicBSONObject;
import org.bson.util.JSON;
import org.testng.Assert;
import org.testng.SkipException;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import com.sequoiadb.base.CollectionSpace;
import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.DBCursor;
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
import com.sequoiadb.utils.StringUtils;

import org.elasticsearch.client.*;

public class Fulltext12122 extends SdbTestBase {
    private Sequoiadb db = null;
    private CollectionSpace cs1 = null;
    private CollectionSpace cs2 = null;
    private DBCollection cl1 = null;
    private DBCollection cl2 = null;
    private DBCollection cl3 = null;
    private DBCollection cl4 = null;

    private String csName1 = "cs12122_01";
    private String csName2 = "cs12122_02";
    private String clName1 = "ES_12122_cl1_01";
    private String clName2 = "ES_12122_cl1_02";
    private String clName3 = "ES_12122_cl2_01";
    private String clName4 = "ES_12122_cl2_02";

    private String textIndexName = "fulltext12122";
    private Client esClient = null;
    ThreadExecutor te = new ThreadExecutor();
    List< TruncateThread > truncateThreads = new ArrayList<>();
    List< LobThread > lobThreads = new ArrayList<>();
    List< CurdThread > curdThreads = new ArrayList<>();

    @BeforeClass
    public void setUp() {
        esClient = FullTextESUtils.createTransportClient( esHostName,
                Integer.parseInt( esServiceName ) );

        db = new Sequoiadb( SdbTestBase.coordUrl, "", "" );
        if ( CommLib.isStandAlone( db ) ) {
            throw new SkipException( "skip StandAlone" );
        }

        // create cl
        if ( db.isCollectionSpaceExist( csName1 ) ) {
            db.dropCollectionSpace( csName1 );
        }
        if ( db.isCollectionSpaceExist( csName2 ) ) {
            db.dropCollectionSpace( csName2 );
        }
        cs1 = db.createCollectionSpace( csName1 );
        cs2 = db.createCollectionSpace( csName2 );
        createCollections();
    }

    @AfterClass
    public void tearDown() {
        for ( CurdThread thread : curdThreads ) {
            if ( null != thread ) {
                thread.tearDown();
            }
        }

        for ( LobThread thread : lobThreads ) {
            if ( null != thread ) {
                thread.tearDown();
            }
        }

        for ( TruncateThread thread : truncateThreads ) {
            if ( null != thread ) {
                thread.tearDown();
            }
        }

        db.dropCollectionSpace( csName1 );
        db.dropCollectionSpace( csName2 );
        db.close();
        esClient.close();
    }

    @Test  //TODO:测试点需要梳理，并发前的操作可以放到 setUp
    public void test() throws Exception {
        // insert
        FullTextDBUtils.insertData( cl1, 10000 );
        FullTextDBUtils.insertData( cl2, 10000 );
        FullTextDBUtils.insertData( cl3, 10000 );
        FullTextDBUtils.insertData( cl4, 10000 );
        // create fulltext
        BSONObject indexObj = new BasicBSONObject();
        indexObj.put( "a", "text" );
        cl1.createIndex( textIndexName, indexObj, false, false );
        cl3.createIndex( textIndexName, indexObj, false, false );
        
        Assert.assertTrue( FullTextUtils.isIndexCreated( esClient, cl1,
                textIndexName, 10000 ) );
        Assert.assertTrue( FullTextUtils.isIndexCreated( esClient, cl3,
                textIndexName, 10000 ) );
        
        // get _cllid from ES before truncate
        String esIndexNames1 = FullTextDBUtils.getESIndexName( cl1,
                textIndexName );
        int preCLLid1 = FullTextESUtils.getCommitCLLIDFromES( esClient,
                esIndexNames1 );

        // truncate cl
        truncateThreads.add( new TruncateThread( csName1, clName1 ) );
        truncateThreads.add( new TruncateThread( csName2, clName4 ) );
        lobThreads.add( new LobThread( csName1, clName1 ) );
        lobThreads.add( new LobThread( csName2, clName4 ) );
        curdThreads.add( new CurdThread( csName1, clName1 ) );
        curdThreads.add( new CurdThread( csName1, clName2 ) );
        curdThreads.add( new CurdThread( csName2, clName3 ) );
        curdThreads.add( new CurdThread( csName2, clName4 ) );

        for ( TruncateThread thread : truncateThreads ) {
            te.addWorker( thread );
        }
        for ( LobThread thread : lobThreads ) {
            te.addWorker( thread );
        }
        for ( CurdThread thread : curdThreads ) {
            te.addWorker( thread );
        }

        // concurrent run
        te.run();

        Assert.assertTrue( FullTextUtils.isFulltextRebuild( esClient,
                esIndexNames1, preCLLid1 ) );
        Assert.assertTrue( FullTextUtils.isIndexCreated( esClient, cl1,
                textIndexName, ( int ) cl1.getCount() ) );
        Assert.assertTrue( FullTextUtils.isIndexCreated( esClient, cl3,
                textIndexName, ( int ) cl3.getCount() ) );

        // query
        BSONObject matcher = ( BSONObject ) JSON.parse(
                "{'':{'$Text':{'query':{'match':{'a' : 'cs12122_01'}}}}}" );
        DBCursor cursor = cl1.query( matcher, null, null, null );
        int count = 0;
        while ( cursor.hasNext() ) {
            BSONObject object = ( BSONObject ) cursor.getNext();// TODO:未使用变量，不需要定义
            count++;
        }
        System.out.println( csName1 + "." + clName1 + "'s count: " + count );
    }

    public void createCollections() {
        cl1 = cs1.createCollection( clName1 );
        cl2 = cs1.createCollection( clName2 );
        cl3 = cs2.createCollection( clName3 );
        cl4 = cs2.createCollection( clName4 );
    }

    class TruncateThread {
        private Sequoiadb db = null;
        private DBCollection cl = null;

        public TruncateThread( String csName, String clName ) {
            db = new Sequoiadb( SdbTestBase.coordUrl, "", "" );
            cl = db.getCollectionSpace( csName ).getCollection( clName );
        }

        @ExecuteOrder(step = 1, desc = "清空原始集合")
        public void truncate() {
            try {
                cl.truncate();
            } catch ( BaseException e ) {
                boolean isExpectedErr = false;
                if ( -321 == e.getErrorCode() || -190 == e.getErrorCode() ) {
                    isExpectedErr = true;
                }
                Assert.assertTrue( isExpectedErr,
                        "actual exception: " + e.getErrorCode() );
            }
        }

        public void tearDown() {
            if ( null != db && !db.isClosed() ) {
                db.close();
            }
        }
    }

    class LobThread {
        private Sequoiadb db = null;
        private DBCollection cl = null;

        public LobThread( String csName, String clName ) {
            db = new Sequoiadb( SdbTestBase.coordUrl, "", "" );
            cl = db.getCollectionSpace( csName ).getCollection( clName );
        }

        @ExecuteOrder(step = 1, desc = "插入lob")
        public void createLob() {
            DBLob lob = null;
            try {
                String lobStringBuff = StringUtils
                        .getRandomString( new Random().nextInt( 1024 ) );
                lob = cl.createLob();
                lob.write( lobStringBuff.getBytes() );
            } finally {
                if ( lob != null ) {
                    lob.close();
                }
            }
        }

        public void tearDown() {
            if ( null != db && !db.isClosed() ) {
                db.close();
            }
        }
    }

    class CurdThread { // TODO:增删改建议分开写线程并发
        private Sequoiadb db = null;
        private DBCollection cl = null;

        public CurdThread( String csName, String clName ) {
            db = new Sequoiadb( SdbTestBase.coordUrl, "", "" );
            cl = db.getCollectionSpace( csName ).getCollection( clName );
        }

        @ExecuteOrder(step = 1, desc = "插入记录")
        public void insert() {
            System.out.println(
                    "--------------run CurdThread insert--------------" );
            List< BSONObject > insertObjs = new ArrayList< BSONObject >();
            int insertRecordNum = 10000;
            String strA = StringUtils.getRandomString( 64 );
            for ( int i = 0; i < insertRecordNum; i++ ) {
                insertObjs.add( ( BSONObject ) JSON.parse( "{ a: '" + strA
                        + "', b: 'new_insert_12122_" + i + "'}" ) );
            }

            try {
                cl.insert( insertObjs, 0 );
            } catch ( BaseException e ) {
                boolean isExpectedErr = false;
                if ( -321 == e.getErrorCode() || -190 == e.getErrorCode() ) {
                    isExpectedErr = true;
                }
                Assert.assertTrue( isExpectedErr,
                        "actual exception: " + e.getErrorCode() );
            }
        }

        @ExecuteOrder(step = 1, desc = "更新记录")
        public void update() {
            System.out.println(
                    "--------------run CurdThread update--------------" );
            BSONObject modifier = new BasicBSONObject();
            BSONObject value = new BasicBSONObject();
            BSONObject matcher = new BasicBSONObject();
            BSONObject subMatcher = new BasicBSONObject();
            value.put( "a", "12122_after_update" );
            modifier.put( "$set", value );
            subMatcher.put( "$lt", 2000 );
            matcher.put( "id", subMatcher );

            try {
                cl.update( matcher, modifier, null );
            } catch ( BaseException e ) {
                boolean isExpectedErr = false;
                if ( -321 == e.getErrorCode() || -190 == e.getErrorCode() ) {
                    isExpectedErr = true;
                }
                Assert.assertTrue( isExpectedErr,
                        "actual exception: " + e.getErrorCode() );
            }
        }

        @ExecuteOrder(step = 1, desc = "删除记录")
        public void delete() {
            System.out.println(
                    "--------------run CurdThread delete--------------" );
            BSONObject matcher = new BasicBSONObject();
            BSONObject subMatcher = new BasicBSONObject();
            subMatcher.put( "$gt", 5000 );
            matcher.put( "id", subMatcher );

            try {
                cl.delete( matcher );
            } catch ( BaseException e ) {
                boolean isExpectedErr = false;
                if ( -321 == e.getErrorCode() || -190 == e.getErrorCode() ) {
                    isExpectedErr = true;
                }
                Assert.assertTrue( isExpectedErr,
                        "actual exception: " + e.getErrorCode() );
            }
        }

        public void tearDown() {
            if ( null != db && !db.isClosed() ) {
                db.close();
            }
        }
    }
}
