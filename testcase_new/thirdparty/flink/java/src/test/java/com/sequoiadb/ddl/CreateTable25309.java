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
import java.nio.charset.StandardCharsets;
import java.sql.Timestamp;
import java.util.Arrays;
import java.util.Date;

/**
 * @descreption seqDB-25309:创建SDB映射表，binary类型转换
 * @author YiPan
 * @date 2022/2/25
 * @updateUser
 * @updateDate
 * @updateRemark
 * @version 1.0
 */
public class CreateTable25309 extends FlinkTestBase {
    private Sequoiadb sdb;
    private StreamTableEnvironment tableEnv;
    private DBCollection cl;
    private final String csName = "cs_25309";
    private final String clName = "cl_25309";
    private final String tableNameBase = "tb_25309_";
    private final String filed_min = "test_min";
    private final String filed_max = "test_max";
    private final String filed_String = "test_String";
    private final byte[] data_min = { 0 };
    private final byte[] data_max = { 127, 0, -127 };
    private String data_string = "sdb";

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
        // 转换不兼容类型
        toUnSupportType();

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

    public void toUnSupportType() throws Exception {
        DataType[] dataTypes = { DataTypes.BIGINT(), DataTypes.FLOAT(),
                DataTypes.DOUBLE() };
        for ( int i = 0; i < dataTypes.length; i++ ) {
            String tableName = ConversionUtils.initTableName( tableNameBase,
                    dataTypes[ i ] );
            createTable( tableName, dataTypes[ i ], filed_min, filed_max );
            Row row = ConversionUtils.queryOne( tableEnv, tableName );
            Assert.assertEquals( row.getField( filed_min ), ConversionUtils
                    .toDataType( String.valueOf( 0 ), dataTypes[ i ] ) );
            Assert.assertEquals( row.getField( filed_max ), ConversionUtils
                    .toDataType( String.valueOf( 0 ), dataTypes[ i ] ) );
        }
    }

    public void toDate() throws Exception {
        String tableName = ConversionUtils.initTableName( tableNameBase,
                DataTypes.DATE() );
        createTable( tableName, DataTypes.DATE(), filed_min, filed_max );
        tableEnv.executeSql( "select * from " + tableName );
        Row row = ConversionUtils.queryOne( tableEnv, tableName );
        Assert.assertEquals( row.getField( filed_min ),
                ConversionUtils.getLocalDate( new Date( 0 ) ) );
        Assert.assertEquals( row.getField( filed_max ),
                ConversionUtils.getLocalDate( new Date( 0 ) ) );
    }

    public void toTIMESTAMP() throws Exception {
        String tableName = ConversionUtils.initTableName( tableNameBase,
                DataTypes.TIMESTAMP() );
        createTable( tableName, DataTypes.TIMESTAMP(), filed_min, filed_max );
        tableEnv.executeSql( "select * from " + tableName );
        Row row = ConversionUtils.queryOne( tableEnv, tableName );
        Assert.assertEquals( row.getField( filed_min ),
                ConversionUtils.getLocalDateTime( new Timestamp( 0 ) ) );
        Assert.assertEquals( row.getField( filed_max ),
                ConversionUtils.getLocalDateTime( new Timestamp( 0 ) ) );
    }

    public void toBOOLEAN() throws Exception {
        String tableName = ConversionUtils.initTableName( tableNameBase,
                DataTypes.BOOLEAN() );
        createTable( tableName, DataTypes.BOOLEAN(), filed_min, filed_max );
        Row row = ConversionUtils.queryOne( tableEnv, tableName );
        Assert.assertEquals( row.getField( filed_min ), false );
        Assert.assertEquals( row.getField( filed_max ), false );
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
                new BigDecimal( "0.0000000000" ) );
        Assert.assertEquals( row.getField( filed_max ),
                new BigDecimal( ".0000000000" ) );
    }

    private void toCHAR() throws Exception {
        DataType[] dataTypes = { DataTypes.CHAR( Integer.MAX_VALUE ),
                DataTypes.VARCHAR( Integer.MAX_VALUE ), DataTypes.STRING() };
        for ( int i = 0; i < dataTypes.length; i++ ) {
            String tableName = ConversionUtils.initTableName( tableNameBase,
                    dataTypes[ i ] );
            createTable( tableName, dataTypes[ i ], filed_String );
            Row row = ConversionUtils.queryOne( tableEnv, tableName );
            Assert.assertEquals( row.getField( filed_String ), data_string );
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
            Assert.assertEquals( row.getField( filed_min ), data_min );
            Assert.assertEquals( row.getField( filed_max ), data_max );
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
        bson.put( filed_String,
                data_string.getBytes( StandardCharsets.UTF_8 ) );
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