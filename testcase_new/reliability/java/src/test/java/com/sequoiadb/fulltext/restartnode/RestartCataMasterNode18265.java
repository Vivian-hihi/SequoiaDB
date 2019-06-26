package com.sequoiadb.fulltext.restartnode;

import org.bson.BSONObject;
import org.bson.BasicBSONObject;
import org.testng.Assert;
import org.testng.SkipException;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;
import com.sequoiadb.commlib.GroupMgr;
import com.sequoiadb.commlib.GroupWrapper;
import com.sequoiadb.commlib.NodeWrapper;
import com.sequoiadb.fault.NodeRestart;
import com.sequoiadb.task.FaultMakeTask;
import com.sequoiadb.task.TaskMgr;

import com.sequoiadb.base.CollectionSpace;
import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.DBCursor;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.commlib.SdbTestBase;
import com.sequoiadb.commlib.CommLib;
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.exception.ReliabilityException;
import com.sequoiadb.fulltext.FullTextDBUtils;
import com.sequoiadb.fulltext.FullTextUtils;

/**
 * @FileName seqDB-18265: 正常停止编目主节点，选出新主后创建全文索引
 * @Author liuxiaoxuan
 * @Date 2019-06-26
 */

public class RestartCataMasterNode18265 extends SdbTestBase {

    private GroupMgr groupMgr = null;
    private Sequoiadb sdb;
    private DBCollection cl;
    private String clName = "restartNodes18265";
    private String textIndexName = "fullIndex18265";
    private boolean clearFlag = false;

    @BeforeClass
    public void setUp() throws ReliabilityException {
        sdb = new Sequoiadb( SdbTestBase.coordUrl, "", "" );
        if ( CommLib.isStandAlone( sdb ) ) {
            throw new SkipException( "StandAlone environment!" );
        }

        groupMgr = GroupMgr.getInstance();
        if ( !groupMgr.checkBusiness() ) {
            throw new SkipException( "checkBusiness failed" );
        }

        CollectionSpace cs = sdb.getCollectionSpace( csName );
        cl = cs.createCollection( clName );
    }

    @Test
    public void test() throws Exception {
        GroupWrapper cataGroup = groupMgr.getGroupByName( "SYSCatalogGroup" );
        NodeWrapper primaryNode = cataGroup.getMaster();

        FaultMakeTask faultTask = NodeRestart.getFaultMakeTask( primaryNode, 1,
                10, 10 );
        TaskMgr mgr = new TaskMgr( faultTask );
        mgr.execute();

        // 等待环境恢复
        Assert.assertEquals( mgr.isAllSuccess(), true, mgr.getErrorMsg() );
        Assert.assertEquals( groupMgr.checkBusinessWithLSN( 600 ), true,
                "check LSN consistency fail" );

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
