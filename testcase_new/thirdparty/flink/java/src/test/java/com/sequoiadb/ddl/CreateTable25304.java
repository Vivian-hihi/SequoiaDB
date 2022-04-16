package com.sequoiadb.ddl;

import com.sequoiadb.base.CollectionSpace;
import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.testcommon.FlinkTestBase;
import com.sequoiadb.testcommon.utils.Commlib;
import com.sequoiadb.testcommon.utils.ConversionUtils;
import org.apache.flink.streaming.api.environment.StreamExecutionEnvironment;
import org.apache.flink.table.api.DataTypes;
import org.apache.flink.table.api.Schema;
import org.apache.flink.table.api.TableDescriptor;
import org.apache.flink.table.api.bridge.java.StreamTableEnvironment;
import org.apache.flink.table.types.DataType;
import org.apache.flink.types.Row;
import org.bson.BasicBSONObject;
import org.testng.Assert;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import java.nio.charset.StandardCharsets;
import java.sql.Date;
import java.sql.Timestamp;
import java.text.SimpleDateFormat;

/**
 * @descreption seqDB-25304:创建SDB映射表，string类型非数值字符串转换
 * @author YiPan
 * @date 2022/1/29
 * @updateUser
 * @updateDate
 * @updateRemark
 * @version 1.0
 */
public class CreateTable25304 extends FlinkTestBase {
    private Sequoiadb sdb;
    private StreamTableEnvironment tableEnv;
    private DBCollection cl;
    private final String csName = "cs_25304";
    private final String clName = "cl_25304";
    private final String tableNameBase = "tb_25304_";
    private final String filed_string = "test_string";
    private final String data_string = "abcasACIO123456";
    private final String filed_date = "test_date";
    private final long data_date = System.currentTimeMillis();
    private final String filed_bool = "test_bool";
    private final String bool_data = "True";

    @BeforeClass
    public void setUp() {
        StreamExecutionEnvironment env = StreamExecutionEnvironment
                .getExecutionEnvironment();
        tableEnv = StreamTableEnvironment.create( env );
        sdb = new Sequoiadb( FlinkTestBase.getCoord(), FlinkTestBase.username,
                FlinkTestBase.password );
        if ( sdb.isCollectionSpaceExist( csName ) ) {
            sdb.dropCollectionSpace( csName );
        }
        CollectionSpace cs = sdb.createCollectionSpace( csName );
        cl = cs.createCollection( clName );
        insertData();
    }

    @Test
    public void test() throws Exception {
        // 非数值字符串转换数值类型,不兼容转null
        testNotSupport();

        // 非时间格式转换成DATE类型，预期不兼容；时间格式转换成Date类型，预期兼容
        toDate();

        // 非时间格式转换成TIMESTAMP类型，预期不兼容；时间格式转换成Date类型，预期兼容
        toTIMESTAMP();

        // 非BOOL字符串转换成BOOLEAN类型，预期不兼容，BOOL字符串转换成BOOLEAN类型，预期兼容
        toBOOLEAN();

        // 非数值字符串转换成DECIMAL类型，预期不兼容转null
        toDECIMAL();

        // 非数值字符串转换成字符串类型,预期兼容
        toCHAR();

        // 非数值字符串转换成二进制类型，预期兼容
        toBINARY();
    }

    public void testNotSupport() throws Exception {
        DataType[] dataTypes = { DataTypes.TINYINT(), DataTypes.SMALLINT(),
                DataTypes.INT(), DataTypes.BIGINT(), DataTypes.FLOAT(),
                DataTypes.DOUBLE() };
        for ( int i = 0; i < dataTypes.length; i++ ) {
            String tableName = ConversionUtils.initTableName( tableNameBase,
                    dataTypes[ i ] );
            createTable( tableName, dataTypes[ i ], filed_string );
            Row row = ConversionUtils.queryOne( tableEnv, tableName );
            Assert.assertNull(row.getField(filed_string));
        }
    }

    public void toDate() throws Exception {
        String tableName = ConversionUtils.initTableName( tableNameBase,
                DataTypes.DATE() );
        createTable( tableName, DataTypes.DATE(), filed_string, filed_date );
        Row row = ConversionUtils.queryOne( tableEnv, tableName );
        Assert.assertEquals( row.getField( filed_string ),
                ConversionUtils.getLocalDate( new Date( 0 ) ) );
        Assert.assertEquals( row.getField( filed_date ),
                ConversionUtils.getLocalDate( new Date( data_date ) ) );
    }

