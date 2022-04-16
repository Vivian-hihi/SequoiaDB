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
import org.bson.types.ObjectId;
import org.testng.Assert;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import java.math.BigDecimal;
import java.sql.Date;
import java.sql.Timestamp;

/**
 * @descreption seqDB-25305:创建SDB映射表，objectid类型转换
 * @author YiPan
 * @date 2022/2/22
 * @updateUser
 * @updateDate
 * @updateRemark
 * @version 1.0
 */
public class CreateTable25305 extends FlinkTestBase {
    private Sequoiadb sdb;
    private StreamTableEnvironment tableEnv;
    private DBCollection cl;
    private final String csName = "cs_25305";
    private final String clName = "cl_25305";
    private final String tableNameBase = "tb_25305_";
    private final String filed_ObjectId = "_id";
    private ObjectId data_ObjectId = new ObjectId();

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
        // 转换成TINYINT类型
        toTINYINT();

        // 转换成SMALLINT类型
        toSMALLINT();

        // 转换成INT类型
        toINT();

        // 转换成BIGINT类型
        toBIGINT();

        // 转换成FLOAT类型
        toFLOAT();

        // 转换成DOUBLE类型
        toDOUBLE();

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

    public void toTINYINT() throws Exception {
        String tableName = ConversionUtils.initTableName( tableNameBase,
                DataTypes.TINYINT() );
        createTable( tableName, DataTypes.TINYINT(), filed_ObjectId );
        Row row = ConversionUtils.queryOne( tableEnv, tableName );
        Assert.assertEquals( row.getField( filed_ObjectId ), new Byte( "0" ) );

    }

    public void toSMALLINT() throws Exception {
        String tableName = ConversionUtils.initTableName( tableNameBase,
                DataTypes.SMALLINT() );
        createTable( tableName, DataTypes.SMALLINT(), filed_ObjectId );
        Row row = ConversionUtils.queryOne( tableEnv, tableName );
        Assert.assertEquals( row.getField( filed_ObjectId ),
                Short.parseShort( "0" ) );
    }

    public void toINT() throws Exception {
        String tableName = ConversionUtils.initTableName( tableNameBase,
                DataTypes.INT() );
        createTable( tableName, DataTypes.INT(), filed_ObjectId );
        Row row = ConversionUtils.queryOne( tableEnv, tableName );
        Assert.assertEquals( row.getField( filed_ObjectId ),
                Integer.parseInt( "0" ) );
    }

    public void toBIGINT() throws Exception {
        String tableName = ConversionUtils.initTableName( tableNameBase,
                DataTypes.BIGINT() );
        createTable( tableName, DataTypes.BIGINT(), filed_ObjectId );
        Row row = ConversionUtils.queryOne( tableEnv, tableName );
        Assert.assertEquals( row.getField( filed_ObjectId ),
                Long.parseLong( "0" ) );
    }

    public void toFLOAT() throws Exception {
        String tableName = ConversionUtils.initTableName( tableNameBase,
                DataTypes.FLOAT() );
        createTable( tableName, DataTypes.FLOAT(), filed_ObjectId );
        Row row = ConversionUtils.queryOne( tableEnv, tableName );
        Assert.assertEquals( row.getField( filed_ObjectId ),
                Float.parseFloat( "0" ) );
    }

    public void toDOUBLE() throws Exception {
        String tableName = ConversionUtils.initTableName( tableNameBase,
                DataTypes.DOUBLE() );
        createTable( tableName, DataTypes.DOUBLE(), filed_ObjectId );
        Row row = ConversionUtils.queryOne( tableEnv, tableName );
        Assert.assertEquals( row.getField( filed_ObjectId ),
                Double.parseDouble( "0" ) );
    }

    public void toDate() throws Exception {
        String tableName = ConversionUtils.initTableName( tableNameBase,
                DataTypes.DATE() );
        createTable( tableName, DataTypes.DATE(), filed_ObjectId );
        Row row = ConversionUtils.queryOne( tableEnv, tableName );
        Assert.assertEquals( row.getField( filed_ObjectId ),
                ConversionUtils.getLocalDate( new Date( 0 ) ) );
    }

    public void toTIMESTAMP() throws Exception {
        String tableName = ConversionUtils.initTableName( tableNameBase,
                DataTypes.TIMESTAMP() );
        createTable( tableName, DataTypes.TIMESTAMP(), filed_ObjectId );
        Row row = ConversionUtils.queryOne( tableEnv, tableName );
        Assert.assertEquals( row.getField( filed_ObjectId ),
                ConversionUtils.getLocalDateTime( new Timestamp( 0 ) ) );
    }

    public void toBOOLEAN() throws Exception {
        String tableName = ConversionUtils.initTableName( tableNameBase,
                DataTypes.BOOLEAN() );
        createTable( tableName, DataTypes.BOOLEAN(), filed_ObjectId );
        Row row = ConversionUtils.queryOne( tableEnv, tableName );
        Assert.assertEquals( row.getField( filed_ObjectId ), false );
    }

    public void toDECIMAL() throws Exception {
        String tableName = ConversionUtils.initTableName( tableNameBase,
                DataTypes.DECIMAL( 38, 14 ) );
        createTable( tableName, DataTypes.DECIMAL( 38, 14 ), filed_ObjectId );
        Row row = ConversionUtils.queryOne( tableEnv, tableName );
        // 小数点后的位数根据scale确定
        Assert.assertEquals( row.getField( filed_ObjectId ),
                new BigDecimal( "0.00000000000000" ) );

    }

    private void toCHAR() throws Exception {
        DataType[] dataTypes = { DataTypes.CHAR( Integer.MAX_VALUE ),
                DataTypes.VARCHAR( Integer.MAX_VALUE ), DataTypes.STRING() };
        for ( int i = 0; i < dataTypes.length; i++ ) {
            String tableName = ConversionUtils.initTableName( tableNameBase,
                    dataTypes[ i ] );
            createTable( tableName, dataTypes[ i ], filed_ObjectId );
            Row row = ConversionUtils.queryOne( tableEnv, tableName );
            Assert.assertEquals( row.getField( filed_ObjectId ),
                    data_ObjectId.toString() );
        }
    }

    public void toBINARY() throws Exception {
        DataType[] dataTypes = { DataTypes.BINARY( Integer.MAX_VALUE ),
                DataTypes.VARBINARY( Integer.MAX_VALUE ), DataTypes.BYTES() };
        byte[] except_Id_Bytes = data_ObjectId.toByteArray();
        for ( int i = 0; i < dataTypes.length; i++ ) {
            String tableName = ConversionUtils.initTableName( tableNameBase,
                    dataTypes[ i ] );
            createTable( tableName, dataTypes[ i ], filed_ObjectId );
            Row row = ConversionUtils.queryOne( tableEnv, tableName );
            Assert.assertEquals( row.getField( filed_ObjectId ),
                    except_Id_Bytes );
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
        bson.put( filed_ObjectId, data_ObjectId );
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