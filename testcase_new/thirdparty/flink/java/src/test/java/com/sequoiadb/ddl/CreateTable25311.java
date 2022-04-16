package com.sequoiadb.ddl;

import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.exception.SDBError;
import com.sequoiadb.testcommon.FlinkTestBase;
import com.sequoiadb.testcommon.utils.Commlib;
import com.sequoiadb.testcommon.utils.SDBAttribute;
import org.apache.flink.streaming.api.environment.StreamExecutionEnvironment;
import org.apache.flink.table.api.*;
import org.apache.flink.table.api.bridge.java.StreamTableEnvironment;
import org.apache.flink.types.Row;
import org.testng.Assert;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import java.util.ArrayList;

/**
 * @descreption seqDB-25311:修改SDB映射表属性，修改连接参数
 * @author YiPan
 * @date 2022/2/28
 * @updateUser
 * @updateDate
 * @updateRemark
 * @version 1.0
 */
public class CreateTable25311 extends FlinkTestBase {
    private StreamTableEnvironment tableEnv;
    private final String csName = "cs_25311";
    private final String clName = "cl_25311";
    private final String filed_A = "test_a";
    private final String filed_B = "test_b";
    private final String data_A = "abc";
    private final int data_B = 123456;
    private final String tableName = "tb_25311";
    private Sequoiadb sdb;

    @BeforeClass
    public void setUp() {
        StreamExecutionEnvironment env = StreamExecutionEnvironment
                .getExecutionEnvironment();
        tableEnv = StreamTableEnvironment.create( env );
        sdb = new Sequoiadb( FlinkTestBase.getCoord(), FlinkTestBase.username,
                FlinkTestBase.password );
        Commlib.dropCS( sdb, csName );
        Schema schema = Schema.newBuilder()
                .column( filed_A, DataTypes.STRING() )
                .column( filed_B, DataTypes.INT() ).build();
        TableDescriptor tableDescriptor = Commlib.createTableDescriptor( schema,
                csName, clName );
        tableEnv.createTable( tableName, tableDescriptor );
    }

    @Test
    public void test() {
        tableEnv.executeSql( "alter table " + tableName + " set ('"
                + SDBAttribute.hosts + "'='wronghosts')" );
        try {
            tableEnv.executeSql(
                    "insert into " + tableName + " values('abc',123)" );
            Assert.fail( "except fail but success" );
        } catch ( ValidationException e ) {
            String cause = e.getCause().getMessage();
            if ( !( cause.contains(
                    SDBError.SDB_NET_CANNOT_CONNECT.toString() ) ) ) {
                throw e;
            }
        }

        tableEnv.executeSql( "alter table " + tableName + " set ('"
                + SDBAttribute.hosts + "'='" + FlinkTestBase.getCoords() + "')" );
        tableEnv.executeSql( "insert into " + tableName + " values('" + data_A
                + "'," + data_B + ")" );
        ArrayList<Row> rows = Commlib.collectToArrayList( tableEnv
                .executeSql( "select * from " + tableName ).collect() );
        Assert.assertEquals( rows.size(), 1 );
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
