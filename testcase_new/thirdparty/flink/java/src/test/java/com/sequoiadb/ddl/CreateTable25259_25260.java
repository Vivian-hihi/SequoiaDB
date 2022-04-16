package com.sequoiadb.ddl;

import com.sequoiadb.base.CollectionSpace;
import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.testcommon.FlinkTestBase;
import com.sequoiadb.testcommon.utils.Commlib;
import com.sequoiadb.testcommon.utils.ConversionUtils;
import org.apache.flink.streaming.api.environment.StreamExecutionEnvironment;
import org.apache.flink.table.api.DataTypes;
import org.apache.flink.table.api.Schema;
import org.apache.flink.table.api.TableDescriptor;
import org.apache.flink.table.api.bridge.java.StreamTableEnvironment;
import org.apache.flink.types.Row;
import org.bson.BasicBSONObject;
import org.testng.Assert;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

/**
 * @descreption seqDB-25259:创建SDB映射表，定义字段已存在SDB中
 *              seqDB-25260:创建SDB映射表，只映射存量数据部分字段
 * @author YiPan
 * @date 2022/2/22
 * @updateUser
 * @updateDate
 * @updateRemark
 * @version 1.0
 */
public class CreateTable25259_25260 extends FlinkTestBase {
    private StreamTableEnvironment tableEnv;
    private final String csName = "cs_25259";
    private final String clName = "cl_25259";
    private final String filed_a = "test_a";
    private final String filed_b = "test_b";
    private final String filed_c = "test_c";
    private final String filed_d = "test_d";
    private final String data_String = "abc";
    private final int data_int = 123;
    private DBCollection cl;
    private Sequoiadb sdb;
    private final String tableName = "tb_25259";

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
        CollectionSpace cs = sdb.createCollectionSpace( csName );
        cl = cs.createCollection( clName );
        insertData();
    }

    @Test
    public void test() throws Exception {
        Schema schema = Schema.newBuilder()
                .column( filed_a, DataTypes.STRING() )
                .column( filed_b, DataTypes.STRING() )
                .column( filed_c, DataTypes.INT() ).build();
        TableDescriptor tableDescriptor = Commlib.createTableDescriptor( schema,
                csName, clName );
        tableEnv.createTable( tableName, tableDescriptor );
        Row row = ConversionUtils.queryOne( tableEnv, tableName );
        // 发生转换字段
        Assert.assertEquals( row.getField( filed_b ),
                String.valueOf( data_int ) );
        // 正确类型的字段
        Assert.assertEquals( row.getField( filed_a ), data_String );
        // flink映射的sdb中不存在字段
        Assert.assertNull( row.getField( filed_c ) );
        // sdb中未被flink映射的字段
        try {
            row.getField( filed_d );
            Assert.fail( "except fail but success" );
        } catch ( IllegalArgumentException e ) {
            if ( !( e.getMessage().contains(
                    "Unknown field name 'test_d' for mapping to a position" ) ) ) {
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

    private void insertData() {
        BasicBSONObject bson = new BasicBSONObject();
        bson.put( filed_a, data_String );
        bson.put( filed_b, data_int );
        bson.put( filed_d, "sequoiadb" );
        cl.insert( bson );
    }
}
