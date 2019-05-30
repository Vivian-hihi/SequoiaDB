package com.sequoiadb.fulltextparallel;

import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Date;
import java.util.List;

import org.bson.BSONObject;
import org.bson.util.JSON;
import org.elasticsearch.client.Client;
import org.testng.Assert;
import org.testng.SkipException;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import com.sequoiadb.base.CollectionSpace;
import com.sequoiadb.base.DBCollection;
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
 * @FileName: seqDB-12126:并发删除不同集合上的全文索引
 * @Author zhaoyu
 * @Date 2019-05-13
 */
public class Fulltext12126 extends SdbTestBase {
    private Sequoiadb sdb = null;
    private List<String> csNames = new ArrayList<String>();
    private List<String> clNames = new ArrayList<String>();
    private String csBasicName = "cs12126";
    private String clBasicName = "cl12126";
    private int csNum = 2;
    private int clNum = 2;
    private String indexName = "fulltext12126";
    private Client esClient = null;
    private int insertNum = 50000;
    private ThreadExecutor te = new ThreadExecutor( 3600000 );
    private SimpleDateFormat df = new SimpleDateFormat( "yyyy-MM-dd HH:mm:ss.S" );

    @BeforeClass
    public void setUp() {
        esClient = FullTextESUtils.createTransportClient( esHostName, Integer.parseInt( esServiceName ) );
        sdb = new Sequoiadb( SdbTestBase.coordUrl, "", "" );
        if ( CommLib.isStandAlone( sdb ) ) {
            throw new SkipException( "skip StandAlone" );
        }
        for ( int i = 0; i < csNum; i++ ) {
            String csName = csBasicName + "_" + i;
            csNames.add( csName );
        }

        for ( int i = 0; i < clNum; i++ ) {
            String clName = clBasicName + "_" + i;
            clNames.add( clName );
        }
        for ( String csName : csNames ) {
            if ( sdb.isCollectionSpaceExist( csName ) ) {
                sdb.dropCollectionSpace( csName );
            }
            CollectionSpace cs = sdb.createCollectionSpace( csName );
            for ( String clName : clNames ) {
                DBCollection cl = cs.createCollection( clName );
                cl.createIndex( "id", "{id:1}", false, false );
                cl.createIndex( indexName, "{a:'text',b:'text'}", false, false );
                insertRecord( cl, insertNum );
            }
        }

    }

    @AfterClass
    public void tearDown() throws Exception {
        try {
            List<String> esIndexNames = new ArrayList<String>();
            List<String> cappedCLNames = new ArrayList<String>();
            for ( String csName : csNames ) {
                CollectionSpace cs = sdb.getCollectionSpace( csName );
                for ( String clName : clNames ) {
                    DBCollection cl = cs.getCollection( clName );
                    if ( cl.isIndexExist( indexName ) ) {
                        String esIndexName = FullTextDBUtils.getESIndexName( cl, indexName );
                        esIndexNames.add( esIndexName );
                        String cappedCLName = FullTextDBUtils.getCappedName( cl, indexName );
                        cappedCLNames.add( cappedCLName );
                    }
                }

            }
            for ( String csName : csNames ) {
                FullTextDBUtils.dropCollectionSpace( sdb, csName );
            }
            if ( !esIndexNames.isEmpty() && !cappedCLNames.isEmpty() ) {
                Assert.assertTrue( FullTextUtils.isIndexDeleted( sdb, esClient, esIndexNames, cappedCLNames ) );
            }
        } finally {
            sdb.close();
            esClient.close();
        }
    }

    @Test
    public void test() throws Exception {
        // 执行并发测试及结果校验
        for ( String csName : csNames ) {
            for ( String clName : clNames ) {
                te.addWorker( new DropFullIndexThread( csName, clName ) );
            }
        }
        te.run();
    }

    private class DropFullIndexThread {
        private String csName = null;
        private String clName = null;
        private String cappedCLName = null;
        private String esIndexName = null;

        public DropFullIndexThread( String csName, String clName ) {
            super();
            this.csName = csName;
            this.clName = clName;
        }

        @ExecuteOrder(step = 1, desc = "创建全文索引")
        public void dropFullIndex() {
            System.out.println( this.getClass().getName().toString() + " start at:" + df.format( new Date() ) );
            Sequoiadb db = new Sequoiadb( SdbTestBase.coordUrl, "", "" );
            try {
                DBCollection cl = db.getCollectionSpace( csName ).getCollection( clName );
                cappedCLName = FullTextDBUtils.getCappedName( cl, indexName );
                esIndexName = FullTextDBUtils.getESIndexName( cl, indexName );
                cl.dropIndex( indexName );
            } finally {
                db.close();
            }
            System.out.println( this.getClass().getName().toString() + " stop at:" + df.format( new Date() ) );
        }

        @ExecuteOrder(step = 2, desc = "结果校验")
        public void checkResult() throws Exception {
            Sequoiadb db = new Sequoiadb( SdbTestBase.coordUrl, "", "" );
            Client es = FullTextESUtils.createTransportClient( esHostName, Integer.parseInt( esServiceName ) );
            try {
                DBCollection cl = db.getCollectionSpace( csName ).getCollection( clName );

                // 固定集合及ES端的全文索引已删除成功
                Assert.assertTrue( FullTextUtils.isIndexDeleted( db, es, esIndexName, cappedCLName ) );

                // 全文检索数据报错-52、-6
                cl.query( "{'':{'$Text':{query:{match_all:{}}}}}", "{a:1,c:1}", null, null );
            } catch ( BaseException e ) {
                if ( e.getErrorCode() != -6 && e.getErrorCode() != -52 ) {
                    Assert.fail( e.getMessage() );
                }
            } finally {
                db.close();
                es.close();
            }
        }

    }

    public void insertRecord( DBCollection cl, int insertNums ) {
        List<BSONObject> insertObjs = new ArrayList<>();
        for ( int i = 0; i < 100; i++ ) {
            for ( int j = 0; j < insertNums / 100; j++ ) {
                int k = i * 100 + j;
                insertObjs.add( (BSONObject) JSON.parse( "{id:" + k + ",a: 'test_11981_" + i * 100 + j
                        + "', b: 'test_11981_aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa " + i * 100 + j + "'}" ) );
            }
            cl.insert( insertObjs, 0 );
            insertObjs.clear();
        }
    }
}
