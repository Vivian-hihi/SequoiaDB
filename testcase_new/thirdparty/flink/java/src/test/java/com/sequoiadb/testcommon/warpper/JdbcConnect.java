package com.sequoiadb.testcommon.warpper;

import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.SQLException;
import java.sql.Statement;
import java.util.List;

public class JdbcConnect {
    public static void update( String url, String sql )
            throws ClassNotFoundException, SQLException {
        Class.forName( "com.mysql.cj.jdbc.Driver" );
        Connection conn = DriverManager.getConnection( url );
        Statement statement = null;
        try {
            statement = conn.createStatement();
            statement.executeUpdate( sql );
        } finally {
            if ( statement != null ) {
                statement.close();
            }
            if ( conn != null ) {
                conn.close();
            }
        }
    }

    public static void updateBranch( String url, List< String > sqls )
            throws SQLException, ClassNotFoundException {
        Class.forName( "com.mysql.cj.jdbc.Driver" );
        Connection conn = DriverManager.getConnection( url );
        Statement statement = conn.createStatement();
        try {
            for ( int i = 0; i < sqls.size(); i++ ) {
                statement.addBatch( sqls.get( i ) );
            }
            statement.executeBatch();
        } finally {
            if ( statement != null ) {
                statement.close();
            }
            conn.close();
        }
    }
}
