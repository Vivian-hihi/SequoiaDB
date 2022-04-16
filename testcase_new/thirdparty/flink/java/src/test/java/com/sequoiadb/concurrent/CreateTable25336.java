package com.sequoiadb.concurrent;

import com.sequoiadb.base.CollectionSpace;
import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.exception.SDBError;
import com.sequoiadb.threadexecutor.ThreadExecutor;
import com.sequoiadb.threadexecutor.annotation.ExecuteOrder;
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
 * @descreption seqDB-25336:flink插入数据时，SDB创建集合
 * @author YiPan
 * @date 2022/2/18
 * @updateUser
 * @updateDate
 * @updateRemark
 * @version 1.0
 */
public class CreateTable25336 extends FlinkTestBase {
    private Sequoiadb sdb;
    private final String csName = "cs_25336";
    private final String clName = "cl_25336";
    private final String filed_int = "test_int";
    private final String filed_String = "test_string";
    private final String tableName = "tb_25336";
    private BSONObject bson;

    @BeforeClass
    public void setUp() {
        sdb = new Sequoiadb( FlinkTestBase.getCoord(), FlinkTestBase.username,
                FlinkTestBase.password );
        Commlib.dropCS( sdb, csName );
        bson = new BasicBSONObject();
        bson.put( filed_int, 1 );
        bson.put( filed_String, "abc" );
    }

    @Test
    public void test() throws Exception {
        // 并发插入
        ThreadExecutor t = new ThreadExecutor();
        t.addWorker( new CreateCl() );
        t.addWorker( new InsertData() );
        t.run();

        // 查询结果
        CollectionSpace cs = sdb.getCollectionSpace( csName );
        DBCollection cl = cs.getCollection( clName );
        BSONObject result = cl.queryOne();
        String ignoreId = "_id";
        result.removeField( ignoreId );
        bson.removeField( ignoreId );
        Assert.assertEquals( result, bson );
    }

    @AfterClass
    public void tearDown() {
        try {
            sdb.dropCollectionSpace( csName );
        } finally {
            sdb.close();
        }
    }

    private class InsertData {

        @ExecuteOrder(step = 1)
        private void run() throws Exception {
            StreamExecutionEnvironment env = StreamExecutionEnvironment
                    .getExecutionEnvironment();
            StreamTableEnvironment tableEnv = StreamTableEnvironment.create( env );
            Schema schema = Schema.newBuilder()
                    .column( filed_int, DataTypes.INT() )
                    .column( filed_String, DataTypes.STRING() ).build();
            TableDescriptor tableDescriptor = Commlib
                    .createTableDescriptor( schema, csName, clName );
            tableEnv.createTable( tableName, tableDescriptor );
            String sql = "insert into %s  values(%d,'%s')";
            TableResult tableResult = tableEnv.executeSql(String.format(sql, tableName,
                    bson.get(filed_int), bson.get(filed_String)));
            Commlib.waitJobFinish(tableResult);
        }
    }

    private class CreateCl {
        @ExecuteOrder(step = 2)
        private void run() {
            try {
                CollectionSpace cs = sdb.createCollectionSpace( csName );
                cs.createCollection( clName );
            } catch ( BaseException e ) {
                String errorType = e.getErrorType();
                if ( errorType.equals( SDBError.SDB_DMS_CS_EXIST.getErrorType() )
                        || errorType
                                .equals( SDBError.SDB_DMS_EXIST.getErrorType() ) ) {
                    return;
                }
                throw e;
            }
        }
    }
}
