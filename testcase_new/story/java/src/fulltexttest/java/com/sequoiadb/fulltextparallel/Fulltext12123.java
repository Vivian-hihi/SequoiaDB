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
 * @FileName: seqDB-12123:删除集合空间与创建/删除全文索引并发
 * @Author zhaoyu
 * @Date 2019-05-10
 */

public class Fulltext12123 extends SdbTestBase {
    private Sequoiadb sdb = null;
    private List<String> csNames = new ArrayList<String>();
    private List<String> clNames = new ArrayList<String>();
    private String csBasicName = "cs12123";
    private String clBasicName = "cl12123";
    private int csNum = 2;
    private int clNum = 4;
    private String indexName = "fulltext12123";
    private Client esClient = null;
    private int insertNum = 20000;
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
            for ( int i = 0; i < clNum; i++ ) {
                DBCollection cl = cs.createCollection( clNames.get( i ) );
                cl.createIndex( "id", "{id:1}", false, false );
                insertRecord( cl, insertNum );
                if ( i < clNum / 2 ) {
                    cl.createIndex( indexName, "{a:'text',b:'text'}", false, false );
                }
            }
        }
    }

    @AfterClass
    public void tearDown() throws Exception {
        try {
            List<String> esIndexNames = new ArrayList<String>();
            List<String> cappedCLNames = new ArrayList<String>();
            for ( String csName : csNames ) {
                if ( sdb.isCollectionSpaceExist( csName ) ) {
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
        // 获取原始集合所在组及固定集合名，作为后续结果校验的输入
        List<String> cappedCLNames = new ArrayList<>();
        List<String> esIndexNames = new ArrayList<>();
        for ( String csName : csNames ) {
            CollectionSpace cs = sdb.getCollectionSpace( csName );
            for ( int i = 0; i < clNum / 2; i++ ) {
                DBCollection cl = cs.getCollection( clNames.get( i ) );
                String cappedCLName = FullTextDBUtils.getCappedName( cl, indexName );
                cappedCLNames.add( cappedCLName );
                List<String> esIndexName = FullTextDBUtils.getESIndexNames( cl, indexName );
                esIndexNames.addAll( esIndexName );
            }
        }

        // 执行并发测试
        for ( String csName : csNames ) {
            for ( int i = 0; i < clNum; i++ ) {
                if ( i < clNum / 2 ) {
                    te.addWorker( new DropFullIndexThread( csName, clNames.get( i ) ) );
                } else {
                    te.addWorker( new CreateFullIndexThread( csName, clNames.get( i ) ) );
                }
            }
        }

        te.addWorker( new DropCS() );
        te.run();

        // 由于线程同时创建/删除了多个全文索引，无法通过返回值来判断预期结果；
        // 如果集合存在，则原始集合与ES端数据一致，主备节点数据一致
        // 如果集合不存在，则对应固定集合也被删除，无固定集合残留
        for ( int i = 0; i < csNames.size(); i++ ) {
            if ( sdb.isCollectionSpaceExist( csNames.get( i ) ) ) {
                CollectionSpace cs = sdb.getCollectionSpace( csNames.get( i ) );
                for ( int j = 0; j < clNum; j++ ) {
                    DBCollection cl = cs.getCollection( clNames.get( j ) );
                    if ( cs.isCollectionExist( clNames.get( j ) ) ) {
                        if ( cl.isIndexExist( indexName ) ) {
                            Assert.assertTrue( FullTextUtils.isIndexCreated( esClient, cl, indexName, insertNum ) );
                        }
                    }
                    // 只校验删除全文索引成功时，固定集合删除成功的逻辑；成功的逻辑在if分支已校验
                    if ( ( i + 1 ) * j + 1 <= cappedCLNames.size() ) {
                        System.out.println( "cappedCLNames.get((i + 1) * j):" + cappedCLNames.get( ( i + 1 ) * j ) );
                        System.out.println( "esIndexNames.get((i + 1) * j):" + esIndexNames.get( ( i + 1 ) * j ) );

                        Assert.assertTrue( FullTextUtils.isIndexDeleted( sdb, esClient,
                                esIndexNames.get( ( i + 1 ) * j ), cappedCLNames.get( ( i + 1 ) * j ) ) );
                    }
                }
            }
        }
    }

    private class DropFullIndexThread {
        private Sequoiadb db = new Sequoiadb( SdbTestBase.coordUrl, "", "" );
        private String csName = null;
        private String clName = null;

        public DropFullIndexThread( String csName, String clName ) {
            super();
            this.csName = csName;
            this.clName = clName;
        }

        @ExecuteOrder(step = 1, desc = "删除全文索引")
        public void dropFullIndex() {
            System.out.println( this.getClass().getName().toString() + " start at:" + df.format( new Date() ) );
            try {
                DBCollection cl = db.getCollectionSpace( csName ).getCollection( clName );
                cl.dropIndex( indexName );
            } catch ( BaseException e ) {
                if ( e.getErrorCode() != -34 && e.getErrorCode() != -23 && e.getErrorCode() != -248 ) {
                    e.printStackTrace();
                    Assert.fail( e.getMessage() );
                }
            } finally {
                db.close();
            }

            System.out.println( this.getClass().getName().toString() + " stop at:" + df.format( new Date() ) );
        }
    }

    private class CreateFullIndexThread {
        private Sequoiadb db = new Sequoiadb( SdbTestBase.coordUrl, "", "" );
        private String csName = null;
        private String clName = null;

        public CreateFullIndexThread( String csName, String clName ) {
            super();
            this.csName = csName;
            this.clName = clName;
        }

        @ExecuteOrder(step = 1, desc = "创建全文索引")
        public void createFullIndex() {
            System.out.println( this.getClass().getName().toString() + " start at:" + df.format( new Date() ) );
            try {
                DBCollection cl = db.getCollectionSpace( csName ).getCollection( clName );
                cl.createIndex( indexName, "{a:'text',b:'text'}", false, false );
            } catch ( BaseException e ) {
                if ( e.getErrorCode() != -34 && e.getErrorCode() != -23 && e.getErrorCode() != -248 ) {
                    e.printStackTrace();
                    Assert.fail( e.getMessage() );
                }
            } finally {
                db.close();
            }

            System.out.println( this.getClass().getName().toString() + " stop at:" + df.format( new Date() ) );
        }

    }

    private class DropCS {
        private Sequoiadb db = new Sequoiadb( SdbTestBase.coordUrl, "", "" );

        @ExecuteOrder(step = 1, desc = "删除集合")
        public void dropCS() {
            try {
                System.out.println( this.getClass().getName().toString() + " start at:" + df.format( new Date() ) );
                db.dropCollectionSpace( csNames.get( 0 ) );
                System.out.println( this.getClass().getName().toString() + " stop at:" + df.format( new Date() ) );
            } catch ( BaseException e ) {
                if ( e.getErrorCode() != -147 && e.getErrorCode() != -190 ) {
                    e.printStackTrace();
                    Assert.fail( e.getMessage() );
                }
            } finally {
                db.close();
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
