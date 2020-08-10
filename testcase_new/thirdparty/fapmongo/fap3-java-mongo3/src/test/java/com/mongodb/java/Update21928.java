package com.mongodb.java;

import java.net.UnknownHostException;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.util.concurrent.CopyOnWriteArrayList;

import org.bson.BsonInt32;
import org.bson.Document;
import org.bson.conversions.Bson;
import org.testng.Assert;
import org.testng.ITestContext;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import com.mongodb.BasicDBObject;
import com.mongodb.client.MongoCollection;
import com.mongodb.client.MongoDatabase;
import com.mongodb.client.model.Filters;
import com.mongodb.client.model.IndexOptions;
import com.mongodb.client.model.Indexes;
import com.mongodb.client.model.UpdateOptions;
import com.mongodb.client.model.Updates;
import com.mongodb.client.result.UpdateResult;
import com.mongodb.utils.MongodbTestBase;

import static com.mongodb.client.model.Filters.eq;
import static com.mongodb.client.model.Filters.exists;
import static com.mongodb.client.model.Filters.gt;
import static com.mongodb.client.model.Filters.gte;
import static com.mongodb.client.model.Filters.in;
import static com.mongodb.client.model.Filters.lt;
import static com.mongodb.client.model.Updates.inc;
import static com.mongodb.client.model.Updates.set;
import static com.mongodb.client.model.Updates.setOnInsert;

/**
 * @Description seqDB-21928:update操作
 * @author fanyu
 * @Date 2020/3/23
 * @version 1.00
 */
public class Update21928 extends MongodbTestBase {
    private MongoDatabase db;
    private String clName = "cl21928v3";
    private MongoCollection< Document > cl;
    // 不能小于30
    private int num = 30;
    private List< Document > list;

    @BeforeClass
    public void setUp() throws UnknownHostException {
        db = MongodbTestBase.getDataBase( client );
        list = new CopyOnWriteArrayList<>();
        for ( int i = 0; i < num; i++ ) {
            list.add( new Document( "a", i ).append( "b", "" + i )
                    .append( "c", Arrays.asList( i + 1, i + 2, i + 3 ) )
                    .append( "d", "test-" + i + "-" + i % 3 )
                    .append( "e", i % 3 ).append( "f", i ).append( "g",
                            Arrays.asList(
                                    new BasicDBObject( "a", i % 3 ).append( "b",
                                            i + 1 ),
                                    new BasicDBObject( "a", i % 4 ).append( "b",
                                            i + num ) ) ) );
        }
        cl = db.getCollection( clName );
        cl.insertMany( list );
        cl.createIndex( Indexes.ascending( "a" ),
                new IndexOptions().unique( false ).name( "a" ) );
    }

