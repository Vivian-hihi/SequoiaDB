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
import java.util.Date;

/**
 * @descreption seqDB-25306:创建SDB映射表，bool类型转换
 * @author YiPan
 * @date 2022/1/29
 * @updateUser
 * @updateDate
 * @updateRemark
 * @version 1.0
 */
public class CreateTable25306 extends FlinkTestBase {
    private Sequoiadb sdb;
    private StreamTableEnvironment tableEnv;
    private DBCollection cl;
    private final String csName = "cs_25306";
    private final String clName = "cl_25306";
    private final String tableNameBase = "tb_25306_";
    private final String filed_true = "test_true";
    private final String filed_false = "test_false";
    private final boolean data_true = true;
    private final boolean data_false = false;

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
        // 转换成功数值类型，预期true转1 false转0
        toSuccessType();

        // 转换成Date类型，预期不兼容
        toDate();

        // 转换成TIMESTAMP类型，预期不兼容
        toTIMESTAMP();

        // 转换成BOOLEAN类型，预期兼容
        toBOOLEAN();

        // 转换成DECIMAL类型，预期true转1 false转0
        toDECIMAL();

        // 转换成字符串类型,预期兼容
        toCHAR();

        // 转换成二进制类型，预期兼容
        toBINARY();
    }

    public void toSuccessType() throws Exception {
        DataType[] dataTypes = { DataTypes.TINYINT(), DataTypes.SMALLINT(),
                DataTypes.INT(), DataTypes.BIGINT(), DataTypes.FLOAT(),
                DataTypes.DOUBLE() };
        for ( int i = 0; i < dataTypes.length; i++ ) {
            String tableName = ConversionUtils.initTableName( tableNameBase,
                    dataTypes[ i ] );
            createTable( tableName, dataTypes[ i ], filed_true, filed_false );
            Row row = ConversionUtils.queryOne( tableEnv, tableName );
            Assert.assertEquals( row.getField( filed_true ),
                    ConversionUtils.toDataType( "1", dataTypes[ i ] ) );
            Assert.assertEquals( row.getField( filed_false ),
                    ConversionUtils.toDataType( "0", dataTypes[ i ] ) );
        }
    }

    public void toDate() throws Exception {
        String tableName = ConversionUtils.initTableName( tableNameBase,
                DataTypes.DATE() );
        createTable( tableName, DataTypes.DATE(), filed_true, filed_false );
        Row row = ConversionUtils.queryOne( tableEnv, tableName );
        Assert.assertEquals( row.getField( filed_true ),
                ConversionUtils.getLocalDate( new Date( 0 ) ) );
        Assert.assertEquals( row.getField( filed_false ),
                ConversionUtils.getLocalDate( new Date( 0 ) ) );
    }

    public void toTIMESTAMP() throws Exception {
        String tableName = ConversionUtils.initTableName( tableNameBase,
                DataTypes.TIMESTAMP() );
        createTable( tableName, DataTypes.TIMESTAMP(), filed_true,
                filed_false );
        Row row = ConversionUtils.queryOne( tableEnv, tableName );
        Assert.assertEquals( row.getField( filed_true ),
                ConversionUtils.getLocalDateTime( new Timestamp( 0 ) ) );
        Assert.assertEquals( row.getField( filed_false ),
                ConversionUtils.getLocalDateTime( new Timestamp( 0 ) ) );
    }

    public void toBOOLEAN() throws Exception {
        String tableName = ConversionUtils.initTableName( tableNameBase,
                DataTypes.BOOLEAN() );
        createTable( tableName, DataTypes.BOOLEAN(), filed_true, filed_false );
        Row row = ConversionUtils.queryOne( tableEnv, tableName );
        Assert.assertEquals( row.getField( filed_true ), data_true );
        Assert.assertEquals( row.getField( filed_false ), data_false );
    }

    public void toDECIMAL() throws Exception {
        int precision = 38;
        int scale = 14;
        String tableName = ConversionUtils.initTableName( tableNameBase,
                DataTypes.DECIMAL( precision, scale ) );
        createTable( tableName, DataTypes.DECIMAL( precision, scale ),
                filed_true, filed_false );
        Row row = ConversionUtils.queryOne( tableEnv, tableName );
        // 小数点后的位数根据scale确定
        Assert.assertEquals( row.getField( filed_true ),
                new BigDecimal( "1.00000000000000" ) );
        Assert.assertEquals( row.getField( filed_false ),
                new BigDecimal( "0.00000000000000" ) );
    }

    private void toCHAR() throws Exception {
        DataType[] dataTypes = { DataTypes.CHAR( Integer.MAX_VALUE ),
                DataTypes.VARCHAR( Integer.MAX_VALUE ), DataTypes.STRING() };
        for ( int i = 0; i < dataTypes.length; i++ ) {
            String tableName = ConversionUtils.initTableName( tableNameBase,
                    dataTypes[ i ] );
            createTable( tableName, dataTypes[ i ], filed_true, filed_false );
            Row row = ConversionUtils.queryOne( tableEnv, tableName );
            Assert.assertEquals( row.getField( filed_true ),
                    String.valueOf( data_true ) );
            Assert.assertEquals( row.getField( filed_false ),
                    String.valueOf( data_false ) );
        }
    }

    public void toBINARY() throws Exception {
        DataType[] dataTypes = { DataTypes.BINARY( Integer.MAX_VALUE ),
                DataTypes.VARBINARY( Integer.MAX_VALUE ), DataTypes.BYTES() };
        byte[] except_trueBytes = { 1 };
        byte[] except_falseBytes = { 0 };
        for ( int i = 0; i < dataTypes.length; i++ ) {
            String tableName = ConversionUtils.initTableName( tableNameBase,
                    dataTypes[ i ] );
            createTable( tableName, dataTypes[ i ], filed_true, filed_false );
            Row row = ConversionUtils.queryOne( tableEnv, tableName );
            Assert.assertEquals( row.getField( filed_true ), except_trueBytes );
            Assert.assertEquals( row.getField( filed_false ),
                    except_falseBytes );
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