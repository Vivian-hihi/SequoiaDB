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
 * @Descreption seqDB-33923:设置字符集GB18030创建连接，执行索引操作
 * @Author chenzejia
 * @CreateDate 2023/12/21
 * @UpdateUser
 * @UpdateDate
 * @UpdateRemark
 * @Version
 */
public class GB18030_33923 extends SdbTestBase {
    private Sequoiadb sdb;
    private CollectionSpace cs;
    private DBCollection cl;
    private String csName = "集合空间33923";
    private String clName = "集合33923";
    private String[] indexNames = { "索引33923", "场尝常33923", "㓶㓷㓸33923" };
    List< BSONObject > data = new ArrayList<>();

    @BeforeClass
    public void setUp() {
        ClientCharset charset = new ClientCharset();
        charset.setCharsets( ClientCharsetEnum.GB18030 );
        sdb = GB18030Utils.createCharsetClient( coordUrl, charset );
        if ( sdb.isCollectionSpaceExist( csName ) ) {
            sdb.dropCollectionSpace( csName );
        }
        cs = sdb.createCollectionSpace( csName );
        cl = cs.createCollection( clName );

        for ( int i = 0; i < 10; i++ ) {
            BSONObject bson = new BasicBSONObject( "_id", i );
            bson.put( "a", "测试数据" + i );
            data.add( bson );
        }
        cl.bulkInsert( data );
    }

    @Test
    public void test() {
        for ( int i = 0; i < indexNames.length; i++ ) {
            // create index
            cl.createIndex( indexNames[ i ], new BasicBSONObject( "a", 1 ),
                    true, false );
            // get indexInfo
            BSONObject indexInfo = cl.getIndexInfo( indexNames[ i ] );
            BSONObject indexDef = ( BSONObject ) indexInfo.get( "IndexDef" );
            Assert.assertEquals( indexDef.get( "name" ), indexNames[ i ],
                    "index name is not expected" );
            BSONObject key = ( BSONObject ) indexDef.get( "key" );
            Assert.assertEquals( key, new BasicBSONObject( "a", 1 ),
                    "index key is not expected" );
            // select by index
            DBCursor query = cl.query(
                    new BasicBSONObject( "a",
                            new BasicBSONObject( "$gte", "测试数据5" ) ),
                    null, null, new BasicBSONObject( "", indexNames[ 0 ] ) );
            List< BSONObject > result = new ArrayList<>();
            while ( query.hasNext() ) {
                result.add( query.getNext() );
            }
            List< BSONObject > expected_result = data.subList( 5, 10 );
            Assert.assertEquals( result, expected_result );
            // drop index
            cl.dropIndex( indexNames[ i ] );
            Assert.assertFalse( cl.isIndexExist( indexNames[ i ] ),
                    "the index [" + indexNames[ i ] + "] drop failed" );
        }
    }

    @AfterClass
    public void tearDown() {
        sdb.dropCollectionSpace( csName );
        sdb.close();
    }
}
