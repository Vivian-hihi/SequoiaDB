package com.sequoiadb.gb18030;

import com.sequoiadb.base.*;
import com.sequoiadb.base.result.DeleteResult;
import com.sequoiadb.base.result.InsertResult;
import org.bson.BSONObject;
import org.bson.BasicBSONObject;
import org.testng.Assert;

import java.util.ArrayList;
import java.util.List;

/**
 * @Descreption 公共方法类
 * @Author chenzejia
 * @CreateDate 2023/12/21
 * @UpdateUser
 * @UpdateDate
 * @UpdateRemark
 * @Version
 */
public class GB18030Utils {
    public static Sequoiadb createCharsetClient( String coordUrl,
            ClientCharset charset ) {
        ConfigOptions options = new ConfigOptions();
        if ( charset != null ) {
            options.setCharset( charset );
        }
        return new Sequoiadb( coordUrl, "", "", options );
    }

    public static void checkClFunction( DBCollection cl ) {
        List< BSONObject > data = new ArrayList<>();
        for ( int i = 0; i < 10; i++ ) {
            BSONObject bson = new BasicBSONObject( "_id", i );
            bson.put( "a", "测试数据" + i );
            data.add( bson );
        }
        InsertResult insertResult = cl.bulkInsert( data );
        Assert.assertEquals( insertResult.getInsertNum(), 10,
                "insert data failed" );
        DBCursor query = cl.query();
        List< BSONObject > result = new ArrayList<>();
        while ( query.hasNext() ) {
            result.add( query.getNext() );
        }
        query.close();
        Assert.assertEquals( result, data, "the " + cl.getName()
                + " query result is not equal to the insert data" );
    }

    public static void checkConnFunction( Sequoiadb conn, String csName ) {
        String[] chineseData = { "测试数据", "㓥珸珹珺珻珼㓦㓧", "㏷㏸㏹", "㪫㪬㪭", "巨杉数据库",
                "中国人民", "", "56个民族", "1", "赵钱孙李" };
        // 创建集合空间
        CollectionSpace cs = conn.createCollectionSpace( csName );
        // 创建集合
        DBCollection cl = cs.createCollection( "测试数据库" );
        // 创建索引
        cl.createIndex( "测试索引", new BasicBSONObject( "字符串字段", 1 ), false,
                false );
        Assert.assertTrue( cl.isIndexExist( "测试索引" ) );
        // CRUD操作
        List< BSONObject > data = new ArrayList<>();
        for ( int i = 0; i < chineseData.length; i++ ) {
            BSONObject bson = new BasicBSONObject( "_id", i );
            bson.put( "字符串字段", chineseData[ i ] );
            bson.put( "对象字段", new BasicBSONObject( "a", chineseData[ i ] ) );
            data.add( bson );
        }
        cl.bulkInsert( data );
        DBCursor query = cl.query();
        List< BSONObject > result = new ArrayList<>();
        while ( query.hasNext() ) {
            BSONObject next = query.getNext();
            result.add( next );
        }
        query.close();
        Assert.assertEquals( result, data, "the " + cl.getName()
                + " query result is not equal to the insert data" );
        for ( int i = 0; i < chineseData.length; i++ ) {
            // query
            BSONObject matcher = new BasicBSONObject( "字符串字段",
                    new BasicBSONObject( "$regex", chineseData[ i ] ) );
            BSONObject actual = cl.queryOne( matcher, null, null, null, 0 );
            Assert.assertEquals( actual, data.get( i ), "the " + cl.getName()
                    + " query result is not equal to the insert data" );
            // update
            matcher.put( "_id", i );
            BSONObject modify = new BasicBSONObject( "$set",
                    new BasicBSONObject( "字符串字段", "修改数据" + i ) );
            cl.updateRecords( matcher, modify );
            matcher = new BasicBSONObject( "_id", i );
            actual = cl.queryOne( matcher, null, null, null, 0 );
            Assert.assertEquals( ( String ) actual.get( "字符串字段" ), "修改数据" + i );
            // delete
            matcher = new BasicBSONObject( "字符串字段", "修改数据" + i );
            DeleteResult deleteResult = cl.deleteRecords( matcher );
            Assert.assertEquals( deleteResult.getDeletedNum(), 1 );
        }
        // 删除索引
        cl.dropIndex( "测试索引" );
        Assert.assertFalse( cl.isIndexExist( "测试索引" ) );
        // 删除集合
        cs.dropCollection( "测试数据库" );
        Assert.assertFalse( cs.isCollectionExist( "测试数据库" ) );
        // 删除集合空间
        conn.dropCollectionSpace( csName );
        Assert.assertFalse( conn.isCollectionSpaceExist( csName ) );
        conn.close();
    }

}
