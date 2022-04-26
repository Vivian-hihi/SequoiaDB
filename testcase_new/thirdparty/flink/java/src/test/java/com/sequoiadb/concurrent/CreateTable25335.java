package com.sequoiadb.concurrent;

import com.sequoiadb.base.CollectionSpace;
import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.DBCursor;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.threadexecutor.ThreadExecutor;
import com.sequoiadb.threadexecutor.annotation.ExecuteOrder;
import com.sequoiadb.testcommon.FlinkTestBase;
import com.sequoiadb.testcommon.utils.Commlib;
import org.apache.commons.lang3.RandomStringUtils;
import org.apache.flink.streaming.api.environment.StreamExecutionEnvironment;
import org.apache.flink.table.api.*;
import org.apache.flink.table.api.bridge.java.StreamTableEnvironment;
import org.bson.BSONObject;
import org.bson.BasicBSONObject;
import org.testng.Assert;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import java.util.ArrayList;

/**
 * @descreption seqDB-25335:flink和SDB并发插入数据
 * @author YiPan
 * @date 2022/2/18
 * @updateUser
 * @updateDate
 * @updateRemark
 * @version 1.0
 */
public class CreateTable25335 extends FlinkTestBase {
    private Sequoiadb sdb;
    private final String csName = "cs_25335";
    private final String clName = "cl_25335";
    private final String dataClName = "cl_25335_source";
    private final String filed_int = "test_int";
    private final String filed_String = "test_string";
    private final String tableName = "tb_25335";
    private final String dataTableName = "tb_25335_data";
    private final int recordNum = 1000;
    private DBCollection cl;
    private DBCollection dataCl;
    private ArrayList< BSONObject > sdbInsert = new ArrayList<>();
    private ArrayList< BSONObject > flinkInsert = new ArrayList<>();

    @BeforeClass
    public void setUp() {
        sdb = new Sequoiadb( FlinkTestBase.getCoord(), FlinkTestBase.username,
                FlinkTestBase.password );
        Commlib.dropCS( sdb, csName );
        CollectionSpace cs = sdb.createCollectionSpace( csName );
        cl = cs.createCollection( clName );
        cl.createIndex( "primarykey", new BasicBSONObject( filed_int, 1 ), true,
                false );
        dataCl = cs.createCollection( dataClName );
        DataClInsertdData();
    }

    @Test
    public void test() throws Exception {
        // 并发插入
        ThreadExecutor t = new ThreadExecutor();
        t.addWorker( new InsertIntoSdb() );
        InsertIntoFlink insertIntoFlink = new InsertIntoFlink();
        t.addWorker( insertIntoFlink );
        t.run();

        // 查询结果
        DBCursor cursor = cl.query();
        ArrayList< BSONObject > actualResult = new ArrayList<>();
        while ( cursor.hasNext() ) {
            BSONObject next = cursor.getNext();
            next.removeField( "_id" );
            actualResult.add( next );
        }
        ArrayList< BSONObject > exceptResult = new ArrayList<>();
        exceptResult.addAll( flinkInsert );
        exceptResult.addAll( sdbInsert );
        // 无排序校验
        Assert.assertEqualsNoOrder( actualResult.toArray(),
                exceptResult.toArray() );
    }

    @AfterClass
    public void tearDown() {
        try {
            sdb.dropCollectionSpace( csName );
        } finally {
            sdb.close();
        }
    }

    private class InsertIntoFlink {

        @ExecuteOrder(step = 1)
        private void run() throws Exception {
            StreamExecutionEnvironment env = StreamExecutionEnvironment
                    .getExecutionEnvironment();
            StreamTableEnvironment tableEnv = StreamTableEnvironment
                    .create( env );
            Schema schema = Schema.newBuilder()
                    .column( filed_int, DataTypes.INT().notNull() )
                    .column( filed_String, DataTypes.STRING() )
                    .primaryKey( filed_int ).build();
            TableDescriptor table = Commlib.createTableDescriptor( schema,
                    csName, clName );
            TableDescriptor dataTable = Commlib.createTableDescriptor( schema,
                    csName, dataClName );
            tableEnv.createTable( tableName, table );
            tableEnv.createTable( dataTableName, dataTable );
            TableResult tableResult = tableEnv.executeSql( "insert into "
                    + tableName + " select * from " + dataTableName );
            Commlib.waitJobFinish( tableResult );
        }
    }

    private class InsertIntoSdb {
        @ExecuteOrder(step = 1)
        private void run() {
            for ( int i = recordNum / 2; i < recordNum; i++ ) {
                BSONObject record = new BasicBSONObject();
                String data_string = RandomStringUtils.randomAlphanumeric( 10 );
                record.put( filed_int, i );
                record.put( filed_String, data_string );
                cl.insert( record );
                record.removeField( "_id" );
                sdbInsert.add( record );
            }
        }
    }

    private void DataClInsertdData() {
        for ( int i = 0; i < recordNum / 2; i++ ) {
            BSONObject record = new BasicBSONObject();
            String data_string = RandomStringUtils.randomAlphanumeric( 10 );
            record.put( filed_int, i );
            record.put( filed_String, data_string );
            dataCl.insert( record );
            record.removeField( "_id" );
            flinkInsert.add( record );
        }
    }
}
