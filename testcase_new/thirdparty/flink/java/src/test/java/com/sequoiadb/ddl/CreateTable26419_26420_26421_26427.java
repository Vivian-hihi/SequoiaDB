package com.sequoiadb.ddl;

import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.exception.SDBError;
import com.sequoiadb.testcommon.FlinkTestBase;
import com.sequoiadb.testcommon.utils.Commlib;
import com.sequoiadb.testcommon.utils.SDBAttribute;
import org.apache.flink.streaming.api.environment.StreamExecutionEnvironment;
import org.apache.flink.table.api.DataTypes;
import org.apache.flink.table.api.Schema;
import org.apache.flink.table.api.TableDescriptor;
import org.apache.flink.table.api.ValidationException;
import org.apache.flink.table.api.bridge.java.StreamTableEnvironment;
import org.bson.BSONObject;
import org.bson.BasicBSONObject;
import org.testng.Assert;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

/**
 * @descreption seqDB-26419:指定overwrite=true，指定主键，自动创建集合和唯一索引
 *              seqDB-26420:指定overwrite=true，不指定主键，自动创建集合
 *              seqDB-26421:指定overwrite=false，指定主键，自动创建集合和唯一索引
 *              seqDB-26427:指定overwrite=false，不指定主键，自动创建集合
 * @author YiPan
 * @date 2022/4/26
 * @updateUser
 * @updateDate
 * @updateRemark
 * @version 1.0
 */
public class CreateTable26419_26420_26421_26427 extends FlinkTestBase {
    private StreamTableEnvironment tableEnv;
    private final String csName = "cs_26419";
    private final String clName = "cl_26419";
    private final String filed_String = "test_s";
    private final String filed_int = "test_i";
    private Sequoiadb sdb;
    private final String tableName = "tb_26419";

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
        Schema schema = Schema.newBuilder()
                .column( filed_int, DataTypes.INT().notNull() )
                .column( filed_String, DataTypes.STRING().notNull() )
                .primaryKey( filed_int, filed_String ).build();
        // seqDB-26419:指定overwrite=true，指定主键，自动创建集合和唯一索引
        createTable( schema, "true" );
        insertData();
        checkUniqueIndexInfo( filed_int, filed_String );

        // seqDB-26421:指定overwrite=false，指定主键，自动创建集合和唯一索引
        createTable( schema, "true" );
        insertData();
        checkUniqueIndexInfo( filed_int, filed_String );

        schema = Schema.newBuilder().column( filed_int, DataTypes.INT() )
                .column( filed_String, DataTypes.STRING() ).build();
        // seqDB-26420:指定overwrite=true，不指定主键，自动创建集合
        createTable( schema, "true" );
        try {
            insertData();
            Assert.fail( "except fail but success" );
        } catch ( ValidationException e ) {
            String message = e.getCause().getMessage();
            if ( !( message.contains(
                    "Can not perform idempotent write without primary key/unique key" ) ) ) {
                throw e;
            }
        }

        schema = Schema.newBuilder().column( filed_int, DataTypes.INT() )
                .column( filed_String, DataTypes.STRING() ).build();
        // seqDB-26427:指定overwrite=false，不指定主键，自动创建集合
        createTable( schema, "false" );
        insertData();
        DBCollection cl = sdb.getCollectionSpace( csName )
                .getCollection( clName );
        Assert.assertFalse( cl.isIndexExist( "primarykey" ) );
    }

    @AfterClass
    public void tearDown() {
        try {
            sdb.dropCollectionSpace( csName );
        } finally {
            sdb.close();
        }
    }

    private void createTable( Schema schema, String isOverWrite ) {
        tableEnv.executeSql( "drop table if exists " + tableName );
        TableDescriptor tableDescriptor = createTableDescriptor( schema,
                isOverWrite );
        tableEnv.createTable( tableName, tableDescriptor );
    }

    private void insertData() throws Exception {
        if ( sdb.isCollectionSpaceExist( csName ) ) {
            sdb.dropCollectionSpace( csName );
        }
        Commlib.waitJobFinish( tableEnv.executeSql(
                "insert into " + tableName + " values(1,'abc')" ) );
    }

    private void checkUniqueIndexInfo( String... primaryKey ) {
        DBCollection cl = sdb.getCollectionSpace( csName )
                .getCollection( clName );
        BSONObject indexinfo = cl.getIndexInfo( "primarykey" );
        BSONObject exceptKey = new BasicBSONObject();
        for ( int i = 0; i < primaryKey.length; i++ ) {
            exceptKey.put( primaryKey[ i ], 1 );
        }
        BSONObject indexDef = ( BSONObject ) indexinfo.get( "IndexDef" );
        Assert.assertEquals( indexDef.get( "unique" ), true );
        Assert.assertEquals( indexDef.get( "key" ), exceptKey );
    }

    private TableDescriptor createTableDescriptor( Schema schema,
            String isOverWrite ) {
        TableDescriptor.Builder option = Commlib.createBaseTableOption( schema,
                csName, clName );
        option.option( SDBAttribute.overwrite, isOverWrite );
        return option.build();
    }
}
