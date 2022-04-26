package com.sequoiadb.option;

import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.testcommon.FlinkTestBase;
import com.sequoiadb.testcommon.utils.Commlib;
import com.sequoiadb.testcommon.utils.SDBAttribute;
import org.apache.flink.streaming.api.environment.StreamExecutionEnvironment;
import org.apache.flink.table.api.*;
import org.apache.flink.table.api.bridge.java.StreamTableEnvironment;
import org.testng.Assert;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

/**
 * @descreption seqDB-25263:创建SDB映射表，指定参数connector
 *              seqDB-25265:创建SDB映射表，指定参数collectionspace
 *              seqDB-25267:创建SDB映射表，指定参数username
 * @author YiPan
 * @date 2022/2/11
 * @updateUser
 * @updateDate
 * @updateRemark
 * @version 1.0
 */
public class CreateTable25263_25265_25267 extends FlinkTestBase {
    private StreamTableEnvironment tableEnv;
    private final String csName = "cs_25263";
    private final String clName = "cl_25263";
    private final String tableNameBase = "tb_25263_";
    private Schema schema;

    @BeforeClass
    public void setUp() {
        StreamExecutionEnvironment env = StreamExecutionEnvironment
                .getExecutionEnvironment();
        tableEnv = StreamTableEnvironment.create( env );
        schema = Schema.newBuilder().column( "a", DataTypes.INT().notNull() )
                .column( "b", DataTypes.VARCHAR( 50 ) ).primaryKey( "a" )
                .build();
    }

    @Test
    public void test() throws Exception {
        // connector为有效值 csName为有效值 username为有效值
        String tableName = tableNameBase + "sequoiadb";
        TableDescriptor sequoiadb = createTableDescriptor( schema,
                "sequoiadb" );
        tableEnv.createTable( tableName, sequoiadb );
        TableResult tableResult = tableEnv
                .executeSql( "insert into  " + tableName + " values(1,'abc')" );
        Commlib.waitJobFinish( tableResult );

        // connector为无效值
        String[] invalidConnector = { "abc", "123", "!@#" };
        for ( int i = 0; i < invalidConnector.length; i++ ) {
            TableDescriptor tableDescriptor = createTableDescriptor( schema,
                    invalidConnector[ i ] );
            try {
                tableEnv.createTable( tableNameBase + i, tableDescriptor );
                tableEnv.executeSql( "select * from " + tableNameBase + i );
                Assert.fail( "except fail but success" );
            } catch ( ValidationException e ) {
                String cause = e.getCause().toString();
                if ( !( cause.contains( "Cannot discover a connector" ) ) ) {
                    throw e;
                }
            }
        }
    }

    @AfterClass
    public void tearDown() {
        Sequoiadb sdb = new Sequoiadb( FlinkTestBase.getCoord(),
                FlinkTestBase.username, FlinkTestBase.password );
        try {
            Commlib.dropCS( sdb, csName );
        } finally {
            sdb.close();
        }
    }

    private TableDescriptor createTableDescriptor( Schema schema,
            String Connector ) {
        return TableDescriptor.forConnector( Connector ).schema( schema )
                .option( SDBAttribute.hosts, FlinkTestBase.getCoord() )
                .option( SDBAttribute.collectionspace, csName )
                .option( SDBAttribute.collection, clName )
                .option( SDBAttribute.username, FlinkTestBase.username )
                .option( SDBAttribute.password, FlinkTestBase.password )
                .build();
    }
}
