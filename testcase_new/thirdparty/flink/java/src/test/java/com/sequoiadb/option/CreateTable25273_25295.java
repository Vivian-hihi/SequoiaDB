package com.sequoiadb.option;

import com.sequoiadb.base.CollectionSpace;
import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.DBCursor;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.testcommon.FlinkTestBase;
import com.sequoiadb.testcommon.utils.Commlib;
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
 * @descreption seqDB-25273:创建SDB映射表，SDB中集合/集合空间不存在
 *              seqDB-25295:创建SDB映射表，SDB集合参数使用默认值
 * @author YiPan
 * @date 2022/2/11
 * @updateUser
 * @updateDate
 * @updateRemark
 * @version 1.0
 */
public class CreateTable25273_25295 extends FlinkTestBase {
    private StreamTableEnvironment tableEnv;
    private final String csName = "cs_25273";
    private final String clName = "cl_25273";
    private final String filed_A = "test_a";
    private final String filed_B = "test_b";
    private final String data_A = "abc";
    private final int data_B = 123;
    private Sequoiadb sdb;
    private final String tableNameBase = "tb_25273";

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
        // flink创建表
        Schema schema = Schema.newBuilder()
                .column( filed_A, DataTypes.STRING() )
                .column( filed_B, DataTypes.INT() ).build();
        TableDescriptor tableDescriptor = Commlib.createTableDescriptor( schema,
                csName, clName );
        tableEnv.createTable( tableNameBase, tableDescriptor );

        // 插入数据
        TableResult tableResult = tableEnv.executeSql( "insert into "
                + tableNameBase + " values('" + data_A + "', " + data_B + ")" );

        // 等待job执行完
        Commlib.waitJobFinish( tableResult );

        // 校验结果
        checkResult();

        // 校验集合属性
        checkClOptions();
    }

    private void checkClOptions() {
        DBCursor snap_catalog = sdb.getSnapshot( Sequoiadb.SDB_SNAP_CATALOG,
                new BasicBSONObject( "Name", csName + "." + clName ), null,
                null );
        try {
            BSONObject options = snap_catalog.getNext();
            Assert.assertEquals( options.get( "CompressionTypeDesc" ), "lzw" );
            // ReplSize使用默认值，catalog快照会隐藏该参数
            if ( options.get( "ReplSize" ) != null ) {
                Assert.assertEquals( options.get( "ReplSize" ), 1 );
            }
        }finally {
            snap_catalog.close();
        }

        DBCursor snap_cs = sdb.getSnapshot( Sequoiadb.SDB_SNAP_COLLECTIONSPACES,
                new BasicBSONObject( "Name", csName ), null, null );
        try {
            BSONObject options = snap_cs.getNext();
            Assert.assertEquals( options.get( "PageSize" ), 65536 );
        }finally {
            snap_cs.close();
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

    private void checkResult() {
        CollectionSpace cs = sdb.getCollectionSpace( csName );
        DBCollection cl = cs.getCollection( clName );
        BSONObject actual = cl.queryOne();
        actual.removeField( "_id" );
        BasicBSONObject except = new BasicBSONObject();
        except.put( filed_A, data_A );
        except.put( filed_B, data_B );
        Assert.assertEquals( actual, except );
    }
}
