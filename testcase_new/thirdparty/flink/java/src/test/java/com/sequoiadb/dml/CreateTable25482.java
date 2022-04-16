package com.sequoiadb.dml;

import com.sequoiadb.base.CollectionSpace;
import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.testcommon.FlinkTestBase;
import com.sequoiadb.testcommon.utils.Commlib;
import org.apache.flink.streaming.api.environment.StreamExecutionEnvironment;
import org.apache.flink.table.api.*;
import org.apache.flink.table.api.bridge.java.StreamTableEnvironment;
import org.apache.flink.table.types.DataType;
import org.apache.flink.types.Row;
import org.bson.BasicBSONObject;
import org.testng.Assert;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.DataProvider;
import org.testng.annotations.Test;

import java.sql.SQLException;
import java.util.ArrayList;

/**
 * @descreption seqDB-25482:使用INSERT INTO SELECT插入空数据至空表中
 * @author YiPan
 * @date 2022/3/3
 * @updateUser
 * @updateDate
 * @updateRemark
 * @version 1.0
 */
public class CreateTable25482 extends FlinkTestBase {
    private final String tableName_A = "tb_25482_A";
    private final String tableName_B = "tb_25482_B";
    private final String csName = "cs_25482";
    private final String clName_A = "cl_25482_A";
    private final String clName_B = "cl_25482_B";
    private final String filed_A = "test_A";
    private final String filed_B = "test_B";
    private Sequoiadb sdb;
    private CollectionSpace cs;

    @BeforeClass
    public void setUp() throws SQLException, ClassNotFoundException {
        sdb = new Sequoiadb( FlinkTestBase.getCoord(), FlinkTestBase.username,
                FlinkTestBase.password );
        Commlib.dropCS( sdb, csName );
        cs = sdb.createCollectionSpace( csName );
        cs.createCollection( clName_B );
        insertData();
    }

    @DataProvider(name = "dataProvider")
    public Object[] dataType() {
        return new Object[] { DataTypes.TINYINT(), DataTypes.SMALLINT(),
                DataTypes.INT(), DataTypes.BIGINT(), DataTypes.FLOAT(),
                DataTypes.DOUBLE(), DataTypes.DECIMAL( 38, 10 ),
                DataTypes.STRING(), DataTypes.CHAR( 10 ),
                DataTypes.VARCHAR( 10 ), DataTypes.BOOLEAN(), DataTypes.DATE(),
                DataTypes.TIMESTAMP(), DataTypes.BINARY( 100 ),
                DataTypes.VARBINARY( 100 ), DataTypes.BYTES() };
    }

    @Test(dataProvider = "dataProvider")
    public void test( DataType dataType ) throws Exception {
        if ( cs.isCollectionExist( clName_B ) ) {
            cs.dropCollection( clName_B );
        }
        StreamExecutionEnvironment env = StreamExecutionEnvironment
                .getExecutionEnvironment();
        StreamTableEnvironment tableEnv = StreamTableEnvironment.create( env );

        Schema schema = Schema.newBuilder()
                .column( filed_A, DataTypes.VARCHAR( 10 ) )
                .column( filed_B, dataType ).build();
        TableDescriptor tableDesc_data = Commlib.createTableDescriptor( schema,
                csName, clName_A );
        TableDescriptor tableDesc_null = Commlib.createTableDescriptor( schema,
                csName, clName_B );
        tableEnv.createTable( tableName_A, tableDesc_data );
        tableEnv.createTable( tableName_B, tableDesc_null );
        tableEnv.executeSql( "insert into " + tableName_B + " select * from "
                + tableName_A );
        ArrayList< Row > resultA = Commlib.collectToArrayList( tableEnv
                .executeSql( "select * from " + tableName_A ).collect() );
        ArrayList< Row > resultB = Commlib.collectToArrayList( tableEnv
                .executeSql( "select * from " + tableName_B ).collect() );
        Assert.assertEqualsNoOrder( resultA.toArray(), resultB.toArray() );
    }

    @AfterClass
    public void tearDown() {
        try {
            Commlib.dropCS( sdb, csName );
        } finally {
            sdb.close();
        }
    }

    private void insertData() {
        DBCollection cl = cs.createCollection( clName_A );
        cl.insert( new BasicBSONObject( filed_A, null ) );
        cl.insert( new BasicBSONObject( filed_B, null ) );
        cl.insert( new BasicBSONObject( filed_A, "" ) );
        cl.insert( new BasicBSONObject( filed_A, "null" ) );
    }
}
