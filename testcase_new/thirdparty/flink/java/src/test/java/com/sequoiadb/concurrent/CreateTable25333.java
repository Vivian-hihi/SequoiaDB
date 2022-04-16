package com.sequoiadb.concurrent;

import com.sequoiadb.base.CollectionSpace;
import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.threadexecutor.ThreadExecutor;
import com.sequoiadb.threadexecutor.annotation.ExecuteOrder;
import com.sequoiadb.testcommon.FlinkTestBase;
import com.sequoiadb.testcommon.utils.Commlib;
import org.apache.commons.lang3.RandomStringUtils;
import org.apache.flink.streaming.api.environment.StreamExecutionEnvironment;
import org.apache.flink.table.api.DataTypes;
import org.apache.flink.table.api.Schema;
import org.apache.flink.table.api.TableDescriptor;
import org.apache.flink.table.api.TableResult;
import org.apache.flink.table.api.bridge.java.StreamTableEnvironment;
import org.apache.flink.types.Row;
import org.bson.BasicBSONObject;
import org.testng.Assert;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import java.util.ArrayList;
import java.util.List;
import java.util.Random;

/**
 * @descreption seqDB-25333:并发查询数据
 * @author YiPan
 * @date 2022/3/15
 * @updateUser
 * @updateDate
 * @updateRemark
 * @version 1.0
 */
public class CreateTable25333 extends FlinkTestBase {
    private Sequoiadb sdb;
    private final String csName = "cs_25333";
    private final String clName = "cl_25333";
    private final String filed_int = "test_int";
    private final String filed_String = "test_string";
    private final String filed_boolean = "test_boolean";
    private final String tableName = "tb_25333";
    private final int recordNum = 10000;
    private DBCollection cl;
    private ArrayList< Row > allRows;
    private TableDescriptor tableDescriptor;

    @BeforeClass
    public void setUp() {
        sdb = new Sequoiadb( FlinkTestBase.getCoord(), FlinkTestBase.username,
                FlinkTestBase.password );
        Schema schema = Schema.newBuilder().column( filed_int, DataTypes.INT() )
                .column( filed_String, DataTypes.STRING() )
                .column( filed_boolean, DataTypes.BOOLEAN() ).build();
        tableDescriptor = Commlib.createTableDescriptor( schema, csName,
                clName );
        Commlib.dropCS( sdb, csName );
        CollectionSpace cs = sdb.createCollectionSpace( csName );
        cl = cs.createCollection( clName );
        allRows = insertData();
    }

    @Test
    public void test() throws Exception {
        // 并发创建表查询
        ThreadExecutor t = new ThreadExecutor();
        Select selectAll = new Select( "select * from " + tableName );
        Select selectHalf = new Select( "select * from " + tableName + " where "
                + filed_int + "> " + recordNum / 2 );
        t.addWorker( selectAll );
        t.addWorker( selectHalf );
        t.run();

        // 校验查询结果，使用无排序校验方法
        Assert.assertEqualsNoOrder( selectAll.getRows().toArray(),
                allRows.toArray() );
        List< Row > halfRows = allRows.subList( recordNum / 2 + 1,
                allRows.size() );
        Assert.assertEqualsNoOrder( selectHalf.getRows().toArray(),
                halfRows.toArray() );
    }

    @AfterClass
    public void tearDown() {
        try {
            sdb.dropCollectionSpace( csName );
        } finally {
            sdb.close();
        }
    }

    private class Select {
        String sql;
        ArrayList< Row > rows;

        public Select( String sql ) {
            this.sql = sql;
        }

        public ArrayList< Row > getRows() {
            return rows;
        }

        @ExecuteOrder(step = 1)
        private void run() {
            StreamExecutionEnvironment env = StreamExecutionEnvironment
                    .getExecutionEnvironment();
            StreamTableEnvironment tableEnv = StreamTableEnvironment.create( env );
            tableEnv.createTable( tableName, tableDescriptor );
            TableResult tableResult = tableEnv.executeSql( sql );
            rows = Commlib.collectToArrayList( tableResult.collect() );
        }
    }

    private ArrayList< Row > insertData() {
        ArrayList< Row > allResult = new ArrayList<>();
        for ( int i = 0; i < recordNum; i++ ) {
            int data_int = i;
            String data_string = RandomStringUtils.randomAlphanumeric( 20 );
            boolean data_boolean = new Random().nextBoolean();
            Row row = new Row( 3 );
            row.setField( 0, data_int );
            row.setField( 1, data_string );
            row.setField( 2, data_boolean );
            BasicBSONObject record = new BasicBSONObject();
            record.put( filed_int, data_int );
            record.put( filed_String, data_string );
            record.put( filed_boolean, data_boolean );
            cl.insert( record );
            allResult.add( row );
        }
        return allResult;
    }
}
