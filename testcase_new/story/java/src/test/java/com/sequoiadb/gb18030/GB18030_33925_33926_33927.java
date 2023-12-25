package com.sequoiadb.gb18030;

import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Date;
import java.util.List;

import com.sequoiadb.base.result.DeleteResult;
import org.bson.BSONObject;
import org.bson.BasicBSONObject;
import org.bson.types.*;
import org.testng.Assert;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import com.sequoiadb.base.*;
import com.sequoiadb.testcommon.SdbTestBase;

/**
 * @Descreption seqDB-33925:设置字符集GB18030创建连接，执行插入操作
 *              seqDB-33926:设置字符集GB18030创建连接，执行查询操作
 *              seqDB-33927:设置字符集GB18030创建连接，执行更新、删除操作
 * @Author chenzejia
 * @CreateDate 2023/12/21
 * @UpdateUser
 * @UpdateDate
 * @UpdateRemark
 * @Version
 */
public class GB18030_33925_33926_33927 extends SdbTestBase {
    private Sequoiadb sdb;
    private CollectionSpace cs;
    private DBCollection cl;
    private String csName = "集合空间33925";
    private String clName = "集合33925";
    private List< BSONObject > data = new ArrayList<>();

    private void addDMLData( int _id, String str, BSONObject obj,
            BasicBSONList arr ) {
        BSONObject bson = new BasicBSONObject( "_id", _id );
        bson.put( "字符串", str );
        bson.put( "对象", obj );
        bson.put( "数组", arr );
        data.add( bson );
    }

    public void prepareData() throws ParseException {
        // date timestamp
        Date d = new SimpleDateFormat( "yyyy-MM-dd HH:mm:ss" )
                .parse( "1970-01-01 12:59:59" );
        BSONDate date = new BSONDate( d.getTime() );
        BSONTimestamp timestamp = new BSONTimestamp( 1404189030, 0 );
        // symbol
        Symbol symbol = new Symbol( "你好" );
        // binary
        Binary binary = new Binary( "hello".getBytes() );
        // objectid
        ObjectId objectId = new ObjectId( "53bb5667c5d061d6f579d0bb" );
        // minkey maxkey
        MinKey minKey = new MinKey();
        MaxKey maxKey = new MaxKey();
        // decimal
        BSONDecimal decimal = new BSONDecimal(
                "12345.067891234567890123456789" );
        // code codeWScope
        Code code = new Code( "function(){return \"a㔔b㔕d㔖\"}" );
        CodeWScope codeWScope = new CodeWScope( "function(){return \"1糇糌@\"}",
                new BasicBSONObject( "a", 10 ) );
        // regex
        BasicBSONObject regex = new BasicBSONObject( "$regex", "^潮流服饰$" );
        BSONObject obj1 = new BasicBSONObject();
        obj1.put( "_id", 1 );
        obj1.put( "minKey", minKey );
        obj1.put( "maxKey", maxKey );
        obj1.put( "oId", objectId );
        obj1.put( "null", null );
        obj1.put( "bool", true );
        BSONObject obj2 = new BasicBSONObject();
        obj2.put( "_id", 2 );
        obj2.put( "date", date );
        obj2.put( "timestamp", timestamp );
        obj2.put( "binary", binary );
        obj2.put( "symbol", symbol );
        obj2.put( "regex", regex );
        BSONObject obj3 = new BasicBSONObject();
        obj3.put( "_id", 3 );
        obj3.put( "int", 123 );
        obj3.put( "float", 3.14 );
        obj3.put( "decimal", decimal );
        obj3.put( "code", code );
        obj3.put( "codeWS", codeWScope );
        data.add( obj1 );
        data.add( obj2 );
        data.add( obj3 );

        BasicBSONList arr1 = new BasicBSONList();
        arr1.put( "0", "侤䵋䵌䵍䵎侫侭" );
        arr1.put( "1", date );
        arr1.put( "2", code );
        BasicBSONObject object1 = new BasicBSONObject( "a", "䵪䵫䵬" );
        addDMLData( 4, "巨杉数据库", object1, arr1 );

        BasicBSONList arr2 = new BasicBSONList();
        arr2.put( "0", "上䳏䳐䳑下" );
        arr2.put( "1", "左右" );
        arr2.put( "2", "前䤥䤦后" );
        BasicBSONObject object2 = new BasicBSONObject( "a", "123@你好" );
        object2.put( "b", "栓Q" );
        addDMLData( 5, "珸珹珺珻珼", object2, arr2 );
        // 嵌套数据
        BasicBSONList arr3 = new BasicBSONList();
        arr3.put( "0", arr1 );
        arr3.put( "1", arr2 );
        arr3.put( "2", "㓕1㓖2㓗a" );
        BasicBSONObject object3 = new BasicBSONObject( "a", "㕏㕐㕑" );
        object3.put( "b", object1 );
        object3.put( "c", object2 );
        addDMLData( 6, "テデēéě", object3, arr3 );

        BasicBSONList arr4 = new BasicBSONList();
        arr4.put( "0", arr3 );
        arr4.put( "1", object1 );
        arr4.put( "2", object2 );
        BasicBSONObject object4 = new BasicBSONObject( "a", "" );
        object4.put( "b", object3 );
        object4.put( "c", arr3 );
        addDMLData( 7, " 㐩㐪㐫㐬", object4, arr4 );

        BasicBSONList arr5 = new BasicBSONList();
        arr5.put( "0", 12 );
        arr5.put( "1", decimal );
        arr5.put( "2", "" );
        BasicBSONObject object5 = new BasicBSONObject( "a", "營燡燢燣" );
        object5.put( "b", null );
        object5.put( "c", date );
        addDMLData( 8, "㎎㎏㎜ ", object5, arr5 );

        BasicBSONList arr6 = new BasicBSONList();
        BasicBSONObject object6 = new BasicBSONObject( "a",
                new BasicBSONObject() );
        addDMLData( 9, "", object6, arr6 );

        BasicBSONList arr7 = new BasicBSONList();
        arr7.put( "0", "冬至" );
        arr7.put( "1", regex );
        arr7.put( "2", code );
        BasicBSONObject object7 = new BasicBSONObject( "a", "a1;<=>" );
        object7.put( "b", objectId );
        addDMLData( 10, "a1炄炆;<=>", object7, arr7 );
    }

