package com.sequoiadb.dml;

import com.sequoiadb.base.CollectionSpace;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.testcommon.FlinkTestBase;
import com.sequoiadb.testcommon.utils.Commlib;
import com.sequoiadb.testcommon.warpper.StreamTableEnvWarpper;
import org.apache.flink.table.api.*;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import java.sql.SQLException;

/**
 * @descreption seqDB-25321:使用JOIN字段连接表查询 seqDB-25322:使用AS关键字查询
 * @author YiPan
 * @date 2022/3/2
 * @updateUser
 * @updateDate
 * @updateRemark
 * @version 1.0
 */
public class CreateTable25321_25322 extends FlinkTestBase {
    private StreamTableEnvWarpper tableEnvWarpper;
    private final String csName = "cs_25321";
    private final String clNameA = "cl_25321_A";
    private final String clNameB = "cl_25321_B";
    private final String tableNameA = "tb_25321_A";
    private final String tableNameB = "tb_25321_B";
    private Sequoiadb sdb;

    @BeforeClass
    public void setUp() throws SQLException, ClassNotFoundException {
        tableEnvWarpper = StreamTableEnvWarpper.create();
        sdb = new Sequoiadb( FlinkTestBase.getCoord(), FlinkTestBase.username,
                FlinkTestBase.password );
        Commlib.dropCS( sdb, csName );
        Schema schemaA = Schema.newBuilder()
                .column( "id", DataTypes.INT().notNull() )
                .column( "name", DataTypes.VARCHAR( 10 ) )
                .column( "age", DataTypes.INT() ).primaryKey( "id" ).build();
        Schema schemaB = Schema.newBuilder()
                .column( "id", DataTypes.INT().notNull() )
                .column( "Math", DataTypes.INT() )
                .column( "English", DataTypes.INT() )
                .column( "Language", DataTypes.INT() ).primaryKey( "id" )
                .build();

        tableEnvWarpper.createTable( tableNameA, schemaA, csName, clNameA );
        tableEnvWarpper.createTable( tableNameB, schemaB, csName, clNameB );
    }

    @Test
    public void test() throws Exception {
        insertData();
        tableEnvWarpper.assertTableDataNoOrderWithSql(
                "select * from " + tableNameA + " as A left join " + tableNameB
                        + " as B on A.id = B.id" );
        tableEnvWarpper.assertTableDataNoOrderWithSql(
                "select * from " + tableNameA + " as A right join " + tableNameB
                        + " as B on A.id = B.id" );
        tableEnvWarpper.assertTableDataNoOrderWithSql(
                "select * from " + tableNameA + " as A full join " + tableNameB
                        + " as B on A.id = B.id" );
    }

    @AfterClass
    public void tearDown() throws SQLException, ClassNotFoundException {
        try {
            tableEnvWarpper.clearSourceDatabase( tableNameA );
            tableEnvWarpper.clearSourceDatabase( tableNameB );
            Commlib.dropCS( sdb, csName );
        } finally {
            sdb.close();
        }
    }

    private void insertData() throws Exception {
        tableEnvWarpper
                .executeSql(
                        "insert into " + tableNameA + " values(1,'Happy',30)" )
                .waitJobFinish();
        tableEnvWarpper
                .executeSql(
                        "insert into " + tableNameA + " values(2,'Bob',20)" )
                .waitJobFinish();
        tableEnvWarpper
                .executeSql(
                        "insert into " + tableNameB + " values(1,90,90,100)" )
                .waitJobFinish();
        tableEnvWarpper
                .executeSql(
                        "insert into " + tableNameB + " values(3,80,80,90)" )
                .waitJobFinish();
    }
}
