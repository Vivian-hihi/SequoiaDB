package com.sequoiadb.ddl;

import com.sequoiadb.base.CollectionSpace;
import com.sequoiadb.base.Sequoiadb;
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
 * @descreption seqDB-25314:删除SDB映射表
 * @author YiPan
 * @date 2022/2/28
 * @updateUser
 * @updateDate
 * @updateRemark
 * @version 1.0
 */
public class CreateTable25314 extends FlinkTestBase {
    private StreamTableEnvironment tableEnv;
    private final String csName = "cs_25314";
    private final String clName = "cl_25314";
    private final String filed_A = "test_a";
    private final String filed_B = "test_b";
    private final String tableName = "tb_25314";
    private Sequoiadb sdb;
    private TableDescriptor tableDescriptor;

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
        tableDescriptor = Commlib.createTableDescriptor( schema, csName,
                clName );
    }

    @Test
    public void test() {
        // sdb中集合已存在
        CollectionSpace cs = sdb.createCollectionSpace( csName );
        cs.createCollection( clName );
        tableEnv.createTable( tableName, tableDescriptor );
        tableEnv.executeSql( "drop table " + tableName );
        Assert.assertTrue( cs.isCollectionExist( clName ) );

        // 重复删除不存在集合
        try {
            tableEnv.executeSql( "drop table " + tableName );
        } catch ( ValidationException e ) {
            if ( !( e.getMessage().contains( "does not exist" ) ) ) {
                throw e;
            }
        }

        // sdb中集合不存在
        Commlib.dropCS( sdb, csName );
        tableEnv.createTable( tableName, tableDescriptor );
        tableEnv.executeSql( "drop table " + tableName );
        Assert.assertFalse( cs.isCollectionExist( clName ) );
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
