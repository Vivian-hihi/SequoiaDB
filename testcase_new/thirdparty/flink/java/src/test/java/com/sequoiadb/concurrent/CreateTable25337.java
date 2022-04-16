package com.sequoiadb.concurrent;

import com.sequoiadb.base.CollectionSpace;
import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.threadexecutor.ThreadExecutor;
import com.sequoiadb.threadexecutor.annotation.ExecuteOrder;
import com.sequoiadb.testcommon.FlinkTestBase;
import com.sequoiadb.testcommon.utils.Commlib;
import org.apache.flink.streaming.api.environment.StreamExecutionEnvironment;
import org.apache.flink.table.api.*;
import org.apache.flink.table.api.bridge.java.StreamTableEnvironment;
import org.testng.Assert;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

/**
 * @descreption seqDB-25337:flink插入数据时，SDB删除集合
 * @author YiPan
 * @date 2022/2/18
 * @updateUser
 * @updateDate
 * @updateRemark
 * @version 1.0
 */
public class CreateTable25337 extends FlinkTestBase {
    private Sequoiadb sdb;
    private final String csName = "cs_25337";
    private final String clName = "cl_25337";
    private final String filed_int = "test_int";
    private final String filed_String = "test_string";
    private final String tableName = "tb_25337";
    private final int recordNum = 5000;
    private DBCollection cl;

    @BeforeClass
    public void setUp() {
        sdb = new Sequoiadb( FlinkTestBase.getCoord(), FlinkTestBase.username,
                FlinkTestBase.password );
        Commlib.dropCS( sdb, csName );
        CollectionSpace cs = sdb.createCollectionSpace( csName );
        cl = cs.createCollection( clName );
    }

    @Test
    public void test() throws Exception {
        // 并发插入
        ThreadExecutor t = new ThreadExecutor( 180000 );
        t.addWorker( new InsertData() );
        t.addWorker( new DropCl() );
        t.run();

        long actRecordNum = cl.getCount();
        // 排除未撞到并发场景后再校验
        if ( actRecordNum != 0 ) {
            Assert.assertEquals( actRecordNum, recordNum );
        }
    }

    @AfterClass
    public void tearDown() {
        try {
            sdb.dropCollectionSpace( csName );
        } finally {
            sdb.close();
        }
    }

    private class InsertData {
        @ExecuteOrder(step = 1)
        private void run() throws Exception {
            StreamExecutionEnvironment env = StreamExecutionEnvironment
                    .getExecutionEnvironment();
            StreamTableEnvironment tableEnv = StreamTableEnvironment.create( env );
            Schema schema = Schema.newBuilder()
                    .column( filed_int, DataTypes.INT() )
                    .column( filed_String, DataTypes.STRING() ).build();
            TableDescriptor tableDescriptor = Commlib
                    .createTableDescriptor( schema, csName, clName );
            tableEnv.createTable( tableName, tableDescriptor );
            // 创建datagen表
            tableEnv.executeSql( "CREATE TABLE datagen (\n" + " f_sequence INT,\n"
                    + " f_random_str STRING\n" + ") WITH (\n"
                    + " 'connector' = 'datagen',\n"
                    + " 'rows-per-second'='100',\n"
                    + " 'fields.f_sequence.kind'='sequence',\n"
                    + " 'fields.f_sequence.start'='1',\n"
                    + " 'fields.f_sequence.end'='" + recordNum + "',\n"
                    + " 'fields.f_random_str.length'='10'\n" + ")" );
            // 持续写入流式数据
            TableResult tableResult = tableEnv.executeSql(
                    "insert into " + tableName + " select * from datagen" );
            // 等待线程执行完
            Commlib.waitJobFinish( tableResult, 120000 );
        }
    }

    private class DropCl {
        @ExecuteOrder(step = 1)
        private void run() throws Exception {
            CollectionSpace cs = sdb.getCollectionSpace( csName );
            DBCollection cl = cs.getCollection( clName );
            int times = 0;
            // 写入一部分数据后执行删除
            while ( cl.getCount() <= 0 ) {
                Thread.sleep( 1000 );
                times++;
                if ( times > 120 ) {
                    throw new Exception( "wait insert data time out" );
                }
            }
            cs.dropCollection( clName );
        }
    }
}
