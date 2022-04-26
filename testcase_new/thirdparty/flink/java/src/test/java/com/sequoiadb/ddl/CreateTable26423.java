package com.sequoiadb.ddl;

import com.sequoiadb.base.CollectionSpace;
import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.testcommon.FlinkTestBase;
import com.sequoiadb.testcommon.utils.Commlib;
import com.sequoiadb.testcommon.utils.SDBAttribute;
import org.apache.flink.streaming.api.environment.StreamExecutionEnvironment;
import org.apache.flink.table.api.*;
import org.apache.flink.table.api.bridge.java.StreamTableEnvironment;
import org.bson.BasicBSONObject;
import org.testng.Assert;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

/**
 * @descreption seqDB-26423:sdb集合已存在，flink表无主键，指定overwrite=true插入数据
 * @author YiPan
 * @date 2022/4/26
 * @updateUser
 * @updateDate
 * @updateRemark
 * @version 1.0
 */
public class CreateTable26423 extends FlinkTestBase {
    private StreamTableEnvironment tableEnv;
    private final String csName = "cs_26423";
    private final String clName = "cl_26423";
    private final String filed_String = "test_s";
    private final String filed_int = "test_i";
    private Sequoiadb sdb;
    private final String tableName = "tb_26423";

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
        Schema schema = Schema.newBuilder().column( filed_int, DataTypes.INT() )
                .column( filed_String, DataTypes.STRING() ).build();
        TableDescriptor tableDescriptor = Commlib.createTableDescriptor( schema,
                csName, clName );
        tableEnv.createTable( tableName, tableDescriptor );

        // 插入数据验证报错
        try {
            tableEnv.executeSql(
                    "insert into " + tableName + " values(1,'aaa')" );
            Assert.fail( "except fail but success" );
        } catch ( ValidationException e ) {
            String message = e.getCause().getMessage();
            if ( !( message.contains(
                    "Can not perform idempotent write without primary key/unique key" ) ) ) {
                throw e;
            }
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
}