    @Test
    public void test1() {
        Bson query;
        Bson update;
        Bson check;
        // $inc
        query = Filters.and( lt( "a", num / 3 ) );
        update = Updates.combine( inc( "a", num + 50 ), inc( "f", num + 50 ) );
        check = Filters.and( gte( "a", num + 50 ),
                lt( "a", num / 3 + num + 50 ), gte( "f", num + 50 ) );
        updatedAndCheckResult( query, update, check,
                UpdateResult.acknowledged( num / 3, num / 3L, null ), 0 );

        // 不存在的字段
        query = Filters.and( gte( "a", num + 50 ),
                lt( "a", num / 3 + num + 50 ), gte( "f", num + 50 ) );
        update = Updates.inc( "h", -( num + 50 ) );
        check = Filters.eq( "h", -( num + 50 ) );
        updatedAndCheckResult( query, update, check,
                UpdateResult.acknowledged( num / 3, num / 3L, null ), num / 3 );

        // $set
        // 单个字段
        query = Filters.and( gte( "a", num / 3 ), lt( "a", 2 * num / 3 ) );
        update = Updates.set( "a", -1 );
        check = Filters.eq( "a", -1 );
        updatedAndCheckResult( query, update, check,
                UpdateResult.acknowledged( num / 3, num / 3L, null ), 0 );

        // 多个字段
        query = Filters.eq( "a", -1 );
        update = Updates.combine( set( "a", 0 ), set( "f", 0 ) );
        check = Filters.and( eq( "a", 0 ), eq( "f", 0 ) );
        updatedAndCheckResult( query, update, check,
                UpdateResult.acknowledged( num / 3, num / 3L, null ), 0 );

        // 数组
        query = Filters.and( eq( "a", 0 ), eq( "f", 0 ) );
        update = Updates.set( "c.0", 3 * num );
        check = Filters.and( eq( "c.0", 3 * num ), eq( "a", 0 ), eq( "f", 0 ) );
        updatedAndCheckResult( query, update, check,
                UpdateResult.acknowledged( num / 3, num / 3L, null ), num / 3 );

        // 不存在的字段
        query = Filters.and( eq( "a", 0 ), eq( "f", 0 ) );
        update = Updates.set( "l", 1 );
        check = Filters.and( eq( "a", 0 ), eq( "f", 0 ), eq( "l", 1 ) );
        updatedAndCheckResult( query, update, check,
                UpdateResult.acknowledged( num / 3, num / 3L, null ), num / 3 );

        // $unset
        // 单个字段
        query = Filters.and( eq( "a", 0 ), eq( "f", 0 ) );
        update = Updates.unset( "l" );
        check = Filters.and( eq( "a", 0 ), eq( "f", 0 ), exists( "l", false ) );
        updatedAndCheckResult( query, update, check,
                UpdateResult.acknowledged( num / 3, num / 3L, null ), num / 3 );

        // 数组
        query = Filters.and( eq( "a", 0 ), eq( "f", 0 ) );
        update = Updates.unset( "c.0" );
        check = Filters.and( eq( "a", 0 ), eq( "f", 0 ), eq( "c.0", null ) );
        updatedAndCheckResult( query, update, check,
                UpdateResult.acknowledged( num / 3, num / 3L, null ), num / 3 );

        // 不存在的字段
        query = Filters.and( eq( "a", 0 ), eq( "f", 0 ) );
        update = Updates.unset( "k" );
        check = Filters.and( eq( "a", 0 ), eq( "f", 0 ), exists( "k", true ) );
        updatedAndCheckResult( query, update, check,
                UpdateResult.acknowledged( num / 3, 0L, null ), num / 3 );

        // $pop
        // $pop 操作是删除指定数组对象
        query = Filters.and( eq( "a", 0 ), eq( "f", 0 ) );
        update = Updates.popFirst( "c" );
        check = Filters.and( eq( "a", 0 ), eq( "f", 0 ) );
        updatedAndCheckResult( query, update, check,
                UpdateResult.acknowledged( num / 3, num / 3L, null ), num / 3 );

        // 指定的字段不存在
        query = Filters.and( eq( "a", 0 ), eq( "f", 0 ) );
        update = Updates.popFirst( "m" );
        check = Filters.and( eq( "a", 0 ), eq( "f", 0 ), exists( "m", true ) );
        updatedAndCheckResult( query, update, check,
                UpdateResult.acknowledged( num / 3, 0L, null ), num / 3 );

        // $pull
        // 从数组中删除与 <值> 相同的元素
        query = Filters.and( eq( "a", 0 ), eq( "f", 0 ) );
        update = Updates.pull( "c", 13 );
        check = Filters.and( eq( "a", 0 ), eq( "f", 0 ), eq( "c.0", 13 ) );
        UpdateResult actResult = cl.updateMany( query, update );
        Assert.assertEquals( actResult.getMatchedCount(), num / 3 );
        Assert.assertEquals( actResult.getModifiedCount(), 2L );
        Assert.assertEquals( cl.count( check ), 0 );

        // 指定的值不存在
        query = Filters.and( eq( "a", 0 ), eq( "f", 0 ) );
        update = Updates.pull( "c", num + 1 );
        check = Filters.and( eq( "a", 0 ), eq( "f", 0 ), eq( "c.0", num + 1 ) );
        updatedAndCheckResult( query, update, check,
                UpdateResult.acknowledged( num / 3, 0L, null ), num / 3 );

        // $push
        // 向数组中添加元素
        query = Filters.and( eq( "a", 0 ), eq( "f", 0 ) );
        update = Updates.push( "c", num + 1 );
        check = Filters.and( eq( "a", 0 ), eq( "f", 0 ), in( "c", num + 1 ) );
        updatedAndCheckResult( query, update, check,
                UpdateResult.acknowledged( num / 3, num / 3L, null ), num / 3 );

        // 指定的字段名不存在
        query = Filters.and( eq( "a", 0 ), eq( "f", 0 ) );
        update = Updates.push( "i", num + 1 );
        check = Filters.and( eq( "a", 0 ), eq( "f", 0 ), in( "i", num + 1 ) );
        updatedAndCheckResult( query, update, check,
                UpdateResult.acknowledged( num / 3, num / 3L, null ), num / 3 );

        // $addtoset
        // mongodb的addToSet和sequoiadb的addtoset的大小写字母不一样，sequoiadb目前不支持$addtoset操作符号
    }

