package com.sequoiadb.dml;

import com.sequoiadb.base.CollectionSpace;
import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.testcommon.FlinkTestBase;
import com.sequoiadb.testcommon.utils.Commlib;
import org.apache.commons.lang3.RandomStringUtils;
import org.apache.flink.streaming.api.environment.StreamExecutionEnvironment;
import org.apache.flink.table.api.*;
import org.apache.flink.table.api.bridge.java.StreamTableEnvironment;
import org.apache.flink.types.Row;
import org.bson.BasicBSONObject;
import org.testng.Assert;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import java.sql.SQLException;
import java.util.ArrayList;

/**
 * @descreption seqDB-25326:使用INSERT INTO SELECT创建表
 * @author YiPan
 * @date 2022/3/2
 * @updateUser
 * @updateDate
 * @updateRemark
 * @version 1.0
 */
public class CreateTable25326 extends FlinkTestBase {
    private StreamTableEnvironment tableEnv;
    private final String tableNameA = "tb_25326_A";
    private final String tableNameB = "tb_25326_B";
    private final String csName = "cs_25326";
    private final String clName_A = "cl_25326_A";
    private final String clName_B = "cl_25326_B";
    private Sequoiadb sdb;
    private CollectionSpace cs;
    private Schema schema;

    @BeforeClass
    public void setUp() throws SQLException, ClassNotFoundException {
        StreamExecutionEnvironment env = StreamExecutionEnvironment
                .getExecutionEnvironment();
        tableEnv = StreamTableEnvironment.create( env );
        sdb = new Sequoiadb( FlinkTestBase.getCoord(), FlinkTestBase.username,
                FlinkTestBase.password );
        if ( sdb.isCollectionSpaceExist( csName ) ) {
            sdb.dropCollectionSpace( csName );
        }
        cs = sdb.createCollectionSpace( csName );
        schema = Schema.newBuilder()
                .column( "name", DataTypes.VARCHAR( 10 ).notNull() )
                .column( "age", DataTypes.INT() ).primaryKey( "name" ).build();
        TableDescriptor table_A = Commlib.createTableDescriptor( schema, csName,
                clName_A );
        tableEnv.createTable( tableNameA, table_A );
        TableDescriptor table_B = Commlib.createTableDescriptor( schema, csName,
                clName_B );
        tableEnv.createTable( tableNameB, table_B );
    }

    @Test
    public void test() throws Exception {
        // 表A插入数据
        insertData();

        // 表B复制表A
        TableResult copy_A = tableEnv.executeSql(
                "insert into " + tableNameB + " select * from " + tableNameA );
        Commlib.waitJobFinish( copy_A );

        // 验证表B集合自动生成
        Assert.assertTrue( cs.isCollectionExist( clName_B ) );

        // 验证表数据相同
        ArrayList< Row > result_A = Commlib.collectToArrayList( tableEnv
                .executeSql( "select * from " + tableNameA ).collect() );
        ArrayList< Row > result_B = Commlib.collectToArrayList( tableEnv
                .executeSql( "select * from " + tableNameB ).collect() );
        Assert.assertEqualsNoOrder( result_B.toArray(), result_A.toArray() );
    }

    private void insertData() {
        DBCollection cl = cs.createCollection( clName_A );
        for ( int i = 0; i < 50; i++ ) {
            BasicBSONObject record = new BasicBSONObject();
            record.put( "name", RandomStringUtils.randomAlphanumeric( 5 ) );
            record.put( "age", i );
            cl.insertRecord( record );
        }
    }

    @AfterClass
    public void tearDown() {
        try {
            Commlib.dropCS( sdb, csName );
        } finally {
            sdb.close();
        }
    }
}
