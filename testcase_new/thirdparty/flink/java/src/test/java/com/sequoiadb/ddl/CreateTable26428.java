package com.sequoiadb.ddl;

import com.sequoiadb.base.CollectionSpace;
import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.DBCursor;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.testcommon.FlinkTestBase;
import com.sequoiadb.testcommon.utils.Commlib;
import com.sequoiadb.testcommon.utils.SDBAttribute;
import org.apache.flink.streaming.api.environment.StreamExecutionEnvironment;
import org.apache.flink.table.api.DataTypes;
import org.apache.flink.table.api.Schema;
import org.apache.flink.table.api.TableDescriptor;
import org.apache.flink.table.api.bridge.java.StreamTableEnvironment;
import org.bson.BSONObject;
import org.bson.BasicBSONObject;
import org.testng.Assert;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import java.util.ArrayList;
import java.util.List;

/**
 * @descreption seqDB-26428:sdb集合已存在，flink表指定主键，插入数据
 * @author YiPan
 * @date 2022/4/26
 * @updateUser
 * @updateDate
 * @updateRemark
 * @version 1.0
 */
public class CreateTable26428 extends FlinkTestBase {
    private StreamTableEnvironment tableEnv;
    private final String csName = "cs_26428";
    private final String clName = "cl_26428";
    private final String filed_String = "test_s";
    private final String filed_int = "test_i";
    private Sequoiadb sdb;
    private final String tableName = "tb_26428";

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
        // 创建sdb集合
        CollectionSpace cs = sdb.createCollectionSpace( csName );
        DBCollection cl = cs.createCollection( clName );

        // flink创建表指定主键
        Schema schema = Schema.newBuilder()
                .column( filed_int, DataTypes.INT().notNull() )
                .column( filed_String, DataTypes.STRING() )
                .primaryKey( filed_int ).build();
        TableDescriptor tableDescriptor = createTableDescriptor( schema,
                "false" );
        tableEnv.createTable( tableName, tableDescriptor );

        // 插入数据
        Commlib.waitJobFinish( tableEnv.executeSql(
                "insert into " + tableName + " values(1,'aaa')" ) );

        // 校验索引不存在
        Assert.assertFalse( cl.isIndexExist( "primarykey" ) );

        // 插入主键重复数据
        Commlib.waitJobFinish( tableEnv.executeSql(
                "insert into " + tableName + " values(1,'bbb')" ) );

        // 校验插入重复数据成功，未发生覆写
        DBCursor cursor = cl.query( new BasicBSONObject( filed_int, 1 ),
                new BasicBSONObject( filed_String, 1 ), null, null );
        List< BSONObject > actResult = new ArrayList<>();
        while ( cursor.hasNext() ) {
            actResult.add( cursor.getNext() );
        }
        List< BSONObject > expResult = new ArrayList<>();
        expResult.add( new BasicBSONObject( filed_String, "aaa" ) );
        expResult.add( new BasicBSONObject( filed_String, "bbb" ) );
        Assert.assertEqualsNoOrder( actResult.toArray(), expResult.toArray() );
    }

    @AfterClass
    public void tearDown() {
        try {
            sdb.dropCollectionSpace( csName );
        } finally {
            sdb.close();
        }
    }

    private TableDescriptor createTableDescriptor( Schema schema,
            String isOverWrite ) {
        TableDescriptor.Builder option = Commlib.createBaseTableOption( schema,
                csName, clName );
        option.option( SDBAttribute.overwrite, isOverWrite );
        return option.build();
    }
}