    @Test
    public void test2() {
        Bson query;
        Bson update;
        Bson check;
        // 更新单条记录
        query = Filters.and( eq( "a", 2 * num / 3 + 1 ) );
        update = Updates.set( "a", -( num / 3 + 1 ) );
        check = Filters.and( eq( "a", -( num / 3 + 1 ) ) );
        updatedAndCheckResult( query, update, check,
                UpdateResult.acknowledged( 1, 1L, null ), 0 );

        // 匹配不到记录，更新单条记录
        query = Filters.and( eq( "a", -10 * num ) );
        update = Updates.set( "a", -10 * num );
        check = Filters.and( eq( "a", -10 * num ) );
        updatedAndCheckResult( query, update, check,
                UpdateResult.acknowledged( 0, 0L, null ), 0 );

        // 匹配不到记录，更新多条记录
        query = Filters.and( eq( "a", -10 * num ) );
        update = Updates.set( "a", -10 * num );
        check = Filters.and( eq( "a", -10 * num ) );
        updatedAndCheckResult( query, update, check,
                UpdateResult.acknowledged( 0, 0L, null ), 0 );

        // 集合不存在记录，更新记录
        if ( db.listCollectionNames().into( new ArrayList< String >() )
                .contains( clName + "-test2" ) ) {
            db.getCollection( clName + "-test2" ).drop();
        }
        db.createCollection( clName + "-test2" );
        MongoCollection< Document > cl1 = db.getCollection( clName + "-test2" );
        Bson query5 = Filters.and( gt( "a", 0 ) );
        Bson update5 = Updates.set( "a", -10 * num );
        UpdateResult result5 = cl1.updateMany( query5, update5 );
        Assert.assertTrue( result5.isModifiedCountAvailable() );
        Assert.assertEquals( result5.getMatchedCount(), 0 );
        Assert.assertEquals( result5.getModifiedCount(), 0 );
    }

