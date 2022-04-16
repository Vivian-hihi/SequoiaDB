package com.sequoiadb.option;

import com.sequoiadb.base.CollectionSpace;
import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.testcommon.FlinkTestBase;
import com.sequoiadb.testcommon.utils.Commlib;
import com.sequoiadb.testcommon.utils.ConversionUtils;
import com.sequoiadb.testcommon.utils.SDBAttribute;
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
 * @descreption seqDB-25271:创建SDB映射表，指定多个hosts
 * @author YiPan
 * @date 2022/2/11
 * @updateUser
 * @updateDate
 * @updateRemark
 * @version 1.0
 */
public class CreateTable25271 extends FlinkTestBase {
    private StreamTableEnvironment tableEnv;
    private final String csName = "cs_25271";
    private final String clName = "cl_25271";
    private final String filed_A = "test_a";
    private final String filed_B = "test_b";
    private final String data_A = "abc";
    private final int data_B = 123;
    private DBCollection cl;
    private Sequoiadb sdb;
    private final String tableNameBase = "tb_25271_";
    private Schema schema;

    @BeforeClass
    public void setUp() {
        StreamExecutionEnvironment env = StreamExecutionEnvironment
                .getExecutionEnvironment();
        tableEnv = StreamTableEnvironment.create( env );
        sdb = new Sequoiadb( FlinkTestBase.getCoord(), FlinkTestBase.username,
                FlinkTestBase.password );
        Commlib.dropCS( sdb, csName );
        CollectionSpace cs = sdb.createCollectionSpace( csName );
        cl = cs.createCollection( clName );
        insertData();
        schema = Schema.newBuilder().column( filed_A, DataTypes.STRING() )
                .column( filed_B, DataTypes.INT() ).build();
    }

    @Test
    public void test() throws Exception {
        // coords为多个有效值
        String validCoords = FlinkTestBase.getCoords();
        TableDescriptor valid = createTableDescriptor( schema, validCoords );
        tableEnv.createTable( tableNameBase + "valid", valid );
        Row validRow = ConversionUtils.queryOne( tableEnv,
                tableNameBase + "valid" );
        Assert.assertEquals( validRow.getField( filed_A ), data_A );
        Assert.assertEquals( validRow.getField( filed_B ), data_B );

        // coords只有一个可用
        String invalidCoords = "1.1.1.1:1234," + FlinkTestBase.getCoord();
        TableDescriptor invalid = createTableDescriptor( schema,
                invalidCoords );
        tableEnv.createTable( tableNameBase + "invalid", invalid );
        Row invalidRow = ConversionUtils.queryOne( tableEnv,
                tableNameBase + "valid" );
        Assert.assertEquals( invalidRow.getField( filed_A ), data_A );
        Assert.assertEquals( invalidRow.getField( filed_B ), data_B );
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
        bson.put( filed_A, data_A );
        bson.put( filed_B, data_B );
        cl.insert( bson );
    }

    private TableDescriptor createTableDescriptor( Schema schema,
            String coords ) {
        return TableDescriptor.forConnector( SDBAttribute.sequoiadb )
                .schema( schema ).option( SDBAttribute.hosts, coords )
                .option( SDBAttribute.collectionspace, csName )
                .option( SDBAttribute.collection, clName )
                .option( SDBAttribute.username, FlinkTestBase.username )
                .option( SDBAttribute.password, FlinkTestBase.password ).build();
    }
}
