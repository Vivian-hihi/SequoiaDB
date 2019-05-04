package com.sequoiadb.transaction.rcauto;

import java.util.ArrayList;
import java.util.List;

import org.bson.BSONObject;
import org.bson.util.JSON;
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
import com.sequoiadb.transaction.TransUtils;

/**
 * @testcase seqDB-18235 :: 版本: 1 :: 失败不自动回滚只会显示开启事务生效
 * @date 2019-4-16
 * @author luweikang
 *
 */
@Test(groups = "rcauto")
public class Transaction18235 extends SdbTestBase {
    private Sequoiadb sdb = null;
    private Sequoiadb sdb1;
    private String clName = "cl18235";
    private DBCollection cl = null;
    private List<BSONObject> expList = new ArrayList<>();

    @BeforeClass
    public void setUp() {
        sdb = new Sequoiadb( SdbTestBase.coordUrl, "", "" );
        if ( CommLib.isStandAlone( sdb ) ) {
            throw new SkipException( "STANDALONE MODE" );
        }
        if ( CommLib.OneGroupMode( sdb ) ) {
            throw new SkipException( "less than two groups" );
        }

        cl = sdb.getCollectionSpace( csName ).createCollection( clName,
                (BSONObject) JSON.parse( "{ShardingKey:{a:1}, ShardingType:'hash', AutoSplit: true}" ) );
    }

    @Test
    public void test() {
        sdb1 = new Sequoiadb( SdbTestBase.coordUrl, "", "" );
        DBCollection cl1 = sdb1.getCollectionSpace( csName ).getCollection( clName );

        // 开启事务1，插入记录
        sdb1.beginTransaction();
        TransUtils.insertDatas( cl1, 0, 1000, 1 );
        cl1.update( "{a: 1}", "{$set: {a: 1000}}", null );
        cl1.delete( "{b:{$gte: 0, $lt: 500}}", null );
        List<BSONObject> datas1 = TransUtils.getUpdateDatas( 500, 1000, 1 );

        TransUtils.insertDatas( cl1, 1000, 2000, 2 );
        cl1.update( "{a: 2}", "{$set: {a: 2000}}", null );
        cl1.delete( "{b:{$gte: 1000, $lt: 1500}}", null );
        List<BSONObject> datas2 = TransUtils.getUpdateDatas( 1500, 2000, 2 );

        TransUtils.insertDatas( cl1, 2000, 3000, 3 );
        cl1.update( "{a: 3}", "{$set: {a: 3000}}", null );
        cl1.delete( "{b:{$gte: 2000, $lt: 2500}}", null );
        List<BSONObject> datas3 = TransUtils.getUpdateDatas( 2500, 3000, 3 );

        try {
            cl1.createIndex( "a", "{a: 1}", true, false );
            Assert.fail( "create unique index should be failed" );
        } catch ( BaseException e ) {
            Assert.assertEquals( e.getErrorCode(), -38, e.getMessage() );
        }
        sdb1.commit();

        // 索引扫描记录
        expList.clear();
        expList.addAll( datas1 );
        expList.addAll( datas2 );
        expList.addAll( datas3 );
        DBCursor cursor = cl.query( null, null, null, "{'':'a'}" );
        List<BSONObject> actList = TransUtils.getReadActList( cursor );
        Assert.assertEquals( actList, expList );
        actList.clear();

        // 表扫描记录
        cursor = cl.query( null, null, null, "{'':null}" );
        actList = TransUtils.getReadActList( cursor );
        Assert.assertEquals( actList, expList );
        actList.clear();

    }

    @AfterClass
    public void tearDown() {
        sdb1.commit();
        if ( !sdb1.isClosed() ) {
            sdb1.close();
        }
        CollectionSpace cs = sdb.getCollectionSpace( csName );
        if ( cs.isCollectionExist( clName ) ) {
            cs.dropCollection( clName );
        }
        if ( !sdb.isClosed() ) {
            sdb.close();
        }
    }

}
