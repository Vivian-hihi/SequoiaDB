package com.sequoiadb.fulltextparallel;

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
import com.sequoiadb.testcommon.CommLib;
import com.sequoiadb.testcommon.SdbTestBase;
import com.sequoiadb.threadexecutor.ThreadExecutor;
import com.sequoiadb.threadexecutor.annotation.ExecuteOrder;
import com.sequoiadb.utils.FullTextDBUtils;
import com.sequoiadb.utils.FullTextESUtils;
import com.sequoiadb.utils.FullTextUtils;

/**
 * @FileName seqDB-15857:集合中存在全文索引，并发删除记录
 * @Author huangxiaoni
 * @Date 2019.5.8
 */
// TODO :检视意见同15856
public class FullText15857 extends SdbTestBase {
    private final int THREAD_NUM = 5;
    private final String CL_NAME = "cl_es_15857";
    private final String IDX_NAME = "idx_es_15857";
    private final BSONObject IDX_KEY = (BSONObject) JSON.parse( "{a:'text',b:'text',c:'text',d:'text'}" );
    private final int RECS_NUM = 50000;

    private Sequoiadb sdb = null;
    private CollectionSpace cs;
    private DBCollection cl;
    private String cappedCSName;

    private Client esClient = null;
    private String esIndexName;

    @BeforeClass
    private void setUp() throws Exception {
        esClient = FullTextESUtils.createTransportClient( esHostName, Integer.parseInt( esServiceName ) );
        sdb = new Sequoiadb( SdbTestBase.coordUrl, "", "" );

        if ( CommLib.isStandAlone( sdb ) ) {
            throw new SkipException( "Skip standAlone mode" );
        }

        cs = sdb.getCollectionSpace( SdbTestBase.csName );
        cl = cs.createCollection( CL_NAME );
        cl.createIndex( IDX_NAME, IDX_KEY, false, false );
        cappedCSName = FullTextDBUtils.getCappedName( cl, IDX_NAME );
        esIndexName = FullTextDBUtils.getESIndexName( cl, IDX_NAME );

        FullTextDBUtils.insertData( cl, RECS_NUM );

        // 确保预置的数据同步到es完成
        Assert.assertTrue( FullTextUtils.isIndexCreated( esClient, cl, IDX_NAME, RECS_NUM ) );
    }

    @Test
    private void test() throws Exception {
        // matcher1
        BSONObject obj1 = new BasicBSONObject( "$lt", RECS_NUM / 2 );
        BSONObject matcher1 = new BasicBSONObject( "recordId", obj1 );
        // matcher2
        BSONObject obj2 = new BasicBSONObject( "$gte", RECS_NUM / 2 );
        BSONObject matcher2 = new BasicBSONObject( "recordId", obj2 );
        // thread
        ThreadExecutor es = new ThreadExecutor();
        for ( int i = 0; i < THREAD_NUM; i++ ) {
            es.addWorker( new ThreadDelete( matcher1 ) );
            es.addWorker( new ThreadDelete( matcher2 ) );
        }
        es.run();

        // check consistency
        Assert.assertTrue( FullTextUtils.isIndexCreated( esClient, cl, IDX_NAME, 0 ) );
        // check total count
        long updCnt = cl.getCount();
        Assert.assertEquals( updCnt, 0 );
        // check fullTextSearch
        BSONObject matcher = new BasicBSONObject( "", new BasicBSONObject( "$Text",
                new BasicBSONObject( "query", new BasicBSONObject( "match_all", new BasicBSONObject() ) ) ) );
        DBCursor cursor = cl.query( matcher, null, null, null );
        Assert.assertFalse( cursor.hasNext() );
    }

    @AfterClass
    private void tearDown() throws Exception {
        try {
            FullTextDBUtils.dropCollection( cs, CL_NAME );
            Assert.assertTrue( FullTextUtils.isIndexDeleted( sdb, esClient, esIndexName, cappedCSName ) );
        } finally {
            if ( sdb != null ) {
                sdb.close();
            }
            if ( esClient != null ) {
                esClient.close();
            }
        }
    }

    private class ThreadDelete {
        private BSONObject matcher;

        private ThreadDelete( BSONObject matcher ) {
            this.matcher = matcher;
        }

        @ExecuteOrder(step = 1)
        private void delete() {
            System.out.println( new Date() + " " + this.getClass().getName().toString() );
            try ( Sequoiadb db = new Sequoiadb( SdbTestBase.coordUrl, "", "" ) ) {
                DBCollection cl2 = db.getCollectionSpace( SdbTestBase.csName ).getCollection( CL_NAME );
                cl2.delete( matcher );
            }
        }
    }
}
