package com.sequoiadb.ddl;

import com.sequoiadb.base.CollectionSpace;
import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.testcommon.FlinkTestBase;
import com.sequoiadb.testcommon.utils.Commlib;
import com.sequoiadb.testcommon.utils.ConversionUtils;
import org.apache.commons.lang3.RandomStringUtils;
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

import java.math.BigDecimal;
import java.nio.charset.StandardCharsets;
import java.sql.Date;
import java.sql.Timestamp;
import java.text.SimpleDateFormat;
import java.time.LocalDateTime;
import java.time.format.DateTimeFormatter;

/**
 * @descreption seqDB-25303:创建SDB映射表，string类型数值字符串转换
 * @author YiPan
 * @date 2022/2/22
 * @updateUser
 * @updateDate
 * @updateRemark
 * @version 1.0
 */
public class CreateTable25303 extends FlinkTestBase {
    private Sequoiadb sdb;
    private StreamTableEnvironment tableEnv;
    private DBCollection cl;
    private final String csName = "cs_25303";
    private final String clName = "cl_25303";
    private final String tableNameBase = "tb_25303_";
    private final String filed_a = "test_a";
    private final String filed_b = "test_b";
    private final String filed_time = "test_time";
    private final String filed_true = "test_true";
    private final String filed_false = "test_false";
    private String filed_overflow;
    private final String data_overflow = "test_overflow";
    private final String data_a = "123";
    private final String data_b = "0";
    private final long data_time = System.currentTimeMillis();
    private final String data_true = "true";
    private final String data_false = "False";

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
        // 转换成支持的数值类型
        testSupport();

        // 转换成Date类型
        toDate();

        // // 转换成TIMESTAMP类型
        toTIMESTAMP();

        // 转换成BOOLEAN类型
        toBOOLEAN();

        // 转换成DECIMAL类型
        toDECIMAL();

        // 转换成字符串类型
        toCHAR();

