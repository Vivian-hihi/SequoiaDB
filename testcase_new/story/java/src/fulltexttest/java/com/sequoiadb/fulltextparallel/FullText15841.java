package com.sequoiadb.fulltextparallel;

import java.util.Random;

import org.bson.BSONObject;
import org.bson.BasicBSONObject;
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
 * @FileName FullText15841.java 创建全文索引与truncate并发
 * @Author luweikang
 * @Date 2019年5月10日
 */
public class FullText15841 extends SdbTestBase {

    private Sequoiadb sdb = null;
    private CollectionSpace cs = null;
    private DBCollection cl = null;
    private String clName = "es_15841";
    private Client esClient = null;
    private String indexName = "fulltextIndex15841";
    private String cappedName = null;
    private String esIndexName = null;
    private int insertNum = 100000;

    @BeforeClass
    public void setUp() {
        esClient = FullTextESUtils.createTransportClient( esHostName, Integer.parseInt( esServiceName ) );
        sdb = new Sequoiadb( SdbTestBase.coordUrl, "", "" );
        if ( CommLib.isStandAlone( sdb ) ) {
            throw new SkipException( "skip StandAlone" );
        }
        cs = sdb.getCollectionSpace( csName );
        cl = cs.createCollection( clName );
    }

    @Test
    public void test() throws Exception {//TODO: 同 15838 用例检视意见

        FullTextDBUtils.insertData( cl, insertNum );

        ThreadExecutor thread = new ThreadExecutor();
        thread.addWorker( new CreateIndexThread() );
        thread.addWorker( new TruncateThread() );
        thread.run();
//TODO：创建索引可能成功可能失败，需要分情况检查结果
//TODO：如果创建索引成功，检查结果前是不是要判断一下索引是否意见重建？FullTextUtils.isFulltextRebuild
//TODO：如果truncate失败，需要校验记录数、索引、数据一致性
        Assert.assertTrue( FullTextUtils.isIndexCreated( esClient, cl, indexName, insertNum ) );
        
        cappedName = FullTextDBUtils.getCappedName( cl, indexName );
        esIndexName = FullTextDBUtils.getESIndexName( cl, indexName );
    }

    @AfterClass
    public void tearDown() {
        try {
            FullTextDBUtils.dropCollection( cs, clName );
            Assert.assertTrue( FullTextUtils.isIndexDeleted( sdb, esClient, esIndexName, cappedName ) );
        } finally {
            if ( sdb != null ) {
                sdb.close();
            }
            if ( esClient != null ) {
                esClient.close();
            }
        }
    }

    private class CreateIndexThread {

        @ExecuteOrder(step = 1)
        private void createIndex() {
            try ( Sequoiadb db = new Sequoiadb( SdbTestBase.coordUrl, "", "" ) ) {
                DBCollection cl = db.getCollectionSpace( csName ).getCollection( clName );
                BSONObject indexObj = new BasicBSONObject();
                indexObj.put( "a", "text" );
                indexObj.put( "b", "text" );
                indexObj.put( "c", "text" );
                indexObj.put( "d", "text" );
                indexObj.put( "e", "text" );
                cl.createIndex( indexName, indexObj, false, false );
            } catch ( BaseException e ) {
                Assert.assertEquals( e.getErrorCode(), -321, e.getMessage() );
            }
        }
    }

    private class TruncateThread {//TODO:truncate可能报-190或-147，请确认

        @ExecuteOrder(step = 1)
        private void truncate() throws InterruptedException {
            try ( Sequoiadb db = new Sequoiadb( SdbTestBase.coordUrl, "", "" ) ) {
                Thread.sleep( 1000 + new Random().nextInt( 100 ) );
                DBCollection cl = db.getCollectionSpace( csName ).getCollection( clName );
                cl.truncate();
            }
        }
    }

}
