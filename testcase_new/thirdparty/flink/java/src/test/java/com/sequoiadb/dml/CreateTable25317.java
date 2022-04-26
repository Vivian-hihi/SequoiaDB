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
 * @descreption seqDB-25317:使用INTERSECT关键字，查询映射表
 * @author YiPan
 * @date 2022/2/29
 * @updateUser
 * @updateDate
 * @updateRemark
 * @version 1.0
 */
public class CreateTable25317 extends FlinkTestBase {
    private StreamTableEnvWarpper tableEnvWarpper;
    private final String csName = "cs_25317";
    private final String clNameA = "cl_25317_A";
    private final String clNameB = "cl_25317_B";
    private final String tableNameA = "tb_25317_A";
    private final String tableNameB = "tb_25317_B";
    private Sequoiadb sdb;

    @BeforeClass
    public void setUp() throws SQLException, ClassNotFoundException {
        tableEnvWarpper = StreamTableEnvWarpper.create();
        sdb = new Sequoiadb( FlinkTestBase.getCoord(), FlinkTestBase.username,
                FlinkTestBase.password );
        Commlib.dropCS( sdb, csName );
        Schema schema = Schema.newBuilder()
                .column( "name", DataTypes.VARCHAR( 10 ).notNull() )
                .column( "age", DataTypes.INT() ).primaryKey( "name" ).build();
        tableEnvWarpper.createTable( tableNameA, schema, csName, clNameA );
        tableEnvWarpper.createTable( tableNameB, schema, csName, clNameB );
    }

    @Test
    public void test() throws Exception {
        insertData();
        tableEnvWarpper.assertTableDataNoOrderWithSql( "select * from "
                + tableNameA + " except select * from " + tableNameB );
        tableEnvWarpper.assertTableDataNoOrderWithSql( "select * from "
                + tableNameA + " intersect select * from " + tableNameB );
    }

    private void insertData() throws Exception {
        String tableAsql = "insert into " + tableNameA + " values('Happy',30)";
        String tableBsql = "insert into " + tableNameB + " values('ABC',25)";
        String sameSql = "insert into %s values('same',100)";
        tableEnvWarpper.executeSql( tableAsql );
        tableEnvWarpper.executeSql( tableBsql );
        tableEnvWarpper.executeSql( String.format( sameSql, tableNameA ) );
        tableEnvWarpper.executeSql( String.format( sameSql, tableNameB ) );
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
