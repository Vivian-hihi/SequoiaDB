package com.sequoiadb.testcommon.warpper;

import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.testcommon.FlinkTestBase;
import com.sequoiadb.testcommon.utils.JDBCAttribute;
import com.sequoiadb.testcommon.utils.SDBAttribute;
import org.apache.flink.api.common.RuntimeExecutionMode;
import org.apache.flink.streaming.api.environment.StreamExecutionEnvironment;
import org.apache.flink.table.api.Schema;
import org.apache.flink.table.api.TableDescriptor;
import org.apache.flink.table.api.TableResult;
import org.apache.flink.table.api.bridge.java.StreamTableEnvironment;
import org.apache.flink.types.Row;
import org.testng.Assert;

import java.sql.SQLException;
import java.util.ArrayList;
import java.util.List;

public class StreamTableEnvWarpperImpl extends FlinkTestBase
        implements StreamTableEnvWarpper {
    private StreamTableEnvironment tableEnv;
    private String mysqlUrl;

    public StreamTableEnvWarpperImpl( RuntimeExecutionMode mode ) {
        StreamExecutionEnvironment env = StreamExecutionEnvironment
                .getExecutionEnvironment();
        env.setRuntimeMode( mode );
        tableEnv = StreamTableEnvironment.create( env );
        mysqlUrl = String.format(
                "jdbc:mysql://%s?user=%s&password=%s&useUnicode=true&useSSL=false&characterEncoding=utf-8&autoReconnect=true",
                FlinkTestBase.getMysqlAddress(),
                FlinkTestBase.getMysqlUsername(),
                FlinkTestBase.getMysqlPassword() );
        createFlinkDatabase();
    }

    @Override
    public void createTable( String tableName, Schema schema,
            String mappingCSName, String mappingClName )
            throws SQLException, ClassNotFoundException {
        createMysqlSourseTable( mappingCSName, mappingClName, schema );
        tableEnv.useDatabase( FlinkTestBase.jdbc_test_database_name );
        tableEnv.createTable( tableName, createJDBCTableDescriptor( schema,
                mappingCSName, mappingClName ) );
        tableEnv.useDatabase( FlinkTestBase.sdb_test_database_name );
        tableEnv.createTable( tableName, createSequoiaDBTableDescriptor( schema,
                mappingCSName, mappingClName ) );
    }

    @Override
    public TableResultWarpper executeSql( String sql ) throws Exception {
        tableEnv.useDatabase( FlinkTestBase.sdb_test_database_name );
        TableResult sdbTableResult = tableEnv.executeSql( sql );
        tableEnv.useDatabase( FlinkTestBase.jdbc_test_database_name );
        TableResult jdbcTableResult = tableEnv.executeSql( sql );
        return new TableResultWarpper( sdbTableResult, jdbcTableResult );
    }

    @Override
    public StreamTableEnvironment getStreamTableEnvironment() {
        return tableEnv;
    }

    @Override
    public void dropTable( String tableName ) {
        StringBuilder sql = new StringBuilder( "drop table if exists " );
        tableEnv.useDatabase( FlinkTestBase.sdb_test_database_name );
        tableEnv.executeSql( sql + tableName );

        tableEnv.useDatabase( FlinkTestBase.jdbc_test_database_name );
        tableEnv.executeSql( sql + tableName );
    }

    @Override
    public void clearSourceDatabase( String databaseName )
            throws SQLException, ClassNotFoundException {
        Sequoiadb sdb = new Sequoiadb( FlinkTestBase.getCoord(),
                FlinkTestBase.username, FlinkTestBase.password );
        if ( sdb.isCollectionSpaceExist( databaseName ) ) {
            sdb.dropCollectionSpace( databaseName );
        }
        String sql = "drop database if exists " + databaseName;
        JdbcConnect.update( mysqlUrl, sql );
    }

    @Override
    public void assertTableDataNoOrderWithSql( String sql ) throws Exception {
        TableResultWarpper tableResultWarpper = executeSql( sql );
        ArrayList< Row > jdbcResult = tableResultWarpper.getJdbcResultToList();
        ArrayList< Row > sdbResult = tableResultWarpper.getSdbResultToList();
        Assert.assertEqualsNoOrder( jdbcResult.toArray(), sdbResult.toArray(),
                "sdbResults is " + sdbResult + " jdbcResult is " + jdbcResult );
    }

    @Override
    public void assertTableDataWithSql( String sql ) throws Exception {
        TableResultWarpper tableResultWarpper = executeSql( sql );
        ArrayList< Row > jdbcResult = tableResultWarpper.getJdbcResultToList();
        ArrayList< Row > sdbResult = tableResultWarpper.getSdbResultToList();
        Assert.assertEquals( jdbcResult.toArray(), sdbResult.toArray(),
                "sdbResults is " + sdbResult + " jdbcResult is " + jdbcResult );
    }

    @Override
    public void assertTableData( String tableName ) throws Exception {
        String sql = "select * from " + tableName;
        assertTableDataNoOrderWithSql( sql );
    }

    @Override
    public void assertTableSchema( String tableName ) throws Exception {
        String sql = "show create table " + tableName;
        TableResultWarpper result = executeSql( sql );
        String sdbResult = result.getSdbResultToString();
        String jdbcResult = result.getJdbcResultToString();
        String sdbTableSchema = sdbResult.substring(
                sdbResult.indexOf( FlinkTestBase.sdb_test_database_name )
                        + FlinkTestBase.sdb_test_database_name.length(),
                sdbResult.indexOf( "WITH" ) );
        String jdbcTableSchema = jdbcResult.substring(
                jdbcResult.indexOf( FlinkTestBase.jdbc_test_database_name )
                        + FlinkTestBase.jdbc_test_database_name.length(),
                jdbcResult.indexOf( "WITH" ) );
        if ( !( sdbTableSchema.equals( jdbcTableSchema ) ) ) {
            throw new Exception( "sdbTableSchema is " + sdbTableSchema
                    + " jdbcTableSchema is " + jdbcTableSchema );
        }
    }

    private void createFlinkDatabase() {
        StringBuilder sql = new StringBuilder(
                "create database if not exists " );
        tableEnv.executeSql( sql + FlinkTestBase.sdb_test_database_name );
        tableEnv.executeSql( sql + FlinkTestBase.jdbc_test_database_name );
    }

    private TableDescriptor createSequoiaDBTableDescriptor( Schema schema,
            String mappingCSName, String mappingClName ) {
        return TableDescriptor.forConnector( SDBAttribute.sequoiadb )
                .schema( schema )
                .option( SDBAttribute.hosts, FlinkTestBase.getCoord() )
                .option( SDBAttribute.collectionspace, mappingCSName )
                .option( SDBAttribute.collection, mappingClName )
                .option( SDBAttribute.username, FlinkTestBase.username )
                .option( SDBAttribute.password, FlinkTestBase.password )
                .build();
    }

    private TableDescriptor createJDBCTableDescriptor( Schema schema,
            String mappingCSName, String mappingClName ) {
        return TableDescriptor.forConnector( JDBCAttribute.jdbc )
                .schema( schema )
                .option( JDBCAttribute.url, getJdbcConnectUrl( mappingCSName ) )
                .option( JDBCAttribute.table_name, mappingClName )
                .option( JDBCAttribute.username,
                        FlinkTestBase.getMysqlUsername() )
                .option( JDBCAttribute.password,
                        FlinkTestBase.getMysqlPassword() )
                .build();
    }

    private String getJdbcConnectUrl( String databaseName ) {
        String urlbase = "jdbc:mysql://%s/%s";
        return String.format( urlbase, FlinkTestBase.getMysqlAddress(),
                databaseName );
    }

    private void createMysqlSourseTable( String mappingCSName,
            String mappingClName, Schema schema )
            throws SQLException, ClassNotFoundException {
        String resolvedSchema = resolvedSchema( schema );
        List< String > sqlBatch = new ArrayList<>();
        sqlBatch.add( "create database if not exists " + mappingCSName );
        sqlBatch.add( "use " + mappingCSName );
        sqlBatch.add( "drop table if exists " + mappingClName );
        sqlBatch.add( "create table " + mappingClName + " " + resolvedSchema );
        JdbcConnect.updateBranch( mysqlUrl, sqlBatch );
    }

    private String resolvedSchema( Schema schema ) {
        List< Schema.UnresolvedColumn > columns = schema.getColumns();
        String resolvedSchema = columns.toString();
        resolvedSchema = resolvedSchema.replace( "[", "(" );
        resolvedSchema = resolvedSchema.replace( "TIMESTAMP", "DATETIME" );
        resolvedSchema = resolvedSchema.replace( "BYTES", "BINARY" );
        resolvedSchema = resolvedSchema.replace( "]", ")" );
        return resolvedSchema;
    }
}
