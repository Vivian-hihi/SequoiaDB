package com.sequoiadb.ddl;

import com.sequoiadb.base.CollectionSpace;
import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.testcommon.FlinkTestBase;
import com.sequoiadb.testcommon.utils.Commlib;
import com.sequoiadb.testcommon.utils.SDBAttribute;
import org.apache.flink.api.common.JobStatus;
import org.apache.flink.core.execution.JobClient;
import org.apache.flink.streaming.api.environment.StreamExecutionEnvironment;
import org.apache.flink.table.api.*;
import org.apache.flink.table.api.bridge.java.StreamTableEnvironment;
import org.bson.BSONObject;
import org.bson.BasicBSONObject;
import org.testng.Assert;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import java.util.Optional;

/**
 * @descreption seqDB-26426:创建flink表指定overwrite=false，插入数据
 * @author YiPan
 * @date 2022/4/26
 * @updateUser
 * @updateDate
 * @updateRemark
 * @version 1.0
 */
public class CreateTable26426 extends FlinkTestBase {
    private StreamTableEnvironment tableEnv;
    private final String csName = "cs_26426";
    private final String clName = "cl_26426";
    private final String filed_String = "test_s";
    private final String filed_int = "test_i";
    private Sequoiadb sdb;
    private final String tableName = "tb_26426";

    @BeforeClass
    public void setUp() {
        StreamExecutionEnvironment env = StreamExecutionEnvironment
                .getExecutionEnvironment();
        tableEnv = StreamTableEnvironment.create( env );
        sdb = new Sequoiadb( FlinkTestBase.getCoord(), FlinkTestBase.username,
                FlinkTestBase.password );
        if ( sdb.isCollectionSpaceExist( csName ) ) {
            sdb.dropCollectionSpace( csName );
        }
    }

    @Test
    public void test() throws Exception {
        // 创建sdb集合,创建主键唯一索引
        CollectionSpace cs = sdb.createCollectionSpace( csName );
        DBCollection cl = cs.createCollection( clName );
        cl.createIndex( "primarykey", new BasicBSONObject( filed_int, 1 ), true,
                false );

        // flink创建表指定主键
        Schema schema = Schema.newBuilder()
                .column( filed_int, DataTypes.INT().notNull() )
                .column( filed_String, DataTypes.STRING() )
                .primaryKey( filed_int ).build();
        TableDescriptor tableDescriptor = createTableDescriptor( schema,
                "false" );
        tableEnv.createTable( tableName, tableDescriptor );

        // 插入数据
        Commlib.waitJobFinish( tableEnv.executeSql(
                "insert into " + tableName + " values(1,'aaa')" ) );

        // 插入主键重复数据,插入的任务失败
        TableResult tableResult = tableEnv
                .executeSql( "insert into " + tableName + " values(1,'bbb')" );
        JobClient jobClient = tableResult.getJobClient().get();
        try {
            Commlib.waitJobStatus( tableResult, JobStatus.RESTARTING, 30000 );
        } finally {
            jobClient.cancel();
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

    private TableDescriptor createTableDescriptor( Schema schema,
            String isOverWrite ) {
        TableDescriptor.Builder option = Commlib.createBaseTableOption( schema,
                csName, clName );
        option.option( SDBAttribute.overwrite, isOverWrite );
        return option.build();
    }
}
