package com.sequoiadb.fulltextparallel;

import java.util.concurrent.atomic.AtomicInteger;

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
 * @FileName seqDB-14414:同一集合并发创建删除相同的全文索引
 * @Author yinzhen
 * @Date 2019-4-28
 */
public class FullText14414 extends SdbTestBase {
    private String clName = "cl14414";
    private Sequoiadb sdb;
    private DBCollection cl;
    private String fullIdxName = "idx14414";
    private Client esClient;
    private String cappedCLName;
    private String esIndexName;
    private AtomicInteger atoint = new AtomicInteger( 0 );

    @BeforeClass
    public void setUp() {
        sdb = new Sequoiadb( SdbTestBase.coordUrl, "", "" );
        if ( CommLib.isStandAlone( sdb ) ) {
            throw new SkipException( "STANDALONE MODE" );
        }

        esClient = FullTextESUtils.createTransportClient( SdbTestBase.esHostName,
                Integer.parseInt( SdbTestBase.esServiceName ) );
        cl = sdb.getCollectionSpace( csName ).createCollection( clName );
        FullTextDBUtils.insertData( cl, 20000 );
    }

    @Test
    public void test() throws Exception {
        ThreadExecutor thExecutor = new ThreadExecutor( 600000 );
        for ( int i = 0; i < 10; i++ ) {
            thExecutor.addWorker( new CreateAndDropIdx() );
        }
        thExecutor.run();
        Assert.assertEquals( atoint.get(), 1 );

        // 主备节点上索引信息一致，固定集合、索引信息、ES端数据一致
        Assert.assertTrue( FullTextUtils.isIndexDeleted( sdb, esClient, esIndexName, cappedCLName ) );
        Assert.assertTrue( FullTextUtils.isCLDataConsistency( cl ) );
    }

    @AfterClass
    public void tearDown() throws Exception {
        try {
            CollectionSpace cs = sdb.getCollectionSpace( csName );
            cs.dropCollection( clName );
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

    private class CreateAndDropIdx {
        @ExecuteOrder(step = 1, desc = "多线程创建删除同一个全文索引")
        private void createFullIdx() {
            Sequoiadb db = null;
            try {
                db = new Sequoiadb( coordUrl, "", "" );
                DBCollection cl = db.getCollectionSpace( csName ).getCollection( clName );
                cl.createIndex( fullIdxName, "{'a':'text','b':'text','c':'text', 'd':'text', 'e':'text', 'f':'text'}",
                        false, false );

                cappedCLName = FullTextDBUtils.getCappedName( cl, fullIdxName );
                esIndexName = FullTextDBUtils.getESIndexName( cl, fullIdxName );
                cl.dropIndex( fullIdxName );
                atoint.incrementAndGet();
            } catch ( BaseException e ) {
                Assert.assertEquals( e.getErrorCode(), -42 );
            } finally {
                if ( db != null ) {
                    db.close();
                }
            }
        }
    }
}