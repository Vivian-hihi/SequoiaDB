package com.sequoiadb.concurrent;

import com.sequoiadb.base.CollectionSpace;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.threadexecutor.ThreadExecutor;
import com.sequoiadb.threadexecutor.annotation.ExecuteOrder;
import com.sequoiadb.testcommon.FlinkTestBase;
import com.sequoiadb.testcommon.utils.Commlib;
import org.apache.flink.core.execution.JobClient;
import org.apache.flink.streaming.api.environment.StreamExecutionEnvironment;
import org.apache.flink.table.api.DataTypes;
import org.apache.flink.table.api.Schema;
import org.apache.flink.table.api.TableDescriptor;
import org.apache.flink.table.api.TableResult;
import org.apache.flink.table.api.bridge.java.StreamTableEnvironment;
import org.apache.flink.types.Row;
import org.apache.flink.util.CloseableIterator;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

/**
 * @descreption seqDB-25334:并发插入和查询数据
 * @author YiPan
 * @date 2022/2/18
 * @updateUser
 * @updateDate
 * @updateRemark
 * @version 1.0
 */
public class CreateTable25334 extends FlinkTestBase {
    private Sequoiadb sdb;
    private final String csName = "cs_25334";
    private final String clName = "cl_25334";
    private final String filed_int = "test_int";
    private final String filed_String = "test_string";
    private final String tableName = "tb_25334";
    private TableDescriptor tableDescriptor;
    private JobClient jobClient;

    @BeforeClass
    public void setUp() {
        sdb = new Sequoiadb( FlinkTestBase.getCoord(), FlinkTestBase.username,
                FlinkTestBase.password );
        Schema schema = Schema.newBuilder().column( filed_int, DataTypes.INT() )
                .column( filed_String, DataTypes.STRING() ).build();
        tableDescriptor = Commlib.createTableDescriptor( schema, csName,
                clName );
        Commlib.dropCS( sdb, csName );
        CollectionSpace cs = sdb.createCollectionSpace( csName );
        cs.createCollection( clName );
    }

    @Test
    public void test() throws Exception {
        ThreadExecutor t = new ThreadExecutor();
        t.addWorker( new Select() );
        t.addWorker( new Insert() );
        t.run();
    }

    @AfterClass
    public void tearDown() {
        try {
            jobClient.cancel();
            sdb.dropCollectionSpace( csName );
        } finally {
            sdb.close();
        }
    }

    private class Insert {
        @ExecuteOrder(step = 1)
        private void run() {
            StreamExecutionEnvironment env = StreamExecutionEnvironment
                    .getExecutionEnvironment();
            StreamTableEnvironment tableEnv = StreamTableEnvironment.create( env );
            tableEnv.createTable( tableName, tableDescriptor );
            // 创建datagen表构建流式数据
            tableEnv.executeSql( "CREATE TABLE datagen (\n"
                    + " f_sequence INT, f_random_str STRING) WITH (\n"
                    + " 'connector' = 'datagen',\n"
                    + " 'rows-per-second'='10',\n"
                    + " 'fields.f_sequence.kind'='sequence',\n"
                    + " 'fields.f_sequence.start'='1',\n"
                    + " 'fields.f_sequence.end'='1000',\n"
                    + " 'fields.f_random_str.length'='10')" );
            TableResult tableResult = tableEnv.executeSql(
                    "insert into " + tableName + " select * from datagen" );
            jobClient = tableResult.getJobClient().get();
        }
    }

    private class Select {
        @ExecuteOrder(step = 1)
        private void run() throws Exception {
            StreamExecutionEnvironment env = StreamExecutionEnvironment
                    .getExecutionEnvironment();
            StreamTableEnvironment tableEnv = StreamTableEnvironment.create( env );
            tableEnv.createTable( tableName, tableDescriptor );
            int times = 0;
            while ( true ) {
                TableResult tableResult = tableEnv
                        .executeSql( "select * from " + tableName );
                CloseableIterator< Row > collect = tableResult.collect();
                if ( collect.hasNext() ) {
                    break;
                }
                Thread.sleep( 1000 );
                times++;
                if ( times > 120 ) {
                    throw new Exception( "select time out " );
                }
            }
        }
    }
}
