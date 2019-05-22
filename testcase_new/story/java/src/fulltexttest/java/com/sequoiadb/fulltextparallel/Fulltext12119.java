/**
 * Copyright (c) 2019, SequoiaDB Ltd.
 * File Name:Fulltext12119.java
 * 并发删除集合空间 
 *
 *  @author liuxiaoxuan
 * Date:2019年5月10日上午11:33:44
 *  @version 1.00
 */
package com.sequoiadb.fulltextparallel;
//TODO:用例批注不需要符合规范
import java.util.ArrayList;
import java.util.List;

import org.bson.BSONObject;
import org.bson.BasicBSONObject;
import org.testng.Assert;
import org.testng.SkipException;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import com.sequoiadb.base.CollectionSpace;
import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.testcommon.CommLib;
import com.sequoiadb.testcommon.SdbTestBase;
import com.sequoiadb.threadexecutor.ThreadExecutor;
import com.sequoiadb.threadexecutor.annotation.ExecuteOrder;
import com.sequoiadb.utils.FullTextDBUtils;
import com.sequoiadb.utils.FullTextESUtils;
import com.sequoiadb.utils.FullTextUtils;
import org.elasticsearch.client.*;

public class Fulltext12119 extends SdbTestBase {
    private Sequoiadb db = null;
    private CollectionSpace cs1 = null;
    private CollectionSpace cs2 = null;
    private DBCollection cl1 = null;
    private DBCollection cl2 = null;
    private DBCollection cl3 = null;
    private DBCollection cl4 = null;

    private String csName1 = "cs12119_01";
    private String csName2 = "cs12119_02";
    private String clName1 = "ES_12119_cl1_01";
    private String clName2 = "ES_12119_cl1_02";
    private String clName3 = "ES_12119_cl2_01";
    private String clName4 = "ES_12119_cl2_02";

    private String textIndexName = "fulltext12119";
    private Client esClient = null;
    ThreadExecutor te = new ThreadExecutor();
    List< DropCSThread > dropCSThreads = new ArrayList<>();

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
        for ( DropCSThread thread : dropCSThreads ) {
            if ( null != thread ) {
                thread.tearDown();
            }
        } // TODO:直接在线程里面关闭连接
        db.close();
        esClient.close();
    }

    @Test  //TODO：test步骤可以再梳理下，测试点“删除CS”前的准备工作都可以放到 setUp 里面
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

        // get capped name
        String cappedName1 = FullTextDBUtils.getCappedName( cl1,
                textIndexName );
        String cappedName2 = FullTextDBUtils.getCappedName( cl3,
                textIndexName );
        // get es index name
        List< String > esIndexNames1 = FullTextDBUtils.getESIndexNames( cl1,
                textIndexName );
        List< String > esIndexNames2 = FullTextDBUtils.getESIndexNames( cl3,
                textIndexName );

        dropCSThreads.add( new DropCSThread( csName1 ) );
        dropCSThreads.add( new DropCSThread( csName2 ) );

        for ( DropCSThread thread : dropCSThreads ) {
            te.addWorker( thread );
        }

        // concurrent run
        te.run();

        Assert.assertTrue( FullTextUtils.isIndexDeleted( db, esClient,
                esIndexNames1.get( 0 ), cappedName1 ) );
        Assert.assertTrue( FullTextUtils.isIndexDeleted( db, esClient,
                esIndexNames2.get( 0 ), cappedName2 ) );
    }

    public void createCollections() {
        cl1 = cs1.createCollection( clName1 );
        cl2 = cs1.createCollection( clName2 );
        cl3 = cs2.createCollection( clName3 );
        cl4 = cs2.createCollection( clName4 );
    }

    class DropCSThread {
        private Sequoiadb db = null;
        private String csName = null;

        public DropCSThread( String csName ) {
            db = new Sequoiadb( SdbTestBase.coordUrl, "", "" );
            this.csName = csName;
        }

        @ExecuteOrder(step = 1, desc = "删除集合空间")
        public void dropCS() {
            System.out
                    .println( "--------------run DropCSThread--------------" );
            db.dropCollectionSpace( csName );
        }

        public void tearDown() {
            if ( null != db && !db.isClosed() ) {
                db.close();
            } // TODO:step1 try后面直接new db，不需要tear down
        }
    }
}
