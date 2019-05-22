/**
 * Copyright (c) 2019, SequoiaDB Ltd.
 * File Name:Fulltext15844.java
 * 删除全文索引与全文检索并发 
 *
 *  @author liuxiaoxuan
 * Date:2019年5月10日上午11:33:44
 *  @version 1.00
 */
package com.sequoiadb.fulltextparallel;

import java.util.List;

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
import org.elasticsearch.client.*;

public class Fulltext15844 extends SdbTestBase {
    private Sequoiadb db = null;
    private CollectionSpace cs = null;
    private DBCollection cl = null;
    private String clName = "ES_15844";
    private String textIndexName = "fulltext15844";
    private Client esClient = null;
    ThreadExecutor te = new ThreadExecutor();
    DropTextIndexThread dropIndexThread;
    QueryThread queryThread;

    @BeforeClass
    public void setUp() {
        esClient = FullTextESUtils.createTransportClient( esHostName,
                Integer.parseInt( esServiceName ) );

        db = new Sequoiadb( SdbTestBase.coordUrl, "", "" );
        if ( CommLib.isStandAlone( db ) ) {
            throw new SkipException( "skip StandAlone" );
        }

        // create cl
        cs = db.getCollectionSpace( csName );
        cl = cs.createCollection( clName );
    }

    @AfterClass
    public void tearDown() {
        if ( null != dropIndexThread ) {
            dropIndexThread.tearDown();
        }
        if ( null != queryThread ) {
            queryThread.tearDown();
        }
        FullTextDBUtils.dropCollection( cs, clName );
        db.close();
        esClient.close();
    }

    @Test
    public void test() throws Exception {
        // insert
        FullTextDBUtils.insertData( cl, 10000 );
        // create fulltext
        BSONObject indexObj = new BasicBSONObject();
        indexObj.put( "a", "text" );
        cl.createIndex( textIndexName, indexObj, false, false );
        Assert.assertTrue( FullTextUtils.isIndexCreated( esClient, cl,
                textIndexName, 10000 ) );

        // get capped name
        String cappedName = FullTextDBUtils.getCappedName( cl, textIndexName );
        // get es index name
        List< String > esIndexNames = FullTextDBUtils.getESIndexNames( cl,
                textIndexName );

        dropIndexThread = new DropTextIndexThread( csName, clName );
        queryThread = new QueryThread( csName, clName );

        te.addWorker( dropIndexThread );
        te.addWorker( queryThread );
        // concurrent run
        te.run();

        int errorcode = queryThread.getRetCode();
        System.out.println( "test() errorcode:　" + errorcode );

        Assert.assertTrue( FullTextUtils.isIndexDeleted( db, esClient,
                esIndexNames.get( 0 ), cappedName ) );

        // query after drop fulltext
        DBCursor cursor = null;
        try {
            BSONObject matcher = ( BSONObject ) JSON.parse(
                    "{'':{'$Text':{'query':{'match':{'a' : 'fulltext15844_after_update'}}}}}" );
            cursor = cl.query( matcher, null, null, null );
            Assert.fail( "query should fail" );
        } catch ( BaseException e ) {
            Assert.assertEquals( e.getErrorCode(), -52,
                    "actual exception: " + e.getErrorCode() );
        } finally {
            if ( cursor != null ) {
                cursor.close();
            }
        }
    }

    class DropTextIndexThread extends ResultStore {
        private Sequoiadb db = null;
        private DBCollection cl = null;

        public DropTextIndexThread( String csName, String clName ) {
            db = new Sequoiadb( SdbTestBase.coordUrl, "", "" );
            cl = db.getCollectionSpace( csName ).getCollection( clName );
        }

        @ExecuteOrder(step = 1, desc = "删除全文索引")
        public void dropTextIndex() {
            System.out.println(
                    "--------------run DropTextIndexThread--------------" );
            cl.dropIndex( textIndexName );
        }

        public void tearDown() {
            if ( null != db && !db.isClosed() ) {
                db.close();
            }
        }
    }

    class QueryThread extends ResultStore {
        private Sequoiadb db = null;
        private DBCollection cl = null;

        public QueryThread( String csName, String clName ) {
            db = new Sequoiadb( SdbTestBase.coordUrl, "", "" );
            cl = db.getCollectionSpace( csName ).getCollection( clName );
        }

        @ExecuteOrder(step = 1, desc = "執行全文检索")
        public void query() {
            System.out.println(
                    "--------------run QueryThread insert--------------" );
            DBCursor cursor = null;
            BSONObject matcher = ( BSONObject ) JSON.parse(
                    "{'':{'$Text':{'query':{'match':{'a' : 'fulltext15844_after_update'}}}}}" );
            try {
                cursor = cl.query( matcher, null, null, null );
                int count = 0;
                while ( cursor.hasNext() ) {
                    BSONObject object = ( BSONObject ) cursor.getNext();
                    count++;
                }
                System.out.println(
                        csName + "." + clName + "'s count: " + count );
            } catch ( BaseException e ) {
                saveResult( e.getErrorCode(), e );
                Assert.assertEquals( e.getErrorCode(), -52,
                        "actual exception: " + e.getErrorCode() );
            } finally {
                cursor.close();
            }
        }

        public void tearDown() {
            if ( null != db && !db.isClosed() ) {
                db.close();
            }
        }
    }
}
