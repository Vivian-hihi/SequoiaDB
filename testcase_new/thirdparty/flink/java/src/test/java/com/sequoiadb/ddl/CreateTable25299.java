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

import java.math.BigDecimal;
import java.sql.Timestamp;
import java.util.ArrayList;
import java.util.Calendar;
import java.util.Date;

/**
 * @descreption seqDB-25299:创建SDB映射表，int32类型转换
 * @author YiPan
 * @date 2022/1/29
 * @updateUser
 * @updateDate
 * @updateRemark
 * @version 1.0
 */
public class CreateTable25299 extends FlinkTestBase {
    private Sequoiadb sdb;
    private StreamTableEnvironment tableEnv;
    private DBCollection cl;
    private final String csName = "cs_25299";
    private final String clName = "cl_25299";
    private final String tableNameBase = "tb_25299_";
    private String filed_min = "test_min";
    private String filed_max = "test_max";
    private String filed_zero = "test_zero";
    private String filed_test = "test_test";
    private int data_min = Integer.MIN_VALUE;
    private int data_max = Integer.MAX_VALUE;
    private int data_test = 10;
    private int data_zero = 0;

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
        // 转换可能丢失精度数值类型
        testLostAccuracyType();

        // 转换成功数值类型，预期兼容
        toSuccessType();

        // 转换成Date类型，可能丢失精度
        toDate();

        // 转换成TIMESTAMP类型，可能丢失精度
        toTIMESTAMP();

        // 转换成BOOLEAN类型，预期兼容
        toBOOLEAN();

        // 转换成DECIMAL类型，预期兼容
        toDECIMAL();

        // 转换成字符串类型,预期兼容
        toCHAR();

        // 转换成二进制类型，预期兼容
        toBINARY();
    }

    private void testLostAccuracyType() {
        DataType[] dataTypes = { DataTypes.TINYINT(), DataTypes.SMALLINT() };
        for ( int i = 0; i < dataTypes.length; i++ ) {
            String tableName = ConversionUtils.initTableName( tableNameBase,
                    dataTypes[ i ] );
            createTable( tableName, dataTypes[ i ], filed_min, filed_max );
            ArrayList< Row > rows = Commlib.collectToArrayList( tableEnv
                    .executeSql( "select * from " + tableName ).collect() );
            Assert.assertEquals( rows.size(), 1 );
        }
    }

    public void toSuccessType() throws Exception {
        DataType[] dataTypes = { DataTypes.INT(), DataTypes.BIGINT(),
                DataTypes.FLOAT(), DataTypes.DOUBLE() };
        for ( int i = 0; i < dataTypes.length; i++ ) {
            String tableName = ConversionUtils.initTableName( tableNameBase,
                    dataTypes[ i ] );
            createTable( tableName, dataTypes[ i ], filed_min, filed_max );
            Row row = ConversionUtils.queryOne( tableEnv, tableName );
            Assert.assertEquals( row.getField( filed_min ), ConversionUtils
                    .toDataType( String.valueOf( data_min ), dataTypes[ i ] ) );
            Assert.assertEquals( row.getField( filed_max ), ConversionUtils
                    .toDataType( String.valueOf( data_max ), dataTypes[ i ] ) );
        }
    }

    public void toDate() throws Exception {
        String tableName = ConversionUtils.initTableName( tableNameBase,
                DataTypes.DATE() );
        createTable( tableName, DataTypes.DATE(), filed_zero, filed_test );
        Row row = ConversionUtils.queryOne( tableEnv, tableName );
        Assert.assertEquals( row.getField( filed_zero ),
                ConversionUtils.getLocalDate( new Date( data_zero ) ) );
        // int32转换date类型时，int32记录的从1970-1-1开始的天数
        Assert.assertEquals( row.getField( filed_test ),
                ConversionUtils.getLocalDate( addDay( data_test ) ) );
    }

    public void toTIMESTAMP() throws Exception {
        String tableName = ConversionUtils.initTableName( tableNameBase,
                DataTypes.TIMESTAMP() );
        createTable( tableName, DataTypes.TIMESTAMP(), filed_zero, filed_test );
        Row row = ConversionUtils.queryOne( tableEnv, tableName );
        Assert.assertEquals( row.getField( filed_zero ), ConversionUtils
                .getLocalDateTime( new Timestamp( data_zero ) ) );
        Assert.assertEquals( row.getField( filed_test ), ConversionUtils
                .getLocalDateTime( new Timestamp( data_test ) ) );
    }

    public void toBOOLEAN() throws Exception {
        String tableName = ConversionUtils.initTableName( tableNameBase,
                DataTypes.BOOLEAN() );
        createTable( tableName, DataTypes.BOOLEAN(), filed_min, filed_max,
                filed_zero );
        Row row = ConversionUtils.queryOne( tableEnv, tableName );
        Assert.assertEquals( row.getField( filed_min ), true );
        Assert.assertEquals( row.getField( filed_max ), true );
        Assert.assertEquals( row.getField( filed_zero ), false );
    }

    public void toDECIMAL() throws Exception {
        int precision = 38;
        int scale = 10;
        String tableName = ConversionUtils.initTableName( tableNameBase,
                DataTypes.DECIMAL( precision, scale ) );
        createTable( tableName, DataTypes.DECIMAL( precision, scale ),
                filed_min, filed_max );
        Row row = ConversionUtils.queryOne( tableEnv, tableName );
        // 小数点后的位数根据scale确定
        Assert.assertEquals( row.getField( filed_min ),
                new BigDecimal( data_min + ".0000000000" ) );
        Assert.assertEquals( row.getField( filed_max ),
                new BigDecimal( data_max + ".0000000000" ) );
    }

    private void toCHAR() throws Exception {
        DataType[] dataTypes = { DataTypes.CHAR( Integer.MAX_VALUE ),
                DataTypes.VARCHAR( Integer.MAX_VALUE ), DataTypes.STRING() };
        for ( int i = 0; i < dataTypes.length; i++ ) {
            String tableName = ConversionUtils.initTableName( tableNameBase,
                    dataTypes[ i ] );
            createTable( tableName, dataTypes[ i ], filed_min, filed_max );
            Row row = ConversionUtils.queryOne( tableEnv, tableName );
            Assert.assertEquals( row.getField( filed_min ),
                    String.valueOf( data_min ) );
            Assert.assertEquals( row.getField( filed_max ),
                    String.valueOf( data_max ) );
        }
    }

    public void toBINARY() throws Exception {
        DataType[] dataTypes = { DataTypes.BINARY( Integer.MAX_VALUE ),
                DataTypes.VARBINARY( Integer.MAX_VALUE ), DataTypes.BYTES() };
        for ( int i = 0; i < dataTypes.length; i++ ) {
            String tableName = ConversionUtils.initTableName( tableNameBase,
                    dataTypes[ i ] );
            createTable( tableName, dataTypes[ i ], filed_min, filed_max );
            Row row = ConversionUtils.queryOne( tableEnv, tableName );
            Assert.assertEquals( row.getField( filed_min ),
                    ConversionUtils.toBytes( data_min ) );
            Assert.assertEquals( row.getField( filed_max ),
                    ConversionUtils.toBytes( data_max ) );
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
        bson.put( filed_min, data_min );
        bson.put( filed_max, data_max );
        bson.put( filed_zero, data_zero );
        bson.put( filed_test, data_test );
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

    private Date addDay( int i ) {
        Date date = new Date( 0 );
        Calendar instance = Calendar.getInstance();
        instance.setTime( date );
        instance.add( Calendar.DATE, i );
        return instance.getTime();
    }
}