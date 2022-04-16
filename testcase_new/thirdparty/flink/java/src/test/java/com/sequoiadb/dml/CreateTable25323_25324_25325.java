package com.sequoiadb.dml;

import com.sequoiadb.base.CollectionSpace;
import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.testcommon.FlinkTestBase;
import com.sequoiadb.testcommon.utils.Commlib;
import com.sequoiadb.testcommon.warpper.StreamTableEnvWarpper;
import com.sequoiadb.testcommon.warpper.TableResultWarpper;
import org.apache.commons.lang3.RandomStringUtils;
import org.apache.flink.table.api.*;
import org.apache.flink.table.api.bridge.java.StreamTableEnvironment;
import org.bson.BasicBSONObject;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import java.sql.SQLException;
import java.util.Random;

/**
 * @descreption seqDB-25323:使用GROUP BY对查询结果分组 seqDB-25324:使用WHERE关键字指定运算符
 *              seqDB-25325:使用HAVING关键字查询
 * @author YiPan
 * @date 2022/3/1
 * @updateUser
 * @updateDate
 * @updateRemark
 * @version 1.0
 */
public class CreateTable25323_25324_25325 extends FlinkTestBase {
    private StreamTableEnvWarpper tableEnvWarpper;
    private final String csName = "cs_25323";
    private final String clName = "cl_25323";
    private final String dataClName = "cl_25323_data";
    private final String tableName = "tb_25323";
    private final String dataTableName = "tb_25323_data";
    private Sequoiadb sdb;
    private Schema schema;
    private DBCollection datacl;

    @BeforeClass
    public void setUp() throws SQLException, ClassNotFoundException {
        tableEnvWarpper = StreamTableEnvWarpper.create();
        sdb = new Sequoiadb( FlinkTestBase.getCoord(), FlinkTestBase.username,
                FlinkTestBase.password );
        Commlib.dropCS( sdb, csName );
        CollectionSpace cs = sdb.createCollectionSpace( csName );
        cs.createCollection( clName );
        datacl = cs.createCollection( dataClName );
        schema = Schema.newBuilder().column( "name", DataTypes.VARCHAR( 10 ) )
                .column( "age", DataTypes.INT() )
                .column( "clazz", DataTypes.VARCHAR( 50 ) )
                .column( "sex", DataTypes.VARCHAR( 50 ) ).build();
        tableEnvWarpper.createTable( tableName, schema, csName, clName );
    }

    @Test
    public void test() throws Exception {
        insertData();
        // group by [column]
        tableEnvWarpper.assertTableDataNoOrderWithSql(
                "select clazz,count(clazz) from " + tableName
                        + " group by clazz" );
        // group by rollup
        tableEnvWarpper.assertTableDataNoOrderWithSql( "select clazz,sex,age from "
                + tableName + " group by rollup( clazz,sex,age )" );
        // group by cube
        tableEnvWarpper.assertTableDataNoOrderWithSql( "select clazz,sex,age from "
                + tableName + " group by cube( clazz,sex,age )" );
        // group by grouping sets
        tableEnvWarpper.assertTableDataNoOrderWithSql(
                "select clazz,sex,age from " + tableName
                        + " group by grouping sets( (clazz,sex),(sex,age) )" );

        // where age = 22
        tableEnvWarpper.assertTableDataNoOrderWithSql(
                "select * from " + tableName + " where age = 22" );
        // where age > 22
        tableEnvWarpper.assertTableDataNoOrderWithSql(
                "select * from " + tableName + " where age > 22" );
        // where age < 22
        tableEnvWarpper.assertTableDataNoOrderWithSql(
                "select * from " + tableName + " where age < 22" );
        // where age <= 22
        tableEnvWarpper.assertTableDataNoOrderWithSql(
                "select * from " + tableName + " where age <= 22" );
        // where age >= 22
        tableEnvWarpper.assertTableDataNoOrderWithSql(
                "select * from " + tableName + " where age >= 22" );
        // where age between 21 and 25
        tableEnvWarpper.assertTableDataNoOrderWithSql(
                "select * from " + tableName + " where age between 21 and 25" );

        // group by age having sum( age )>22
        tableEnvWarpper
                .assertTableDataNoOrderWithSql( "select age,count(age) from "
                        + tableName + " group by age having sum( age )>22" );
    }

    private void insertData() throws Exception {
        // 用于随机组合成记录的数据
        int[] ages = { 20, 21, 22, 23, 24, 25 };
        String[] clazzs = { "one", "two", "three", "four" };
        String[] sexs = { "man", "woman" };
        Random random = new Random();

        // 构造一个存有数据的集合用于插入数据
        for ( int i = 0; i < 50; i++ ) {
            BasicBSONObject bson = new BasicBSONObject();
            bson.put( "name", RandomStringUtils.randomAlphanumeric( 10 ) );
            bson.put( "age", ages[ random.nextInt( ages.length ) ] );
            bson.put( "clazz", clazzs[ random.nextInt( clazzs.length ) ] );
            bson.put( "sex", sexs[ random.nextInt( sexs.length ) ] );
            datacl.insert( bson );
        }
        // 在公共库default_database下创建数据表，映射sdb数据集合
        StreamTableEnvironment tableEnv = tableEnvWarpper.getStreamTableEnvironment();
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
        } finally {
            sdb.close();
        }
    }
}