    @Test
    public void test3() {
        Bson query;
        Bson update;
        Bson check;
        UpdateOptions updateOptions = new UpdateOptions().upsert( true );
        UpdateResult updateResult;
        // 设置upsert更新记录
        // 匹配不到记录，upsert为true,更新普通字段， 不带setOnInsert
        query = Filters.and( eq( "a", -100 * num ) );
        update = Updates.set( "a", -100 * num );
        check = Filters.and( eq( "a", -100 * num ) );
        updateResult = upsertAndCheckResult( query, update, check,
                updateOptions, UpdateResult.acknowledged( 0, 0L, null ), 1 );
        Assert.assertNotNull( updateResult.getUpsertedId() );

        // 匹配不到记录，upsert为true,更新_id字段 ,不带setOnInsert
        query = Filters.and( eq( "b", -200 * num ) );
        update = Updates.combine( Updates.set( "_id", -200 * num ) );
        check = Filters.and( eq( "_id", -200 * num ) );
        updateResult = upsertAndCheckResult( query, update, check,
                updateOptions, UpdateResult.acknowledged( 0, 0L, null ), 1 );
        Assert.assertNotNull( updateResult.getUpsertedId() );

        // 匹配不到记录，upsert为true,更新混合字段 不带setOnInsert
        query = Filters.and( eq( "b", -1500 * num ) );
        update = Updates.combine( Updates.set( "a", -1500 * num ),
                Updates.set( "_id", -1500 * num ) );
        check = Filters.and( eq( "a", -1500 * num ),
                eq( "_id", new BsonInt32( -1500 * num ) ),
                eq( "b", -1500 * num ) );
        updateResult = upsertAndCheckResult( query, update, check,
                updateOptions, UpdateResult.acknowledged( 0, 0L, null ), 1 );
        Assert.assertNotNull( updateResult.getUpsertedId() );

        // 带setOnInsert更新符
        // 匹配不到记录，upsert为true,更新普通字段、查询条件字段相同,带setOnInsert
        query = Filters.and( eq( "a", -300 * num ) );
        update = Updates.setOnInsert( "a", -300 * num );
        check = Filters.and( eq( "a", -300 * num ) );
        updateResult = upsertAndCheckResult( query, update, check,
                updateOptions, UpdateResult.acknowledged( 0, 0L, null ), 1 );
        Assert.assertNotNull( updateResult.getUpsertedId() );

        // 匹配不到记录，upsert为true,更新_id字段、查询条件字段和更新字段不同,带setOnInsert
        Bson query6 = Filters.and( eq( "a", -400 * num ) );
        Bson update6 = Updates
                .combine( Updates.setOnInsert( "_id", -400 * num ) );
        UpdateResult result6 = cl.updateMany( query6, update6,
                new UpdateOptions().upsert( true ) );
        Bson check6 = Filters.and( eq( "a", -400 * num ),
                eq( "_id", result6.getUpsertedId() ) );
        Assert.assertEquals( result6.getUpsertedId().asInt32().getValue(),
                -400 * num );
        Assert.assertTrue( result6.isModifiedCountAvailable() );
        Assert.assertEquals( result6.getMatchedCount(), 0 );
        Assert.assertEquals( result6.getModifiedCount(), 0 );
        Assert.assertEquals( cl.count( check6 ), 1 );

        // 匹配不到记录，upsert为true，混合字段、查询条件字段不同,带setOnInsert
        query = Filters.and( eq( "a", -1100 * num ) );
        update = Updates.combine( Updates.set( "_id", -1100 * num ),
                Updates.set( "b", -1100 * num ) );
        check = Filters.and( eq( "a", -1100 * num ), eq( "_id", -1100 * num ),
                eq( "b", -1100 * num ) );
        updateResult = upsertAndCheckResult( query, update, check,
                updateOptions, UpdateResult.acknowledged( 0, 0L, null ), 1 );
        Assert.assertEquals( updateResult.getUpsertedId().asInt32().getValue(),
                -1100 * num );

        // 匹配不到记录，upsert为true，普通字段、查询条件字段不同,带setOnInsert + $set
        query = Filters.and( eq( "a", -500 * num ), gt( "b", 0 ) );
        update = Updates.combine( setOnInsert( "_id", -500 * num ),
                Updates.set( "aid", 1 ) );
        check = Filters.and( eq( "a", -500 * num ), eq( "_id", -500 * num ),
                eq( "aid", 1 ) );
        updateResult = upsertAndCheckResult( query, update, check,
                updateOptions, UpdateResult.acknowledged( 0, 0L, null ), 1 );
        Assert.assertEquals( updateResult.getUpsertedId().asInt32().getValue(),
                -500 * num );

        // 匹配不到记录，upsert为true,混合字段、查询条件字段不同,带setOnInsert + $inc
        query = Filters.and( eq( "a", -600 * num ), eq( "aa", 1 ) );
        update = Updates.combine( setOnInsert( "_id", -600 * num ),
                Updates.inc( "aa", 1 ) );
        check = Filters.and( eq( "a", -600 * num ), eq( "_id", -600 * num ),
                eq( "aa", 2 ) );
        updateResult = upsertAndCheckResult( query, update, check,
                updateOptions, UpdateResult.acknowledged( 0, 0L, null ), 1 );
        Assert.assertEquals( updateResult.getUpsertedId().asInt32().getValue(),
                -600 * num );

        // 匹配不到记录，upsert为true,混合字段、查询条件字段不同,带setOnInsert + $unset
        query = Filters.and( eq( "a", -700 * num ), eq( "aa", 0 ) );
        update = Updates.combine( setOnInsert( "_id", -700 * num ),
                Updates.unset( "aa" ) );
        check = Filters.and( eq( "a", -700 * num ), eq( "_id", -700 * num ),
                exists( "aa", false ) );
        updateResult = upsertAndCheckResult( query, update, check,
                updateOptions, UpdateResult.acknowledged( 0, 0L, null ), 1 );
        Assert.assertEquals( updateResult.getUpsertedId().asInt32().getValue(),
                -700 * num );

        // 匹配到记录，upsert为true,更新条件带setOnInsert和带其他更新符，更新条件中字段值与更新之前不相等,更新记录
        query = Filters.and( eq( "a", -700 * num ) );
        update = Updates.combine( Updates.set( "a", -800 * num ),
                setOnInsert( "ab", 0 ) );
        check = Filters.and( eq( "a", -800 * num ), exists( "ab", false ) );
        updateResult = upsertAndCheckResult( query, update, check,
                updateOptions, UpdateResult.acknowledged( 1, 1L, null ), 1 );
        Assert.assertNull( updateResult.getUpsertedId() );

        // 匹配到记录，upsert为true,更新条件带setOnInsert和带其他更新符，更新条件中字段值与更新之前相等,更新记录
        query = Filters.and( eq( "a", -800 * num ) );
        update = Updates.combine( Updates.set( "a", -800 * num ),
                setOnInsert( "ab", 0 ) );
        check = Filters.and( eq( "a", -800 * num ), exists( "ab", false ) );
        updateResult = upsertAndCheckResult( query, update, check,
                updateOptions, UpdateResult.acknowledged( 1, 0L, null ), 1 );
        Assert.assertNull( updateResult.getUpsertedId() );

        // 存在记录，upsert为false更新记录
        query = Filters.and( eq( "a", 2 * num / 3 ) );
        update = Updates.set( "a", -10 * num );
        check = Filters.and( eq( "a", -10 * num ) );
        updateResult = upsertAndCheckResult( query, update, check,
                new UpdateOptions().upsert( false ),
                UpdateResult.acknowledged( 1, 1L, null ), 1 );
        Assert.assertNull( updateResult.getUpsertedId() );
    }

