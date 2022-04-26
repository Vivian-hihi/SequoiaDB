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
import org.bson.BSONObject;
import org.bson.BasicBSONObject;
import org.testng.Assert;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

/**
 * @descreption seqDB-26425:使用可幂等写，出现索引键冲突数据
 * @author YiPan
 * @date 2022/4/26
 * @updateUser
 * @updateDate
 * @updateRemark
 * @version 1.0
 */
public class CreateTable26425 extends FlinkTestBase {
    private StreamTableEnvironment tableEnv;
    private final String csName = "cs_26425";
    private final String clName = "cl_26425";
    private final String filed_String = "test_s";
    private final String filed_int = "test_i";
    private Sequoiadb sdb;
    private final String tableName = "tb_26425";

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
        // flink创建表指定主键
        Schema schema = Schema.newBuilder()
                .column( filed_int, DataTypes.INT().notNull() )
                .column( filed_String, DataTypes.STRING() )
                .primaryKey( filed_int ).build();
        TableDescriptor tableDescriptor = Commlib.createTableDescriptor( schema,
                csName, clName );
        tableEnv.createTable( tableName, tableDescriptor );

        // 插入主键重复数据
        Commlib.waitJobFinish( tableEnv.executeSql(
                "insert into " + tableName + " values(1,'aaa')" ) );
        Commlib.waitJobFinish( tableEnv.executeSql(
                "insert into " + tableName + " values(1,'bbb')" ) );
        Commlib.waitJobFinish( tableEnv.executeSql(
                "insert into " + tableName + " values(1,'ccc')" ) );

        DBCollection cl = sdb.getCollectionSpace( csName )
                .getCollection( clName );
        // 校验记录数
        Assert.assertEquals( cl.getCount(), 1 );
        // 校验主键重复数据内容覆写
        BSONObject actRecord = cl.queryOne();
        actRecord.removeField( "_id" );
        BasicBSONObject expRecord = new BasicBSONObject();
        expRecord.put( filed_int, 1 );
        expRecord.put( filed_String, "ccc" );
        Assert.assertEquals( actRecord, expRecord );
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
