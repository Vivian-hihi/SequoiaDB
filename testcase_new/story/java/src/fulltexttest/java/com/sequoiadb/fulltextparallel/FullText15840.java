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
import com.sequoiadb.base.DBCursor;
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
 * @FileName FullText15840.java 创建全文索引与全文检索并发
 * @Author luweikang
 * @Date 2019年5月10日
 */
public class FullText15840 extends SdbTestBase {

    private Sequoiadb sdb = null;
    private CollectionSpace cs = null;
    private DBCollection cl = null;
    private String clName = "es_15840";
    private Client esClient = null;
    private String indexName = "fulltextIndex15840";
    private String cappedName = null;
    private String esIndexName = null;
    private int insertNum = 20000;

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
        thread.addWorker( new QueryByTextIndexThread() );
        thread.run();

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
            }
        }
    }

    private class QueryByTextIndexThread {

        @ExecuteOrder(step = 1)
        private void queryData() throws InterruptedException {
            try ( Sequoiadb db = new Sequoiadb( SdbTestBase.coordUrl, "", "" ) ) {
                Thread.sleep( 5500 + new Random().nextInt( 500 ) );//TODO：建议循环查多次，每查询一次随机暂停 x 毫秒，可能会撞到创建索引不同的点
                DBCollection cl = db.getCollectionSpace( csName ).getCollection( clName );
                DBCursor cur = cl.query( "{'': {'$Text': {'query': {'match_all': {}}}}}", null, "{'a': 1}",
                        "{'': '" + indexName + "'}" );
                if ( cur.hasNext() ) {
                    BSONObject record = cur.getNext();
                    System.out.println( record );
                }
                cur.close();
            } catch ( BaseException e ) {
                e.printStackTrace();
                if ( e.getErrorCode() != -6 && e.getErrorCode() != -52 ) {
                    Assert.fail( e.getMessage() );
                }
            }
        }
    }

}