        // 转换成二进制类型
        toBINARY();
    }

    public void testSupport() throws Exception {
        DataType[] dataTypes = { DataTypes.TINYINT(), DataTypes.SMALLINT(),
                DataTypes.INT(), DataTypes.BIGINT(), DataTypes.FLOAT(),
                DataTypes.DOUBLE() };
        for ( int i = 0; i < dataTypes.length; i++ ) {
            String tableName = ConversionUtils.initTableName( tableNameBase,
                    dataTypes[ i ] );
            createTable( tableName, dataTypes[ i ], filed_a, filed_b,
                    filed_overflow );
            Row row = ConversionUtils.queryOne( tableEnv, tableName );
            Assert.assertEquals( row.getField( filed_a ),
                    ConversionUtils.toDataType( data_a, dataTypes[ i ] ) );
            Assert.assertEquals( row.getField( filed_b ),
                    ConversionUtils.toDataType( data_b, dataTypes[ i ] ) );
            Assert.assertNull( row.getField( filed_overflow ) );
        }
    }

    public void toDate() throws Exception {
        String tableName = ConversionUtils.initTableName( tableNameBase,
                DataTypes.DATE() );
        createTable( tableName, DataTypes.DATE(), filed_a, filed_time );
        Row row = ConversionUtils.queryOne( tableEnv, tableName );
        Assert.assertEquals( row.getField( filed_a ),
                ConversionUtils.getLocalDate( new Date( 0 ) ) );
        Assert.assertEquals( row.getField( filed_time ),
                ConversionUtils.getLocalDate( new Date( data_time ) ) );
    }

    public void toTIMESTAMP() throws Exception {
        String tableName = ConversionUtils.initTableName( tableNameBase,
                DataTypes.TIMESTAMP() );
        createTable( tableName, DataTypes.TIMESTAMP(), filed_b, filed_time );
        Row row = ConversionUtils.queryOne( tableEnv, tableName );
        Assert.assertEquals( row.getField( filed_b ),
                ConversionUtils.getLocalDateTime( new Timestamp( 0 ) ) );
        DateTimeFormatter dtf = DateTimeFormatter
                .ofPattern( "yyyy-MM-dd'T'HH:mm:ss" );
        LocalDateTime expectLocalDateTime = ConversionUtils
                .getLocalDateTime( new Timestamp( data_time ) );
        // 时间格式不一样，需要格式化后转String校验
        Assert.assertEquals( row.getField( filed_time ).toString(),
                dtf.format( expectLocalDateTime ) );
    }

    public void toBOOLEAN() throws Exception {
        String tableName = ConversionUtils.initTableName( tableNameBase,
                DataTypes.BOOLEAN() );
        createTable( tableName, DataTypes.BOOLEAN(), filed_true, filed_false,
                filed_a );
        Row row = ConversionUtils.queryOne( tableEnv, tableName );
        Assert.assertEquals( row.getField( filed_true ), true );
        Assert.assertEquals( row.getField( filed_false ), false );
        Assert.assertEquals( row.getField( filed_a ), false );
    }

    public void toDECIMAL() throws Exception {
        String tableName = ConversionUtils.initTableName( tableNameBase,
                DataTypes.DECIMAL( 38, 14 ) );
        createTable( tableName, DataTypes.DECIMAL( 38, 14 ), filed_a, filed_b );
        Row row = ConversionUtils.queryOne( tableEnv, tableName );
        // 小数点后的位数根据scale确定
        Assert.assertEquals( row.getField( filed_a ),
                new BigDecimal( "123.00000000000000" ) );
        Assert.assertEquals( row.getField( filed_b ),
                new BigDecimal( "0.00000000000000" ) );

    }

    private void toCHAR() throws Exception {
        DataType[] dataTypes = { DataTypes.CHAR( Integer.MAX_VALUE ),
                DataTypes.VARCHAR( Integer.MAX_VALUE ), DataTypes.STRING() };
        for ( int i = 0; i < dataTypes.length; i++ ) {
            String tableName = ConversionUtils.initTableName( tableNameBase,
                    dataTypes[ i ] );
            createTable( tableName, dataTypes[ i ], filed_a, filed_b,
                    filed_true, filed_false );
            Row row = ConversionUtils.queryOne( tableEnv, tableName );
            Assert.assertEquals( row.getField( filed_a ), data_a );
            Assert.assertEquals( row.getField( filed_b ), data_b );
            Assert.assertEquals( row.getField( filed_true ), data_true );
            Assert.assertEquals( row.getField( filed_false ), data_false );
        }
    }

    public void toBINARY() throws Exception {
        DataType[] dataTypes = { DataTypes.BINARY( Integer.MAX_VALUE ),
                DataTypes.VARBINARY( Integer.MAX_VALUE ), DataTypes.BYTES() };
        byte[] except_a_Bytes = data_a.getBytes( StandardCharsets.UTF_8 );
        byte[] except_b_Bytes = data_b.getBytes( StandardCharsets.UTF_8 );
        for ( int i = 0; i < dataTypes.length; i++ ) {
            String tableName = ConversionUtils.initTableName( tableNameBase,
                    dataTypes[ i ] );
            createTable( tableName, dataTypes[ i ], filed_a, filed_b );
            Row row = ConversionUtils.queryOne( tableEnv, tableName );
            Assert.assertEquals( row.getField( filed_a ), except_a_Bytes );
            Assert.assertEquals( row.getField( filed_b ), except_b_Bytes );
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
        filed_overflow = RandomStringUtils.random( 100, false, true );
        SimpleDateFormat simpleDateFormat = new SimpleDateFormat(
                "yyyy-MM-dd.HH:mm:ss" );
        BasicBSONObject bson = new BasicBSONObject();
        bson.put( filed_a, data_a );
        bson.put( filed_b, data_b );
        bson.put( filed_overflow, data_overflow );
        bson.put( filed_time, simpleDateFormat.format( data_time ) );
        bson.put( filed_true, data_true );
        bson.put( filed_false, data_false );
        cl.insert( bson );
    }

    private void createTable( String tableName, DataType dataType,
            String... fileds ) {
        Schema schema = ConversionUtils.createSchemaByDataType( dataType,
                fileds );
        TableDescriptor tableDescriptor = Commlib.createTableDescriptor( schema,
                csName, clName );
        tableEnv.createTable( tableName, tableDescriptor );
    }
}