    @BeforeClass
    public void setUp() {
        ClientCharset charset = new ClientCharset();
        charset.setCharsets( ClientCharsetEnum.GB18030 );
        sdb = GB18030Utils.createCharsetClient( coordUrl, null );
        if ( sdb.isCollectionSpaceExist( csName ) ) {
            sdb.dropCollectionSpace( csName );
        }
        cs = sdb.createCollectionSpace( csName );
        cl = cs.createCollection( clName );
    }

    @Test
    public void test() throws ParseException {
        testInsert();
        testQuery();
        testUpdate();
        testRemove();
    }

    public void testInsert() throws ParseException {
        prepareData();
        cl.bulkInsert( data );
        List< BSONObject > expected = data;
        expected.get( 1 ).put( "symbol", "你好" );
        List< BSONObject > actual = new ArrayList<>();
        DBCursor query = cl.query();
        while ( query.hasNext() ) {
            BSONObject obj = query.getNext();
            actual.add( obj );
        }
        query.close();
        Assert.assertEquals( actual, expected );
    }

    public void testQuery() {
        List< BSONObject > expected;
        BasicBSONObject matcher;

        // 比较查询
        matcher = new BasicBSONObject( "字符串",
                new BasicBSONObject( "$gt", "巨杉" ) );
        expected = data.subList( 3, 5 );
        checkQuery( matcher, null, null, null, expected );

        matcher = new BasicBSONObject( "对象.a",
                new BasicBSONObject( "$gt", "你好" ) );
        expected = new ArrayList<>();
        expected.add( data.get( 7 ) );
        checkQuery( matcher, null, null, null, expected );

        // 模糊查询
        matcher = new BasicBSONObject( "字符串",
                new BasicBSONObject( "$regex", "㐩" ) );
        expected = new ArrayList<>();
        expected.add( data.get( 6 ) );
        checkQuery( matcher, null, null, null, expected );

        matcher = new BasicBSONObject( "数组.0",
                new BasicBSONObject( "$regex", "䳏䳐䳑" ) );
        expected = new ArrayList<>();
        expected.add( data.get( 4 ) );
        checkQuery( matcher, null, null, null, expected );

        // 函数查询
        BasicBSONObject condition = new BasicBSONObject( "$trim", 1 );
        condition.put( "$et", "㐩㐪㐫㐬" );
        matcher = new BasicBSONObject( "字符串", condition );
        expected = new ArrayList<>();
        expected.add( data.get( 6 ) );
        checkQuery( matcher, null, null, null, expected );

        // 排序查询
        BasicBSONObject orderBy = new BasicBSONObject( "字符串", -1 );
        expected = new ArrayList<>();
        expected.add( data.get( 4 ) );
        expected.add( data.get( 3 ) );
        expected.add( data.get( 7 ) );
        expected.add( data.get( 5 ) );
        expected.add( data.get( 9 ) );
        expected.add( data.get( 6 ) );
        matcher = new BasicBSONObject( "字符串",
                new BasicBSONObject( "$gt", "" ) );
        checkQuery( matcher, null, orderBy, null, expected );

        // 聚集查询
        List< BSONObject > pipelines = new ArrayList<>();
        BasicBSONObject match = new BasicBSONObject( "字符串",
                new BasicBSONObject( "$gt", "" ) );
        BasicBSONObject group = new BasicBSONObject( "_id", "$字符串" );
        BasicBSONObject project = new BasicBSONObject( "字符串", 1 );
        project.put( "_id", 1 );
        project.put( "数组", 1 );
        project.put( "对象", 1 );
        pipelines.add( new BasicBSONObject( "$match", match ) );
        pipelines.add( new BasicBSONObject( "$group", group ) );
        pipelines.add( new BasicBSONObject( "$project", project ) );
        List< BSONObject > actual = new ArrayList<>();
        DBCursor aggregate = cl.aggregate( pipelines );
        while ( aggregate.hasNext() ) {
            actual.add( aggregate.getNext() );
        }
        aggregate.close();
        expected = new ArrayList<>();
        expected.add( data.get( 6 ) );
        expected.add( data.get( 9 ) );
        expected.add( data.get( 5 ) );
        expected.add( data.get( 7 ) );
        expected.add( data.get( 3 ) );
        expected.add( data.get( 4 ) );
        Assert.assertEquals( actual, expected );
    }

