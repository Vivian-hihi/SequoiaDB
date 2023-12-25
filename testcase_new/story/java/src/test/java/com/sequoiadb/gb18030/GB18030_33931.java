package com.sequoiadb.gb18030;

import org.bson.BSONObject;
import org.bson.BasicBSONObject;
import org.testng.Assert;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import com.sequoiadb.base.*;
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.exception.SDBError;
import com.sequoiadb.testcommon.SdbTestBase;

import java.util.ArrayList;
import java.util.List;

/**
 * @Descreption seqDB-33931:设置字符集GB18030创建连接，执行内置sql语句
 * @Author chenzejia
 * @CreateDate 2023/12/21
 * @UpdateUser
 * @UpdateDate
 * @UpdateRemark
 * @Version
 */
public class GB18030_33931 extends SdbTestBase {
    private Sequoiadb sdb;
    private CollectionSpace cs;
    private DBCollection cl;
    private String csName = "集合空间33931";
    private String clName = "集合33931";
    private String fullClName = csName + "." + clName;
    private String indexName = "索引33931";
    String[] data = { "爯爾牀", "䶛䶟䶠䶡", "鿋鿌鿑鿒鿓", "𫌀𫖳𫘪𫘬𫞩", "", "䰰䰱䰲䰳",
            "ab123#$%", "如履薄冰" };

    @BeforeClass
    public void setUp() {
        ClientCharset charset = new ClientCharset();
        charset.setCharsets( ClientCharsetEnum.GB18030 );
        sdb = GB18030Utils.createCharsetClient( coordUrl, charset );
        if ( sdb.isCollectionSpaceExist( csName ) ) {
            sdb.dropCollectionSpace( csName );
        }
    }

    @Test
    public void test() {
        // create collection space
        String sql = "create collectionspace " + csName;
        sdb.execUpdate( sql );
        cs = sdb.getCollectionSpace( csName );
        // create collection
        sql = "create collection " + fullClName;
        sdb.execUpdate( sql );
        cl = cs.getCollection( clName );
        // create index
        sql = "create index " + indexName + " on " + fullClName + "(a)";
        sdb.execUpdate( sql );
        // insert data
        for ( int i = 0; i < data.length; i++ ) {
            sql = "insert into " + fullClName + "(a) values(\"" + data[ i ]
                    + "\")";
            sdb.execUpdate( sql );
        }
        // query data
        sql = "select a from " + fullClName + " order by a limit 2 offset 1";
        List< BSONObject > expected = new ArrayList<>();
        expected.add( new BasicBSONObject( "a", data[ 6 ] ) );
        expected.add( new BasicBSONObject( "a", data[ 5 ] ) );
        execQueryAndCheck( sql, expected );
        // group by
        sql = "select a as 字段值 from " + fullClName
                + " where a > \"你好\" group by a";
        expected.clear();
        expected.add( new BasicBSONObject( "字段值", data[ 7 ] ) );
        expected.add( new BasicBSONObject( "字段值", data[ 0 ] ) );
        expected.add( new BasicBSONObject( "字段值", data[ 2 ] ) );
        expected.add( new BasicBSONObject( "字段值", data[ 3 ] ) );
        execQueryAndCheck( sql, expected );
        // where like
        sql = "select a from " + fullClName + " where a like '䶛'";
        expected.clear();
        expected.add( new BasicBSONObject( "a", data[ 1 ] ) );
        execQueryAndCheck( sql, expected );
        // join
        sql = "select t1.a as 值1,t2.a as 值2 from " + fullClName
                + " as t1 inner join " + fullClName + " as t2 on t1.a=t2.a";
        expected.clear();
        for ( String str : data ) {
            expected.add(
                    new BasicBSONObject( "值1", str ).append( "值2", str ) );
        }
        execQueryAndCheck( sql, expected );

        // update
        sql = "update " + fullClName + " set a='1a更新数据' where a>''";
        sdb.execUpdate( sql );
        sql = "select a from " + fullClName + " order by a";
        expected.clear();
        expected.add( new BasicBSONObject( "a", "" ) );
        for ( int i = 0; i < 7; i++ ) {
            expected.add( new BasicBSONObject( "a", "1a更新数据" ) );
        }
        execQueryAndCheck( sql, expected );

        // delete
        sql = "delete from " + fullClName + " where a='1a更新数据'";
        sdb.execUpdate( sql );
        sql = "select a from " + fullClName + " order by a";
        expected.clear();
        expected.add( new BasicBSONObject( "a", data[ 4 ] ) );
        execQueryAndCheck( sql, expected );

        // drop index
        sql = "drop index " + indexName + " on " + fullClName;
        sdb.execUpdate( sql );
        try {
            cl.getIndexInfo( indexName );
            Assert.fail( "drop index failed" );
        } catch ( BaseException e ) {
            Assert.assertEquals( e.getErrorCode(),
                    SDBError.SDB_IXM_NOTEXIST.getErrorCode() );
        }
        // drop collection
        sql = "drop collection " + fullClName;
        sdb.execUpdate( sql );
        Assert.assertFalse( cs.isCollectionExist( clName ) );
        // drop collection space
        sql = "drop collectionspace " + csName;
        sdb.execUpdate( sql );
        Assert.assertFalse( sdb.isCollectionSpaceExist( csName ) );
    }

    @AfterClass
    public void tearDown() {
        sdb.close();
    }

    private void execQueryAndCheck( String sql, List< BSONObject > expected ) {
        List< BSONObject > actual = new ArrayList<>();
        DBCursor cursor = sdb.exec( sql );
        while ( cursor.hasNext() ) {
            BSONObject obj = cursor.getNext();
            actual.add( obj );
        }
        cursor.close();
        Assert.assertEquals( actual, expected, "执行sql语句：" + sql + "，结果不符合预期" );
    }
}
