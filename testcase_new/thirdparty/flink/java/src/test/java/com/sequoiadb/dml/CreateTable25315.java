package com.sequoiadb.dml;

import com.sequoiadb.base.CollectionSpace;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.testcommon.FlinkTestBase;
import com.sequoiadb.testcommon.utils.Commlib;
import com.sequoiadb.testcommon.warpper.StreamTableEnvWarpper;
import org.apache.flink.streaming.api.environment.StreamExecutionEnvironment;
import org.apache.flink.table.api.*;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import java.sql.SQLException;

/**
 * @descreption seqDB-25315:使用UNION关键字，查询映射表
 * @author YiPan
 * @date 2022/2/28
 * @updateUser
 * @updateDate
 * @updateRemark
 * @version 1.0
 */
public class CreateTable25315 extends FlinkTestBase {
    private StreamTableEnvWarpper tableEnvWarpper;
    private final String csName = "cs_25315";
    private final String clNameA = "cl_25315_A";
    private final String clNameB = "cl_25315_B";
    private final String tableNameA = "tb_25315_A";
    private final String tableNameB = "tb_25315_B";
    private Sequoiadb sdb;

    @BeforeClass
    public void setUp() throws SQLException, ClassNotFoundException {
        StreamExecutionEnvironment env = StreamExecutionEnvironment
                .getExecutionEnvironment();
        tableEnvWarpper = StreamTableEnvWarpper.create();
        sdb = new Sequoiadb( FlinkTestBase.getCoord(), FlinkTestBase.username,
                FlinkTestBase.password );
        Commlib.dropCS( sdb, csName );
        CollectionSpace cs = sdb.createCollectionSpace( csName );
        cs.createCollection( clNameA );
        cs.createCollection( clNameB );
        Schema schema = Schema.newBuilder()
                .column( "name", DataTypes.VARCHAR( 10 ) )
                .column( "age", DataTypes.INT() ).build();
        tableEnvWarpper.createTable( tableNameA, schema, csName, clNameA );
        tableEnvWarpper.createTable( tableNameB, schema, csName, clNameB );
    }

    @Test
    public void test() throws Exception {
        insertData();

        tableEnvWarpper.assertTableDataNoOrderWithSql( "select * from " + tableNameA
                + " union select * from " + tableNameB );
    }

    private void insertData() throws Exception {
        String tableAsql = "insert into " + tableNameA + " values('Happy',30)";
        String tableBsql = "insert into " + tableNameB + " values('ABC',25)";
        tableEnvWarpper.executeSql( tableAsql );
        tableEnvWarpper.executeSql( tableBsql );
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