    public void toTIMESTAMP() throws Exception {
        String tableName = ConversionUtils.initTableName( tableNameBase,
                DataTypes.TIMESTAMP() );
        createTable( tableName, DataTypes.TIMESTAMP(), filed_string,
                filed_date );
        Row row = ConversionUtils.queryOne( tableEnv, tableName );
        Assert.assertEquals( row.getField( filed_string ),
                ConversionUtils.getLocalDateTime( new Timestamp( 0 ) ) );
        // 插入的时间类型只精确到秒，需要格式化再校验
        SimpleDateFormat sdf = new SimpleDateFormat( "yyyy-MM-dd'T'HH:mm:ss" );
        Assert.assertEquals( row.getField( filed_date ).toString(),
                sdf.format( new Timestamp( data_date ) ) );
    }

    public void toBOOLEAN() throws Exception {
        String tableName = ConversionUtils.initTableName( tableNameBase,
                DataTypes.BOOLEAN() );
        createTable( tableName, DataTypes.BOOLEAN(), filed_string, filed_bool );
        Row row = ConversionUtils.queryOne( tableEnv, tableName );
        Assert.assertEquals( row.getField( filed_string ), false );
        Assert.assertEquals( row.getField( filed_bool ), true );
    }

    public void toDECIMAL() throws Exception {
        String tableName = ConversionUtils.initTableName( tableNameBase,
                DataTypes.DECIMAL( 38, 14 ) );
        createTable( tableName, DataTypes.DECIMAL( 38, 14 ), filed_string );
        Row row = ConversionUtils.queryOne( tableEnv, tableName );
        Assert.assertNull(row.getField(filed_string));
    }

    private void toCHAR() throws Exception {
        DataType[] dataTypes = { DataTypes.CHAR( Integer.MAX_VALUE ),
                DataTypes.VARCHAR( Integer.MAX_VALUE ), DataTypes.STRING() };
        for ( int i = 0; i < dataTypes.length; i++ ) {
            String tableName = ConversionUtils.initTableName( tableNameBase,
                    dataTypes[ i ] );
            createTable( tableName, dataTypes[ i ], filed_string, filed_bool );
            Row row = ConversionUtils.queryOne( tableEnv, tableName );
            Assert.assertEquals( row.getField( filed_string ), data_string );
            Assert.assertEquals( row.getField( filed_bool ), bool_data );
        }
    }

    public void toBINARY() throws Exception {
        DataType[] dataTypes = { DataTypes.BINARY( Integer.MAX_VALUE ),
                DataTypes.VARBINARY( Integer.MAX_VALUE ), DataTypes.BYTES() };
        for ( int i = 0; i < dataTypes.length; i++ ) {
            String tableName = ConversionUtils.initTableName( tableNameBase,
                    dataTypes[ i ] );
            createTable( tableName, dataTypes[ i ], filed_string, filed_bool );
            Row row = ConversionUtils.queryOne( tableEnv, tableName );
            Assert.assertEquals( row.getField( filed_string ),
                    data_string.getBytes( StandardCharsets.UTF_8 ) );
            Assert.assertEquals( row.getField( filed_bool ),
                    bool_data.getBytes( StandardCharsets.UTF_8 ) );
        }
    }

    @AfterClass
    public void tearDown() {
        try {
            sdb.dropCollectionSpace( csName );
        } finally {
            sdb.close();
        }
    }

    private void insertData() {
        BasicBSONObject bson = new BasicBSONObject();
        bson.put( filed_string, data_string );
        // 拆入的String类型时间时需要格式化为指定类型才能转换
        SimpleDateFormat sdf = new SimpleDateFormat( "yyyy-MM-dd.HH:mm:ss" );
        bson.put( filed_date, sdf.format( new Date( data_date ) ) );
        bson.put( filed_bool, bool_data );
        System.out
                .println( "CreateTable25308 inset timestamp is " + data_date );
        cl.insert( bson );
    }

    private void createTable( String tableName, DataType type,
            String... fileds ) {
        Schema schema = ConversionUtils.createSchemaByDataType( type, fileds );
        TableDescriptor tableDescriptor = Commlib.createTableDescriptor( schema,
                csName, clName );
        tableEnv.createTable( tableName, tableDescriptor );
    }
}