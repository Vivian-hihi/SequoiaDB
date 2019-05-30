package com.sequoiadb.fulltextparallel;

import java.util.List;

import org.bson.BSONObject;
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
 * @FileName seqDB-12121:并发删除同一条记录
 * @Author yinzhen
 * @Date 2019-4-28
 */
public class FullText12121 extends SdbTestBase {
    private String clName = "cl12121";
    private Sequoiadb sdb;
    private DBCollection cl;
    private String fullIdxName = "idx12121";
    private Client esClient;
    private String esIndexName;
    private String cappedCLName;

    @BeforeClass
    public void setUp() throws Exception {
        sdb = new Sequoiadb( SdbTestBase.coordUrl, "", "" );
        if ( CommLib.isStandAlone( sdb ) ) {
            throw new SkipException( "STANDALONE MODE" );
        }

        esClient = FullTextESUtils.createTransportClient( SdbTestBase.esHostName,
                Integer.parseInt( SdbTestBase.esServiceName ) );
        cl = sdb.getCollectionSpace( csName ).createCollection( clName );

        // 创建全文索引
        cl.createIndex( fullIdxName, "{'a':'text','b':'text','c':'text', 'd':'text', 'e':'text', 'f':'text'}", false,
                false );
        esIndexName = FullTextDBUtils.getESIndexName( cl, fullIdxName );
        cappedCLName = FullTextDBUtils.getCappedName( cl, fullIdxName );
        FullTextDBUtils.insertData( cl, 20000 );
        cl.insert( "{a:'idx12121', b:'b12121'}" );
        Assert.assertTrue( FullTextUtils.isIndexCreated( esClient, cl, fullIdxName, 20001 ) );
    }

    @Test
    public void test() throws Exception {
        ThreadExecutor thExecutor = new ThreadExecutor( 600000 );
        for ( int i = 0; i < 10; i++ ) {
            thExecutor.addWorker( new DeleteRecord() );
        }
        thExecutor.run();

        // 固定集合中新增一条操作类型为删除，值正确的记录
        Assert.assertTrue( FullTextUtils.isIndexCreated( esClient, cl, fullIdxName, 20000 ) );
        DBCollection cappedCL = FullTextDBUtils.getCappedCLs( cl, fullIdxName ).get( 0 );
        List<BSONObject> records = FullTextDBUtils.getRecordsFromCL( cappedCL.query() );
        Assert.assertEquals( records.get( 0 ).get( "Type" ), 2 );

        // es中数据与原集合数据一致
        Sequoiadb db2 = new Sequoiadb( SdbTestBase.coordUrl, "", "" );
        try {
            DBCollection cl2 = db2.getCollectionSpace( csName ).getCollection( clName );
            DBCursor dbCursor = cl.query( "{}", "{}", "{_id:1}", "{}" );
            DBCursor esCursor = cl2.query( "{'':{'$Text':{'query':{'match_all':{}}}}}", "{}", "{_id:1}",
                    "{'':'" + fullIdxName + "'}" );
            Assert.assertTrue( FullTextUtils.isCLRecordsConsistency( dbCursor, esCursor ) );
        } finally {
            if ( db2 != null ) {
                db2.close();
            }
        }

        // 在db端执行插入、全文检索
        Sequoiadb db3 = new Sequoiadb( SdbTestBase.coordUrl, "", "" );
        try {
            FullTextDBUtils.insertData( cl, 1000 );
            Assert.assertEquals( cl.getCount(), 21000 );
            Assert.assertTrue( FullTextUtils.isIndexCreated( esClient, cl, fullIdxName, 21000 ) );

            DBCollection cl3 = db3.getCollectionSpace( csName ).getCollection( clName );
            DBCursor dbCursor = cl.query( "{}", "{}", "{_id:1}", "{}" );
            DBCursor esCursor = cl3.query( "{'':{'$Text':{'query':{'match_all':{}}}}}", "{}", "{_id:1}",
                    "{'':'" + fullIdxName + "'}" );
            Assert.assertTrue( FullTextUtils.isCLRecordsConsistency( dbCursor, esCursor ) );
        } finally {
            if ( db3 != null ) {
                db3.close();
            }
        }
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

    private class DeleteRecord {
        @ExecuteOrder(step = 1, desc = "多线程并发删除同一条包含全文索引字段的记录")
        private void deleteRecord() {
            Sequoiadb db = null;
            try {
                db = new Sequoiadb( coordUrl, "", "" );
                DBCollection cl = db.getCollectionSpace( csName ).getCollection( clName );
                cl.delete( "{a:'idx12121', b:'b12121'}", "{'':'" + fullIdxName + "'}" );
            } finally {
                if ( db != null ) {
                    db.close();
                }
            }
        }
    }
}