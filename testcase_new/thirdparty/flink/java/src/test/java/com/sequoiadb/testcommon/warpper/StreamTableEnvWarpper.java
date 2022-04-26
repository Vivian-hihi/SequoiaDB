package com.sequoiadb.testcommon.warpper;

import org.apache.flink.api.common.RuntimeExecutionMode;
import org.apache.flink.table.api.Schema;
import org.apache.flink.table.api.bridge.java.StreamTableEnvironment;

import java.sql.SQLException;

public interface StreamTableEnvWarpper {

    static StreamTableEnvWarpperImpl create() {
        return new StreamTableEnvWarpperImpl( RuntimeExecutionMode.STREAMING );
    }

    static StreamTableEnvWarpperImpl create( RuntimeExecutionMode mode ) {
        return new StreamTableEnvWarpperImpl( mode );
    }

    void createTable( String tableName, Schema schema, String mappingCSName,
            String mappingClName ) throws SQLException, ClassNotFoundException;

    TableResultWarpper executeSql( String sql ) throws Exception;

    StreamTableEnvironment getStreamTableEnvironment();

    void dropTable( String tableName );

    void clearSourceDatabase( String databaseName )
            throws SQLException, ClassNotFoundException;

    void assertTableDataNoOrderWithSql( String sql ) throws Exception;

    void assertTableDataWithSql( String sql ) throws Exception;

    void assertTableData( String tableName ) throws Exception;

    void assertTableSchema( String tableName ) throws Exception;

}
