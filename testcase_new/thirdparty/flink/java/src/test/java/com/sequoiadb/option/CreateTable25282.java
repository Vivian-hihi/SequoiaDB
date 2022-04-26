package com.sequoiadb.option;

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
 * @descreption seqDB-25282:创建SDB映射表，指定参数ignorenullfield
 * @author YiPan
 * @date 2022/2/11
 * @updateUser
 * @updateDate
 * @updateRemark
 * @version 1.0
 */
public class CreateTable25282 extends FlinkTestBase {
    private StreamTableEnvironment tableEnv;
    private Sequoiadb sdb;
    private final String csName = "cs_25282";
    private final String clName_A = "cl_25282_A";
    private final String clName_B = "cl_25282_B";
    private final String filed_A = "test_a";
    private final String filed_B = "test_b";
    private final String filed_PK = "test_pk";
    private final String tableName_A = "tb_25278_A";
    private final String tableName_B = "tb_25278_B";
    private final String ignorenullfield = "true";

    @BeforeClass
    public void setUp() {
        StreamExecutionEnvironment env = StreamExecutionEnvironment
                .getExecutionEnvironment();
        tableEnv = StreamTableEnvironment.create( env );
        sdb = new Sequoiadb( FlinkTestBase.getCoord(), FlinkTestBase.username,
                FlinkTestBase.password );
        Commlib.dropCS( sdb, csName );
        insertNullData();
    }

    @Test
    public void test() throws Exception {
        Schema schema = Schema.newBuilder()
                .column( filed_PK, DataTypes.INT().notNull() )
                .column( filed_A, DataTypes.STRING() )
                .column( filed_B, DataTypes.INT() ).primaryKey( filed_PK )
                .build();
        tableEnv.createTable( tableName_A,
                createTableDescriptor( schema, csName, clName_A ) );
        tableEnv.createTable( tableName_B,
                createTableDescriptor( schema, csName, clName_B ) );

        Commlib.waitJobFinish( tableEnv.executeSql( "insert into " + tableName_B
                + " select * from " + tableName_A ) );

        // 校验插入到sdb表忽略了null字段
        DBCollection cl = sdb.getCollectionSpace( csName )
                .getCollection( clName_B );
        BSONObject result = cl.queryOne();
        result.removeField( "_id" );
        result.removeField( filed_PK );
        Assert.assertTrue( result.isEmpty() );
    }

    @AfterClass
    public void tearDown() {
        try {
            Commlib.dropCS( sdb, csName );
        } finally {
            sdb.close();
        }
    }

    private TableDescriptor createTableDescriptor( Schema schema, String csName,
            String clName ) {
        TableDescriptor.Builder option = Commlib.createBaseTableOption( schema,
                csName, clName );
        option.option( SDBAttribute.ignorenullfield, ignorenullfield );
        return option.build();
    }

    private void insertNullData() {
        CollectionSpace cs = sdb.createCollectionSpace( csName );
        DBCollection cl = cs.createCollection( clName_A );
        BasicBSONObject record = new BasicBSONObject();
        record.put( filed_PK, 1 );
        record.put( filed_A, null );
        cl.insertRecord( record );
    }
}
