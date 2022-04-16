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

import java.sql.Date;
import java.sql.Timestamp;
import java.util.ArrayList;

/**
 * @descreption seqDB-25301:创建SDB映射表，double类型转换
 * @author YiPan
 * @date 2022/1/29
 * @updateUser
 * @updateDate
 * @updateRemark
 * @version 1.0
 */
public class CreateTable25301 extends FlinkTestBase {
    private Sequoiadb sdb;
    private StreamTableEnvironment tableEnv;
    private DBCollection cl;
    private final String csName = "cs_25301";
    private final String clName = "cl_25301";
    private final String tableNameBase = "tb_25301_";
    private final String filed_max = "test_max";
    private final String filed_min = "test_mix";
    private final double data_min = Double.MIN_VALUE;
    private final double data_max = Double.MAX_VALUE;

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
        // 可能丢失精度的数值类型
        testLostAccuracyType();

        // 转换成功数值类型
        toSuccessType();

        // 转换成Date类型，预期转零
        toDate();

        // 转换成TIMESTAMP类型，预期转零
        toTIMESTAMP();

        // 转换成BOOLEAN类型，预期转true
        toBOOLEAN();

        // 转换成DECIMAL类型，预期可能超出范围
        toDECIMAL();

        // 转换成字符串类型,预期兼容
        toCHAR();

        // 转换成二进制类型，预期兼容
        toBINARY();
    }

    public void testLostAccuracyType() {
        DataType[] dataTypes = { DataTypes.TINYINT(), DataTypes.SMALLINT(),
                DataTypes.INT(), DataTypes.BIGINT(), DataTypes.FLOAT() };
        for ( int i = 0; i < dataTypes.length; i++ ) {
            String tableName = ConversionUtils.initTableName( tableNameBase,
                    dataTypes[ i ] );
            createTable( tableName, dataTypes[ i ], filed_max, filed_min );
            ArrayList< Row > rows = Commlib.collectToArrayList( tableEnv
                    .executeSql( "select * from " + tableName ).collect() );
            Assert.assertEquals( rows.size(), 1 );
        }
    }

    public void toSuccessType() throws Exception {
        String tableName = ConversionUtils.initTableName( tableNameBase,
                DataTypes.DOUBLE() );
        createTable( tableName, DataTypes.DOUBLE(), filed_max, filed_min );
        Row row = ConversionUtils.queryOne( tableEnv, tableName );
        Assert.assertEquals( row.getField( filed_max ), data_max );
        Assert.assertEquals( row.getField( filed_min ), data_min );
    }

    public void toDate() throws Exception {
        String tableName = ConversionUtils.initTableName( tableNameBase,
                DataTypes.DATE() );
        createTable( tableName, DataTypes.DATE(), filed_max, filed_min );
        Row row = ConversionUtils.queryOne( tableEnv, tableName );
        Assert.assertEquals( row.getField( filed_max ),
                ConversionUtils.getLocalDate( new Date( 0 ) ) );
        Assert.assertEquals( row.getField( filed_min ),
                ConversionUtils.getLocalDate( new Date( 0 ) ) );
    }

    public void toTIMESTAMP() throws Exception {
        String tableName = ConversionUtils.initTableName( tableNameBase,
                DataTypes.TIMESTAMP() );
        createTable( tableName, DataTypes.TIMESTAMP(), filed_max, filed_min );
        Row row = ConversionUtils.queryOne( tableEnv, tableName );
        Assert.assertEquals( row.getField( filed_min ),
                ConversionUtils.getLocalDateTime( new Timestamp( 0 ) ) );
        Assert.assertEquals( row.getField( filed_max ),
                ConversionUtils.getLocalDateTime( new Timestamp( 0 ) ) );
    }

    public void toBOOLEAN() throws Exception {
        String tableName = ConversionUtils.initTableName( tableNameBase,
                DataTypes.BOOLEAN() );
        createTable( tableName, DataTypes.BOOLEAN(), filed_max, filed_min );
        Row row = ConversionUtils.queryOne( tableEnv, tableName );
        Assert.assertEquals( row.getField( filed_max ), true );
        Assert.assertEquals( row.getField( filed_min ), true );
    }

    public void toDECIMAL() {
        String tableName = ConversionUtils.initTableName( tableNameBase,
                DataTypes.DECIMAL( 38, 14 ) );
        createTable( tableName, DataTypes.DECIMAL( 38, 14 ), filed_max,
                filed_min );
        ArrayList< Row > rows = Commlib.collectToArrayList( tableEnv
                .executeSql( "select * from " + tableName ).collect() );
        Assert.assertEquals( rows.size(), 1 );
    }

    private void toCHAR() throws Exception {
        DataType[] dataTypes = { DataTypes.CHAR( Integer.MAX_VALUE ),
                DataTypes.VARCHAR( Integer.MAX_VALUE ), DataTypes.STRING() };
        for ( int i = 0; i < dataTypes.length; i++ ) {
            String tableName = ConversionUtils.initTableName( tableNameBase,
                    dataTypes[ i ] );
            createTable( tableName, dataTypes[ i ], filed_max, filed_min );
            Row row = ConversionUtils.queryOne( tableEnv, tableName );
            Assert.assertEquals( row.getField( filed_max ),
                    String.valueOf( data_max ) );
            Assert.assertEquals( row.getField( filed_min ),
                    String.valueOf( data_min ) );
        }
    }

    public void toBINARY() throws Exception {
        DataType[] dataTypes = { DataTypes.BINARY( Integer.MAX_VALUE ),
                DataTypes.VARBINARY( Integer.MAX_VALUE ), DataTypes.BYTES() };
        byte[] except_minBytes = ConversionUtils
                .toBytes( Double.doubleToLongBits( data_min ) );
        byte[] except_maxBytes = ConversionUtils
                .toBytes( Double.doubleToLongBits( data_max ) );
        for ( int i = 0; i < dataTypes.length; i++ ) {
            String tableName = ConversionUtils.initTableName( tableNameBase,
                    dataTypes[ i ] );
            createTable( tableName, dataTypes[ i ], filed_max, filed_min );
            Row row = ConversionUtils.queryOne( tableEnv, tableName );
            Assert.assertEquals( row.getField( filed_max ), except_maxBytes );
            Assert.assertEquals( row.getField( filed_min ), except_minBytes );
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
        bson.put( filed_max, data_max );
        bson.put( filed_min, data_min );
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