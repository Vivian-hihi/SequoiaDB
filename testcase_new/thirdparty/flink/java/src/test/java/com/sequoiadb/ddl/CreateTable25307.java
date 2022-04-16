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
import org.bson.BSONObject;
import org.bson.BasicBSONObject;
import org.testng.Assert;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import java.math.BigDecimal;
import java.sql.Timestamp;
import java.util.ArrayList;
import java.util.Date;

/**
 * @descreption seqDB-25307:创建SDB映射表，date类型转换
 * @author YiPan
 * @date 2022/2/23
 * @updateUser
 * @updateDate
 * @updateRemark
 * @version 1.0
 */
public class CreateTable25307 extends FlinkTestBase {
    private Sequoiadb sdb;
    private StreamTableEnvironment tableEnv;
    private DBCollection cl;
    private final String csName = "cs_25307";
    private final String clName = "cl_25307";
    private final String tableNameBase = "tb_25307_";
    private final String filed_Zero = "test_zero";
    private final String filed_Now = "test_now";
    private long data_Zero;
    private long data_Now;

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

        // 转换成功数值类型
        toSuccessType();

        // 转换成Date类型，预期兼容
        toDate();

        // 转换成TIMESTAMP类型，兼容
        toTIMESTAMP();

        // 转换成BOOLEAN类型，预期不兼容
        toBOOLEAN();

        // 转换成DECIMAL类型，预期兼容
        toDECIMAL();

        // 转换为CHAR/VARCHAR/STRING
        toCHAR();

        // 转换成二进制类型，预期兼容
        toBINARY();
    }

    private void testLostAccuracyType() {
        DataType[] dataTypes = { DataTypes.TINYINT(), DataTypes.SMALLINT(),
                DataTypes.INT() };
        for ( int i = 0; i < dataTypes.length; i++ ) {
            String tableName = ConversionUtils.initTableName( tableNameBase,
                    dataTypes[ i ] );
            createTable( tableName, dataTypes[ i ], filed_Zero, filed_Now );
            ArrayList< Row > rows = Commlib.collectToArrayList( tableEnv
                    .executeSql( "select * from " + tableName ).collect() );
            Assert.assertEquals( rows.size(), 1 );
        }
    }

    public void toSuccessType() throws Exception {
        DataType[] dataTypes = { DataTypes.BIGINT(), DataTypes.FLOAT(),
                DataTypes.DOUBLE() };
        for ( int i = 0; i < dataTypes.length; i++ ) {

            String tableName = ConversionUtils.initTableName( tableNameBase,
                    dataTypes[ i ] );
            createTable( tableName, dataTypes[ i ], filed_Zero, filed_Now );
            Row row = ConversionUtils.queryOne( tableEnv, tableName );
            Assert.assertEquals( row.getField( filed_Zero ),
                    ConversionUtils.toDataType( String.valueOf( data_Zero ),
                            dataTypes[ i ] ) );
            Assert.assertEquals( row.getField( filed_Now ), ConversionUtils
                    .toDataType( String.valueOf( data_Now ), dataTypes[ i ] ) );
        }
    }

    public void toDate() throws Exception {
        String tableName = ConversionUtils.initTableName( tableNameBase,
                DataTypes.DATE() );
        createTable( tableName, DataTypes.DATE(), filed_Zero, filed_Now );
        Row row = ConversionUtils.queryOne( tableEnv, tableName );
        Assert.assertEquals( row.getField( filed_Zero ),
                ConversionUtils.getLocalDate( new Date( data_Zero ) ) );
        Assert.assertEquals( row.getField( filed_Now ),
                ConversionUtils.getLocalDate( new Date( data_Now ) ) );
    }

    public void toTIMESTAMP() throws Exception {
        String tableName = ConversionUtils.initTableName( tableNameBase,
                DataTypes.TIMESTAMP() );
        createTable( tableName, DataTypes.TIMESTAMP(), filed_Zero, filed_Now );
        Row row = ConversionUtils.queryOne( tableEnv, tableName );
        Assert.assertEquals( row.getField( filed_Zero ), ConversionUtils
                .getLocalDateTime( new Timestamp( data_Zero ) ) );
        Assert.assertEquals( row.getField( filed_Now ),
                ConversionUtils.getLocalDateTime( new Timestamp( data_Now ) ) );
    }

    public void toBOOLEAN() throws Exception {
        String tableName = ConversionUtils.initTableName( tableNameBase,
                DataTypes.BOOLEAN() );
        createTable( tableName, DataTypes.BOOLEAN(), filed_Zero, filed_Now );
        Row row = ConversionUtils.queryOne( tableEnv, tableName );
        Assert.assertEquals( row.getField( filed_Zero ), false );
        Assert.assertEquals( row.getField( filed_Now ), false );
    }

    public void toDECIMAL() throws Exception {
        int precision = 38;
        int scale = 4;
        String tableName = ConversionUtils.initTableName( tableNameBase,
                DataTypes.DECIMAL( precision, scale ) );
        createTable( tableName, DataTypes.DECIMAL( precision, scale ),
                filed_Zero, filed_Now );
        Row row = ConversionUtils.queryOne( tableEnv, tableName );
        // 小数点后的位数根据scale确定
        Assert.assertEquals( row.getField( filed_Zero ),
                new BigDecimal( data_Zero + ".0000" ) );
        Assert.assertEquals( row.getField( filed_Now ),
                new BigDecimal( data_Now + ".0000" ) );
    }

    private void toCHAR() throws Exception {
        DataType[] dataTypes = { DataTypes.CHAR( Integer.MAX_VALUE ),
                DataTypes.VARCHAR( Integer.MAX_VALUE ), DataTypes.STRING() };
        for ( int i = 0; i < dataTypes.length; i++ ) {
            String tableName = ConversionUtils.initTableName( tableNameBase,
                    dataTypes[ i ] );
            createTable( tableName, dataTypes[ i ], filed_Zero, filed_Now );
            Row row = ConversionUtils.queryOne( tableEnv, tableName );
            Assert.assertEquals( row.getField( filed_Zero ), ConversionUtils
                    .getLocalDate( new Date( data_Zero ) ).toString() );
            Assert.assertEquals( row.getField( filed_Now ), ConversionUtils
                    .getLocalDate( new Date( data_Now ) ).toString() );
        }
    }

    public void toBINARY() throws Exception {
        DataType[] dataTypes = { DataTypes.BINARY( Integer.MAX_VALUE ),
                DataTypes.VARBINARY( Integer.MAX_VALUE ), DataTypes.BYTES() };
        for ( int i = 0; i < dataTypes.length; i++ ) {
            String tableName = ConversionUtils.initTableName( tableNameBase,
                    dataTypes[ i ] );
            createTable( tableName, dataTypes[ i ], filed_Zero, filed_Now );
            Row row = ConversionUtils.queryOne( tableEnv, tableName );
            Assert.assertEquals( row.getField( filed_Zero ),
                    ConversionUtils.toBytes( data_Zero ) );
            Assert.assertEquals( row.getField( filed_Now ),
                    ConversionUtils.toBytes( data_Now ) );
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
        bson.put( filed_Zero, new Date( 0 ) );
        bson.put( filed_Now, new Date( System.currentTimeMillis() ) );
        cl.insert( bson );
        BSONObject result = cl.queryOne();
        data_Now = ( ( Date ) result.get( filed_Now ) ).getTime();
        data_Zero = ( ( Date ) result.get( filed_Zero ) ).getTime();
        System.out.println( "CreateTable25307 insert date is " + data_Now );
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