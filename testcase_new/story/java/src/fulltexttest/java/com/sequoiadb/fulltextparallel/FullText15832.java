package com.sequoiadb.fulltextparallel;

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
 * @FileName seqDB-15832:删除全文索引与删除集合并发
 * @Author yinzhen
 * @Date 2019-4-30
 */
public class FullText15832 extends SdbTestBase {
    private String clName = "cl15832";
    private Sequoiadb sdb;
    private DBCollection cl;
    private String fullIdxName = "idx15832";
    private Client esClient;
    private String cappedCLName;
    private String esIndexName;

    @BeforeClass
    public void setUp() throws Exception {
        sdb = new Sequoiadb( SdbTestBase.coordUrl, "", "" );
        if ( CommLib.isStandAlone( sdb ) ) {
            throw new SkipException( "STANDALONE MODE" );
        }

        // 创建全文索引
        esClient = FullTextESUtils.createTransportClient( SdbTestBase.esHostName,
                Integer.parseInt( SdbTestBase.esServiceName ) );
        cl = sdb.getCollectionSpace( csName ).createCollection( clName );
        FullTextDBUtils.insertData( cl, 20000 );
        cl.createIndex( fullIdxName, "{'a':'text','b':'text','c':'text', 'd':'text', 'e':'text', 'f':'text'}", false,
                false );
        Assert.assertTrue( FullTextUtils.isIndexCreated( esClient, cl, fullIdxName, 20000 ) );

        esIndexName = FullTextDBUtils.getESIndexName( cl, fullIdxName );
        cappedCLName = FullTextDBUtils.getCappedName( cl, fullIdxName );
    }

    @Test
    public void test() throws Exception {
        ThreadExecutor thExecutor = new ThreadExecutor( 600000 );
        thExecutor.addWorker( new DropFullIdx() );
        thExecutor.addWorker( new DropCL() );

        thExecutor.run();

        // 原始集合及固定集合均被删除成功，ES上全文索引删除成功，主备节点数据一致，无数据文件残留
        Assert.assertFalse( sdb.getCollectionSpace( csName ).isCollectionExist( clName ) );
        Assert.assertTrue( FullTextUtils.isIndexDeleted( sdb, esClient, esIndexName, cappedCLName ) );
    }

    @AfterClass
    public void tearDown() throws Exception {
        try {
            CollectionSpace cs = sdb.getCollectionSpace( csName );
            if ( cs.isCollectionExist( clName ) ) {
                cs.dropCollection( clName );
            }
            Assert.assertTrue( FullTextUtils.isIndexDeleted( sdb, esClient, esIndexName, cappedCLName ) );
        } finally {
            if ( sdb != null ) {
                sdb.close();
            }
            if ( esClient != null ) {
                esClient.close();
            }
        }
    }

    private class DropFullIdx {
        @ExecuteOrder(step = 1, desc = "删除全文索引")
        private void dropFullIdx() {
            Sequoiadb db = null;
            try {
                db = new Sequoiadb( coordUrl, "", "" );
                DBCollection cl = db.getCollectionSpace( csName ).getCollection( clName );
                cl.dropIndex( fullIdxName );
            } catch ( BaseException e ) {
                Assert.assertEquals( e.getErrorCode(), -23 );
            } finally {
                if ( db != null ) {
                    db.close();
                }
            }
        }
    }

    private class DropCL {
        @ExecuteOrder(step = 1, desc = "删除集合")
        private void dropCL() {
            Sequoiadb db = null;
            try {
                db = new Sequoiadb( coordUrl, "", "" );
                db.getCollectionSpace( csName ).dropCollection( clName );
            } finally {
                if ( db != null ) {
                    db.close();
                }
            }
        }
    }
}