    public void testUpdate() {
        BasicBSONObject matcher;
        BasicBSONObject modify;
        // $set
        matcher = new BasicBSONObject( "_id", 4 );
        matcher.put( "字符串", "巨杉数据库" );
        modify = new BasicBSONObject( "$set",
                new BasicBSONObject( "字符串", "你好" ) );
        cl.updateRecords( matcher, modify );
        BSONObject actual = cl.queryOne( new BasicBSONObject( "_id", 4 ), null,
                null, null, 0 );
        BasicBSONObject expected = ( BasicBSONObject ) data.get( 3 );
        expected.put( "字符串", "你好" );
        Assert.assertEquals( actual, expected );

        matcher = new BasicBSONObject( "_id", 5 );
        matcher.put( "字符串", "珸珹珺珻珼" );
        modify = new BasicBSONObject( "$push",
                new BasicBSONObject( "数组", "新元素" ) );
        cl.updateRecords( matcher, modify );

        // $push
        actual = cl.queryOne( new BasicBSONObject( "_id", 5 ), null, null, null,
                0 );
        expected = ( BasicBSONObject ) data.get( 4 );
        BasicBSONList arr = ( BasicBSONList ) expected.get( "数组" );
        arr.put( "3", "新元素" );
        Assert.assertEquals( actual, expected );
    }

    public void testRemove() {
        BasicBSONObject matcher = new BasicBSONObject( "对象.a",
                new BasicBSONObject( "$et", "123@你好" ) );
        DeleteResult deleteResult = cl.deleteRecords( matcher );
        Assert.assertEquals( deleteResult.getDeletedNum(), 1 );

        matcher = new BasicBSONObject( "字符串",
                new BasicBSONObject( "$gt", "" ) );
        deleteResult = cl.deleteRecords( matcher );
        Assert.assertEquals( deleteResult.getDeletedNum(), 5 );

        // check delete
        List< BSONObject > actual = new ArrayList<>();
        DBCursor query = cl.query();
        while ( query.hasNext() ) {
            BSONObject obj = query.getNext();
            actual.add( obj );
        }
        query.close();
        List< BSONObject > expected = new ArrayList<>();
        expected.add( data.get( 0 ) );
        expected.add( data.get( 1 ) );
        expected.add( data.get( 2 ) );
        expected.add( data.get( 8 ) );
        Assert.assertEquals( actual, expected );

    }

    @AfterClass
    public void tearDown() {
        sdb.dropCollectionSpace( csName );
        sdb.close();
    }

    private void checkQuery( BSONObject matcher, BSONObject selector,
            BSONObject orderBy, BSONObject hint, List< BSONObject > expected ) {
        DBCursor query = cl.query( matcher, selector, orderBy, hint );
        List< BSONObject > actual = new ArrayList<>();
        while ( query.hasNext() ) {
            BSONObject obj = query.getNext();
            actual.add( obj );
        }
        query.close();
        Assert.assertEquals( actual, expected,
                "check query failed,matcher:" + matcher + " selector:"
                        + selector + " orderBy:" + orderBy + " hint:" + hint );
    }
}
