package com.sequoiadb.ddl;

import com.sequoiadb.base.CollectionSpace;
import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.testcommon.FlinkTestBase;
import com.sequoiadb.testcommon.utils.Commlib;
import com.sequoiadb.testcommon.utils.ConversionUtils;
import com.sequoiadb.testcommon.utils.SDBAttribute;
import org.apache.flink.streaming.api.environment.StreamExecutionEnvironment;
import org.apache.flink.table.api.DataTypes;
import org.apache.flink.table.api.Schema;
import org.apache.flink.table.api.TableDescriptor;
import org.apache.flink.table.api.ValidationException;
import org.apache.flink.table.api.bridge.java.StreamTableEnvironment;
import org.apache.flink.types.Row;
import org.bson.BasicBSONObject;
import org.testng.Assert;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import java.util.Random;

/**
 * @descreption seqDB-25261:创建SDB映射表，指定映射参数
 * @author YiPan
 * @date 2022/2/22
 * @updateUser
 * @updateDate
 * @updateRemark
 * @version 1.0
 */
public class CreateTable25261 extends FlinkTestBase {
    private StreamTableEnvironment tableEnv;
    private final String csName = "cs_25261";
    private final String clName = "cl_25261";
    private final String filed_String = "test_s";
    private final String filed_int = "test_i";
    private final String data_String = "abc";
    private final int data_int = 123;
    private DBCollection cl;
    private Sequoiadb sdb;
    private final String tableNameBase = "tb_25261_";
    private Schema schema;

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
        schema = Schema.newBuilder().column( filed_String, DataTypes.STRING() )
                .column( filed_int, DataTypes.INT() ).build();
    }

    @Test
    public void test() throws Exception {
        //随机插入或者查询
        String[] sqls = new String[] { "select * from %s",
                "insert into %s values('abc',123)" };
        Random random = new Random();

        // with指定单个支持的key,value参数
        singleOptionTest( sqls[ random.nextInt( 2 ) ] );

        // with指定多个支持的key,value参数
        multipleOptionTest();

        // with指定支持的参数key大写
        capitalOptionTest( sqls[ random.nextInt( 2 ) ] );

        // with指定不支持的key,value参数
        unsupportOptionTest( sqls[ random.nextInt( 2 ) ] );
    }

    private void singleOptionTest( String randomSql ) {
        String tableName = tableNameBase + "single";
        TableDescriptor tableDescriptor = createSingleOption( schema ).build();
        tableEnv.createTable( tableName, tableDescriptor );
        try {
            tableEnv.executeSql( String.format( randomSql, tableName ) );
            Assert.fail( "except fail but success" );
        } catch ( ValidationException e ) {
            String message = e.getCause().getMessage();
            if ( !( message.contains(
                    "One or more required options are missing" ) ) ) {
                throw e;
            }
        }
    }

    private void multipleOptionTest() throws Exception {
        String tableName = tableNameBase + "multiple";
        TableDescriptor tableDescriptor = Commlib.createTableDescriptor( schema,
                csName, clName );
        tableEnv.createTable( tableName, tableDescriptor );
        Row row = ConversionUtils.queryOne( tableEnv, tableName );
        Assert.assertEquals( row.getField( filed_int ), data_int );
        Assert.assertEquals( row.getField( filed_String ), data_String );
    }

    private void capitalOptionTest( String randomSql ) {
        String tableName = tableNameBase + "capital";
        TableDescriptor.Builder option = Commlib.createBaseTableOption( schema,
                csName, clName );
        TableDescriptor tableDescriptor = option.option( "BULKSIZE", "300" )
                .build();
        tableEnv.createTable( tableName, tableDescriptor );
        try {
            tableEnv.executeSql( String.format( randomSql, tableName ) );
            Assert.fail( "except fail but success" );
        } catch ( ValidationException e ) {
            String message = e.getCause().getMessage();
            if ( !( message.contains(
                    "Unsupported options found for 'sequoiadb'" ) ) ) {
                throw e;
            }
        }
    }

    private void unsupportOptionTest( String randomSql ) {
        String tableName = tableNameBase + "unsupport";
        TableDescriptor.Builder option = Commlib.createBaseTableOption( schema,
                csName, clName );
        TableDescriptor tableDescriptor = option.option( "test", "test" )
                .build();
        tableEnv.createTable( tableName, tableDescriptor );
        try {
            tableEnv.executeSql( String.format( randomSql, tableName ) );
            Assert.fail( "except fail but success" );
        } catch ( ValidationException e ) {
            String message = e.getCause().getMessage();
            if ( !( message.contains(
                    "Unsupported options found for 'sequoiadb'" ) ) ) {
                throw e;
            }
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
        bson.put( filed_String, data_String );
        bson.put( filed_int, data_int );
        cl.insert( bson );
    }

    private static TableDescriptor.Builder createSingleOption( Schema schema ) {
        return TableDescriptor.forConnector( SDBAttribute.sequoiadb )
                .schema( schema );
    }
}
