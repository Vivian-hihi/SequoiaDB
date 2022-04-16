package com.sequoiadb.ddl;

import com.sequoiadb.testcommon.FlinkTestBase;
import com.sequoiadb.testcommon.utils.Commlib;
import org.apache.flink.streaming.api.environment.StreamExecutionEnvironment;
import org.apache.flink.table.api.*;
import org.apache.flink.table.api.bridge.java.StreamTableEnvironment;
import org.apache.flink.types.Row;
import org.apache.flink.util.CloseableIterator;
import org.testng.Assert;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import java.util.ArrayList;
import java.util.List;

/**
 * @descreption seqDB-25310:修改SDB映射表名
 * @author YiPan
 * @date 2022/2/11
 * @updateUser
 * @updateDate
 * @updateRemark
 * @version 1.0
 */
public class CreateTable25310 extends FlinkTestBase {
    private StreamTableEnvironment tableEnv;
    private final String csName = "cs_25310";
    private final String clName = "cl_25310";
    private final String filed_A = "test_a";
    private final String filed_B = "test_b";
    private final String tableName = "tb_25310";
    private final String newTableName = "tb_25310_new";
    private TableDescriptor tableDescriptor;

    @BeforeClass
    public void setUp() {
        StreamExecutionEnvironment env = StreamExecutionEnvironment
                .getExecutionEnvironment();
        tableEnv = StreamTableEnvironment.create( env );
        Schema schema = Schema.newBuilder()
                .column( filed_A, DataTypes.STRING() )
                .column( filed_B, DataTypes.INT() ).build();
        tableDescriptor = Commlib.createTableDescriptor( schema, csName,
                clName );
    }

    @Test
    public void test() {
        try {
            tableEnv.executeSql(
                    "alter table " + tableName + " rename to " + newTableName );
            Assert.fail( "except fail but success" );
        } catch ( ValidationException e ) {
            String message = e.getMessage();
            if ( !( message.contains( "doesn't exist" ) ) ) {
                throw e;
            }
        }
        tableEnv.createTable( tableName, tableDescriptor );
        tableEnv.executeSql(
                "alter table " + tableName + " rename to " + newTableName );
        List< String > allTableNames = collectTableNames();
        Assert.assertFalse( allTableNames.contains( tableName ) );
        Assert.assertTrue( allTableNames.contains( newTableName ) );

    }

    @AfterClass
    public void tearDown() {
    }

    private List< String > collectTableNames() {
        List< String > tableNames = new ArrayList<>();
        TableResult show_tables = tableEnv.executeSql( "show tables" );
        CloseableIterator< Row > collect = show_tables.collect();
        while ( collect.hasNext() ) {
            Row row = collect.next();
            tableNames.add( ( String ) row.getField( 0 ) );
        }
        return tableNames;
    }
}
