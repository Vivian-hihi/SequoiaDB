package com.sequoiadb.ddl;

import com.sequoiadb.base.Sequoiadb;
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
 * @descreption seqDB-25312:修改SDB映射表属性，修改策略参数
 * @author YiPan
 * @date 2022/2/28
 * @updateUser
 * @updateDate
 * @updateRemark
 * @version 1.0
 */
public class CreateTable25312 extends FlinkTestBase {
    private StreamTableEnvironment tableEnv;
    private final String csName = "cs_25312";
    private final String clName = "cl_25312";
    private final String filed_A = "test_a";
    private final String filed_B = "test_b";
    private final String tableName = "tb_25312";
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
        TableDescriptor.Builder option = Commlib.createBaseTableOption( schema,
                csName, clName );
        TableDescriptor tableDescriptor = option
                .option( SDBAttribute.splitmode, "auto" ).build();
        tableEnv.createTable( tableName, tableDescriptor );
    }

    @Test
    public void test() {
        tableEnv.executeSql(
                "insert into " + tableName + " values('abc',123)" );

        tableEnv.executeSql( "alter table " + tableName + " set ('"
                + SDBAttribute.splitmode + "'='null')" );
        try {
            tableEnv.executeSql( "select * from " + tableName );
            Assert.fail( "except fail but success" );
        } catch ( ValidationException e ) {
            String cause = e.getCause().getMessage();
            if ( !( cause.contains( "unknown split mode: null" ) ) ) {
                throw e;
            }
        }

        tableEnv.executeSql( "alter table " + tableName + " set ('"
                + SDBAttribute.splitmode + "'='auto')" );
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
