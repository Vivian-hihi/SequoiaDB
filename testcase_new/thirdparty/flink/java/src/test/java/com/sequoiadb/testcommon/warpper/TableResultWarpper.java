package com.sequoiadb.testcommon.warpper;

import com.sequoiadb.testcommon.utils.Commlib;
import org.apache.flink.api.common.JobStatus;
import org.apache.flink.core.execution.JobClient;
import org.apache.flink.table.api.TableResult;
import org.apache.flink.types.Row;

import java.util.ArrayList;

public class TableResultWarpper {
    private ArrayList< Row > sdbResult;
    private ArrayList< Row > jdbcResult;
    private JobClient sdbJobClient = null;
    private JobClient jdbcJobClient = null;

    public TableResultWarpper( TableResult sdbTableResult,
            TableResult jdbcTableResult ) throws Exception {
        if ( sdbTableResult.getJobClient().isPresent() ) {
            this.sdbJobClient = sdbTableResult.getJobClient().get();
        }
        if ( jdbcTableResult.getJobClient().isPresent() ) {
            this.jdbcJobClient = jdbcTableResult.getJobClient().get();
        }
        this.sdbResult = Commlib.collectToArrayList( sdbTableResult.collect() );
        this.jdbcResult = Commlib
                .collectToArrayList( jdbcTableResult.collect() );
    }

    public void waitJobFinish() throws Exception {
        if ( sdbJobClient == null || jdbcJobClient == null ) {
            throw new Exception( "no jobClient create" );
        }
        waitJobStatus( jdbcJobClient, JobStatus.FINISHED, 20000 );
        waitJobStatus( sdbJobClient, JobStatus.FINISHED, 20000 );
    }

    public ArrayList< Row > getSdbResultToList() {
        return sdbResult;
    }

    public ArrayList< Row > getJdbcResultToList() {
        return jdbcResult;
    }

    public String getSdbResultToString() {
        return sdbResult.toString();
    }

    public String getJdbcResultToString() {
        return jdbcResult.toString();
    }

    private void waitJobStatus( JobClient jobClient, JobStatus status,
            int timeOut ) throws Exception {
        for ( int i = 0; i < timeOut / 1000; i++ ) {
            JobStatus jobStatus = jobClient.getJobStatus().get();
            if ( jobStatus.equals( status ) ) {
                return;
            }
            Thread.sleep( 1000 );
        }
        throw new Exception( "wait flink job " + status + " time out, jobId:"
                + jobClient.getJobID() );
    }
}
