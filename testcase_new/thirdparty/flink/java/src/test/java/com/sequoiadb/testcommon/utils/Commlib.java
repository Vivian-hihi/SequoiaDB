package com.sequoiadb.testcommon.utils;

import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.testcommon.FlinkTestBase;
import org.apache.flink.api.common.JobStatus;
import org.apache.flink.core.execution.JobClient;
import org.apache.flink.table.api.Schema;
import org.apache.flink.table.api.TableDescriptor;
import org.apache.flink.table.api.TableResult;
import org.apache.flink.types.Row;
import org.apache.flink.types.RowKind;
import org.apache.flink.util.CloseableIterator;

import java.util.*;

public class Commlib extends FlinkTestBase {
    /**
     * @descreption 创建出包含sdb连接器必填参数的TableDescriptor
     * @param schema
     * @param mappingCSName
     * @param mappingClName
     * @return
     */
    public static TableDescriptor createTableDescriptor( Schema schema,
            String mappingCSName, String mappingClName ) {
        return createBaseTableOption( schema, mappingCSName, mappingClName )
                .build();
    }

    /**
     * @descreption 创建出包含sdb连接器必填参数的TableDescriptor.Builder,可追加参数
     * @param schema
     * @param mappingCSName
     * @param mappingClName
     * @return
     */
    public static TableDescriptor.Builder createBaseTableOption( Schema schema,
            String mappingCSName, String mappingClName ) {
        return TableDescriptor.forConnector( SDBAttribute.sequoiadb )
                .schema( schema )
                .option( SDBAttribute.hosts, FlinkTestBase.getCoord() )
                .option( SDBAttribute.collectionspace, mappingCSName )
                .option( SDBAttribute.collection, mappingClName )
                .option( SDBAttribute.username, FlinkTestBase.username )
                .option( SDBAttribute.password, FlinkTestBase.password );

    }

    /**
     * @descreption 将结果收集到ArrayList中，移除删除记录和被删除记录
     * @param collect
     * @return
     */
    public static ArrayList< Row > collectToArrayList(
            CloseableIterator< Row > collect ) {
        ArrayList< Row > InsertResult = new ArrayList<>();
        ArrayList< Row > DeleteResult = new ArrayList<>();
        while ( collect.hasNext() ) {
            Row row = collect.next();
            if ( row.getKind().equals( RowKind.INSERT ) ) {
                InsertResult.add( row );
            } else if ( row.getKind().equals( RowKind.DELETE ) ) {
                row.setKind( RowKind.INSERT );
                DeleteResult.add( row );
            } else {
                // TODO: 在流式数据场景下，需要处理UPDATE后才可用
                InsertResult.add( row );
            }
        }
        InsertResult.removeAll( DeleteResult );
        return InsertResult;
    }

    /**
     * @descreption 将结果转换为String
     * @param collect
     * @return
     */
    public static String collectToString( CloseableIterator< Row > collect ) {
        StringBuilder result = new StringBuilder();
        while ( collect.hasNext() ) {
            result.append( collect.next() );
        }
        return result.toString();
    }

    /**
     * @descreption 等待job执行完成，默认30s
     * @param tableResult
     * @throws Exception
     */
    public static void waitJobFinish( TableResult tableResult )
            throws Exception {
        waitJobFinish( tableResult, 30000 );
    }

    /**
     * @descreption 等待job执行完成
     * @param tableResult
     * @param timeOut
     * @throws Exception
     */
    public static void waitJobFinish( TableResult tableResult, int timeOut )
            throws Exception {
        waitJobStatus( tableResult, JobStatus.FINISHED, timeOut );
    }

    /**
     * @descreption 等待job进入指定状态
     * @param tableResult
     * @param status
     * @param timeOut
     * @throws Exception
     */
    public static void waitJobStatus( TableResult tableResult, JobStatus status,
            int timeOut ) throws Exception {
        JobClient jobClient = tableResult.getJobClient().get();
        for ( int i = 0; i < timeOut / 1000; i++ ) {
            JobStatus jobStatus = jobClient.getJobStatus().get();
            if ( jobStatus.equals( status ) ) {
                return;
            }
            Thread.sleep( 1000 );
        }
        throw new Exception( "wait flink job " + status + " time out, jobId:"
                + jobClient.getJobID() + " ,jobStatus:"
                + jobClient.getJobStatus().get() );
    }

    /**
     * @descreption drop cs if exists
     * @param sdb
     * @param csName
     */
    public static void dropCS( Sequoiadb sdb, String csName ) {
        if ( sdb.isCollectionSpaceExist( csName ) ) {
            sdb.dropCollectionSpace( csName );
        }
    }
}
