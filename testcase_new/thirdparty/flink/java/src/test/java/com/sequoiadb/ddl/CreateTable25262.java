package com.sequoiadb.ddl;

import com.sequoiadb.testcommon.FlinkTestBase;
import com.sequoiadb.testcommon.utils.Commlib;
import org.apache.flink.streaming.api.environment.StreamExecutionEnvironment;
import org.apache.flink.table.api.DataTypes;
import org.apache.flink.table.api.Schema;
import org.apache.flink.table.api.TableDescriptor;
import org.apache.flink.table.api.bridge.java.StreamTableEnvironment;
import org.testng.Assert;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

/**
 * @descreption seqDB-25262:创建SDB映射表，指定表名
 * @author YiPan
 * @date 2022/1/28
 * @updateUser
 * @updateDate
 * @updateRemark
 * @version 1.0
 */
public class CreateTable25262 extends FlinkTestBase {
    private StreamTableEnvironment tableEnv;
    private String csName = "cs_25262";
    private String clName = "cl_25262";
    private String tableName = "tb_25262";

    @BeforeClass
    public void setUp() {
        StreamExecutionEnvironment env = StreamExecutionEnvironment
                .getExecutionEnvironment();
        tableEnv = StreamTableEnvironment.create( env );
    }

    @Test
    public void test() {
        // 创建表a
        Schema schema_a = Schema.newBuilder().column( "a", "int" )
                .column( "b", "varchar(50)" ).build();
        TableDescriptor tableDescriptor_a = Commlib
                .createTableDescriptor( schema_a, csName, clName );
        tableEnv.createTable( tableName, tableDescriptor_a );

        // 创建同名表b
        Schema schema_b = Schema.newBuilder()
                .column( "a", DataTypes.VARBINARY( 50 ) ).build();
        TableDescriptor tableDescriptor_b = Commlib
                .createTableDescriptor( schema_b, csName, clName );
        // 预期失败
        try {
            tableEnv.createTable( tableName, tableDescriptor_b );
            Assert.fail( "except fail but success" );
        } catch ( Exception e ) {
            String cause = e.getCause().toString();
            if ( !( cause.contains( tableName + " already exists" ) ) ) {
                throw e;
            }
        }

    }

    @AfterClass
    public void tearDown() {
    }

}