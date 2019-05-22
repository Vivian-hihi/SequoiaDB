/**
 * Copyright (c) 2019, SequoiaDB Ltd.
 * File Name:Fulltext15843.java
 * 删除全文索引与增删改记录并发
 *
 *  @author liuxiaoxuan
 * Date:2019年5月10日上午11:33:44
 *  @version 1.00
 */
package com.sequoiadb.fulltextparallel;

import java.util.ArrayList;
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
import com.sequoiadb.utils.StringUtils;
import org.elasticsearch.client.*;

/**
 * FileName Fulltext15843.java test content: 删除全文索引与增删改记录并发
 * 
 * @author liuxiaoxuan
 * @Date 2019.05.10
 */
public class Fulltext15843 extends SdbTestBase {
    private Sequoiadb db = null;
    private CollectionSpace cs = null;
    private DBCollection cl = null;
    private String clName = "ES_15843";
    private String textIndexName = "fulltext15843";
    private Client esClient = null;
    ThreadExecutor te = new ThreadExecutor();
    DropTextIndexThread dropIndexThread;
    CurdThread curdThread;

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
        if ( null != curdThread ) {
            curdThread.tearDown();
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

        // get capped name
        String cappedName = FullTextDBUtils.getCappedName( cl, textIndexName );
        // get es index name
        List< String > esIndexNames = FullTextDBUtils.getESIndexNames( cl,
                textIndexName );

        dropIndexThread = new DropTextIndexThread( csName, clName );
        curdThread = new CurdThread( csName, clName );

        te.addWorker( dropIndexThread );
        te.addWorker( curdThread );

        te.run();

        int errorcode = dropIndexThread.getRetCode();
        System.out.println( "test() errorcode:　" + errorcode );

        if ( 0 != errorcode ) {
            Assert.assertTrue( FullTextUtils.isIndexCreated( esClient, cl,
                    textIndexName, ( int ) cl.getCount() ) );

            // query
            BSONObject matcher = ( BSONObject ) JSON.parse(
                    "{'':{'$Text':{'query':{'match':{'a' : 'fulltext15843_after_update'}}}}}" );
            DBCursor cursor = cl.query( matcher, null, null, null );
            int count = 0;
            while ( cursor.hasNext() ) {
                BSONObject object = ( BSONObject ) cursor.getNext();
                count++;
            }
            System.out.println( csName + "." + clName + "'s count: " + count );
        } else {
            Assert.assertTrue( FullTextUtils.isIndexDeleted( db, esClient,
                    esIndexNames.get( 0 ), cappedName ) );

            // query after drop fulltext
            DBCursor cursor = null;
            try {
                BSONObject matcher = ( BSONObject ) JSON.parse(
                        "{'':{'$Text':{'query':{'match':{'a' : 'fulltext15843_after_update'}}}}}" );
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
            // when processing cappedcl data, maybe return -147
            try {
                cl.dropIndex( textIndexName );
            } catch ( BaseException e ) {
                boolean isExpectedErr = false;
                if ( -147 == e.getErrorCode() || -190 == e.getErrorCode() ) {
                    isExpectedErr = true;
                    saveResult( e.getErrorCode(), e );
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

    class CurdThread {
        private Sequoiadb db = null;
        private DBCollection cl = null;

        public CurdThread( String csName, String clName ) {
            db = new Sequoiadb( SdbTestBase.coordUrl, "", "" );
            cl = db.getCollectionSpace( csName ).getCollection( clName );
        }

        @ExecuteOrder(step = 1, desc = "往原始集合插入数据")
        public void insert() {
            System.out.println(
                    "--------------run CurdThread insert--------------" );
            List< BSONObject > insertObjs = new ArrayList< BSONObject >();
            int insertRecordNum = 10000;
            String strA = StringUtils.getRandomString( 64 );
            for ( int i = 0; i < insertRecordNum; i++ ) {
                insertObjs.add( ( BSONObject ) JSON.parse( "{ a: '" + strA
                        + "', b: 'new_insert_15843_" + i + "'}" ) );
            }
            cl.insert( insertObjs, 0 );
        }

        @ExecuteOrder(step = 1, desc = "更新全文索引记录")
        public void update() {
            System.out.println(
                    "--------------run CurdThread update--------------" );
            BSONObject modifier = new BasicBSONObject();
            BSONObject value = new BasicBSONObject();
            BSONObject matcher = new BasicBSONObject();
            BSONObject subMatcher = new BasicBSONObject();
            value.put( "a", "fulltext15843_after_update" );
            modifier.put( "$set", value );
            subMatcher.put( "$lt", 500 );
            matcher.put( "id", subMatcher );
            cl.update( matcher, modifier, null );
        }

        @ExecuteOrder(step = 1, desc = "删除全文索引记录")
        public void delete() {
            System.out.println(
                    "--------------run CurdThread delete--------------" );
            BSONObject matcher = new BasicBSONObject();
            BSONObject subMatcher = new BasicBSONObject();
            subMatcher.put( "$gt", 5000 );
            matcher.put( "id", subMatcher );
            cl.delete( matcher );
        }

        public void tearDown() {
            if ( null != db && !db.isClosed() ) {
                db.close();
            }
        }
    }
}
