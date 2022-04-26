package com.sequoiadb.dml;

import com.sequoiadb.base.CollectionSpace;
import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.testcommon.FlinkTestBase;
import com.sequoiadb.testcommon.utils.Commlib;
import com.sequoiadb.testcommon.warpper.StreamTableEnvWarpper;
import com.sequoiadb.testcommon.warpper.TableResultWarpper;
import org.apache.commons.lang3.RandomStringUtils;
import org.apache.flink.api.common.RuntimeExecutionMode;
import org.apache.flink.table.api.*;
import org.apache.flink.table.api.bridge.java.StreamTableEnvironment;
import org.bson.BasicBSONObject;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import java.sql.SQLException;
import java.sql.Timestamp;

/**
 * @descreption seqDB-25318:使用ORDER BY给查询结果排序 seqDB-25319:使用LIMIT、FETCH限制返回结果
 *              seqDB-25320:使用OFFSET偏移读
 * @author YiPan
 * @date 2022/3/1
 * @updateUser
 * @updateDate
 * @updateRemark
 * @version 1.0
 */
public class CreateTable25318_25319_25320 extends FlinkTestBase {
    private StreamTableEnvWarpper tableEnvWarpper;
    private final String csName = "cs_25318";
    private final String clName = "cl_25318";
    private final String dataClName = "cl_25318_data";
    private final String tableName = "tb_25318";
    private final String dataTableName = "tb_25318_data";
    private Sequoiadb sdb;
    private Schema schema;
    private DBCollection datacl;

    @BeforeClass
    public void setUp() throws SQLException, ClassNotFoundException {
        tableEnvWarpper = StreamTableEnvWarpper
                .create( RuntimeExecutionMode.BATCH );
        sdb = new Sequoiadb( FlinkTestBase.getCoord(), FlinkTestBase.username,
                FlinkTestBase.password );
        Commlib.dropCS( sdb, csName );
        CollectionSpace cs = sdb.createCollectionSpace( csName );
        cs.createCollection( clName );
        datacl = cs.createCollection( dataClName );
        schema = Schema.newBuilder().column( "name", DataTypes.VARCHAR( 10 ) )
                .column( "age", DataTypes.INT().notNull() )
                .column( "ts", DataTypes.TIMESTAMP( 3 ) ).primaryKey( "age" )
                .build();
        tableEnvWarpper.createTable( tableName, schema, csName, clName );
    }

    @Test
    public void test() throws Exception {
        insertData();
        // 根据age降序，limit = 20
        tableEnvWarpper.assertTableDataWithSql(
                "select * from " + tableName + " order by age desc limit 20 " );
        // 根据age升序，limit = 50
        tableEnvWarpper.assertTableDataWithSql(
                "select * from " + tableName + " order by age asc limit 50 " );
        // 根name降序，offset = 10, fetch = 5
        tableEnvWarpper.assertTableDataWithSql( "select * from " + tableName
                + " order by name desc offset 10 rows fetch next 5 rows only" );
        // 根name升序，offset = 20, fetch = 10
        tableEnvWarpper.assertTableDataWithSql( "select * from " + tableName
                + " order by name asc offset 20 rows fetch next 10 rows only" );
    }

    private void insertData() throws Exception {
        // 构造一个存有数据的集合用于插入数据
        for ( int i = 0; i < 50; i++ ) {
            BasicBSONObject bson = new BasicBSONObject();
            bson.put( "name", RandomStringUtils.randomAlphanumeric( 10 ) );
            bson.put( "age", i );
            bson.put( "ts", new Timestamp( System.currentTimeMillis() ) );
            datacl.insertRecord( bson );
        }
        // 在公共库default_database下创建数据表，映射sdb数据集合
        StreamTableEnvironment tableEnv = tableEnvWarpper
                .getStreamTableEnvironment();
        TableDescriptor tableDescriptor = Commlib.createTableDescriptor( schema,
                csName, dataClName );
        tableEnv.useDatabase( "default_database" );
        tableEnv.createTable( dataTableName, tableDescriptor );
        // 插入数据到测试的sdb表和jdbc表
        TableResultWarpper tableResultWarpper = tableEnvWarpper
                .executeSql( "insert into " + tableName
                        + " select * from default_catalog.default_database."
                        + dataTableName );
        tableResultWarpper.waitJobFinish();
    }

    @AfterClass
    public void tearDown() throws SQLException, ClassNotFoundException {
        try {
            tableEnvWarpper.clearSourceDatabase( csName );
            Commlib.dropCS( sdb, csName );
        } finally {
            sdb.close();
        }
    }
}
