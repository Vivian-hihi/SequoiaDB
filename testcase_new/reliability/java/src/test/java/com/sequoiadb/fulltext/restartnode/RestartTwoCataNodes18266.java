package com.sequoiadb.fulltext.restartnode;

import org.bson.BSONObject;
import org.bson.BasicBSONObject;
import org.testng.Assert;
import org.testng.SkipException;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;
import com.sequoiadb.base.ReplicaGroup;
import com.sequoiadb.base.Node;

import com.sequoiadb.base.CollectionSpace;
import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.DBCursor;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.commlib.SdbTestBase;
import com.sequoiadb.commlib.CommLib;
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.fulltext.FullTextDBUtils;
import com.sequoiadb.fulltext.FullTextUtils;

/**
 * @FileName seqDB-18266: 正常停止两个编目节点后再次重启一个节点，并创建全文索引
 * @Author liuxiaoxuan
 * @Date 2019-06-26
 */

public class RestartTwoCataNodes18266 extends SdbTestBase {

    private Sequoiadb sdb;
    private DBCollection cl;
    private String clName = "restartNodes18266";
    private String textIndexName = "fullIndex18266";
    private boolean clearFlag = false;

    @BeforeClass
    public void setUp() {
        sdb = new Sequoiadb( SdbTestBase.coordUrl, "", "" );
        if ( CommLib.isStandAlone( sdb ) ) {
            throw new SkipException( "StandAlone environment!" );
        }
        CollectionSpace cs = sdb.getCollectionSpace( csName );
        cl = cs.createCollection( clName );
    }

    @Test
    public void test() throws Exception {
        // 获取两个节点
        ReplicaGroup cataRG = sdb.getReplicaGroup( "SYSCatalogGroup" );
        Node masterNode = cataRG.getMaster();
        Node slaveNode = cataRG.getSlave();

        // 停止两个节点并只启动一个
        masterNode.stop();
        slaveNode.stop();
        slaveNode.start();

        // 等待选主
        while ( true ) {
            try {
                cataRG.getMaster();
                break;
            } catch ( BaseException e ) {
                if ( -104 != e.getErrorCode() )
                    throw e;
            }
        }

        // 创建全文索引，插入记录
        cl.createIndex( textIndexName, "{\"a\":\"text\"}", false, false );
        FullTextDBUtils.insertData( cl, 20000 );
        Assert.assertTrue(
                FullTextUtils.isIndexCreated( cl, textIndexName, 20000 ) );

        // 执行全文检索
        BSONObject matcher = new BasicBSONObject( "",
                new BasicBSONObject( "$Text",
                        new BasicBSONObject( "query", new BasicBSONObject(
                                "match",
                                new BasicBSONObject( "a", clName ) ) ) ) );
        DBCursor cursor = cl.query( matcher, null, null, null );
        int count = 0;
        while ( cursor.hasNext() ) {
            cursor.getNext();
            count++;
        }
        Assert.assertEquals( count, 20000 );

        clearFlag = true;
    }

    @AfterClass
    public void tearDown() {
        try {
            if ( clearFlag ) {
                sdb.getCollectionSpace( csName ).dropCollection( clName );
            }
        } catch ( BaseException e ) {
            Assert.fail( e.getMessage() );
        } finally {
            if ( sdb != null ) {
                sdb.close();
            }
        }
    }

}