    private void updatedAndCheckResult( Bson query, Bson update, Bson check,
            UpdateResult expResult, int expQueryCount ) {
        UpdateResult actResult = cl.updateMany( query, update );
        Assert.assertEquals( actResult.getMatchedCount(),
                expResult.getMatchedCount() );
        Assert.assertEquals( actResult.getModifiedCount(),
                expResult.getModifiedCount() );
        Assert.assertEquals( actResult.getUpsertedId(),
                expResult.getUpsertedId() );
        Assert.assertEquals( actResult.isModifiedCountAvailable(),
                expResult.isModifiedCountAvailable() );
        Assert.assertEquals( cl.count( query ), expQueryCount );
        Assert.assertEquals( cl.count( check ), expResult.getModifiedCount() );
    }

    private UpdateResult upsertAndCheckResult( Bson query, Bson update,
            Bson check, UpdateOptions updateOptions, UpdateResult expResult,
            int expCheckCount ) {
        UpdateResult actResult = cl.updateMany( query, update, updateOptions );
        Assert.assertEquals( actResult.getMatchedCount(),
                expResult.getMatchedCount() );
        Assert.assertEquals( actResult.getModifiedCount(),
                expResult.getModifiedCount() );
        Assert.assertEquals( actResult.isModifiedCountAvailable(),
                expResult.isModifiedCountAvailable() );
        Assert.assertEquals( cl.count( check ), expCheckCount );
        return actResult;
    }

    @AfterClass
    public void tearDown( ITestContext context ) {
        dropCLByTestResult( context, this.toString(), db, clName );
    }
}
