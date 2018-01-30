package com.sequoiadb.plugin.dao;

import org.bson.BSONObject;
import org.bson.BasicBSONObject;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.stereotype.Component;

import java.sql.*;
import java.util.ArrayList;
import java.util.List;

@Component
public class SqlOperations {

    private final Logger logger = LoggerFactory.getLogger(SqlOperations.class);

    SqlOperations() throws ClassNotFoundException {
        Class.forName("org.postgresql.Driver");
    }

    public List<BSONObject> query(String hostName, String svcname,
                                  String user, String pwd,
                                  String dbName, String sql) throws Exception {
        Connection c = null;
        Statement stmt = null;
        ResultSet rs = null;
        List<BSONObject> content = new ArrayList<BSONObject>();

        try {
            c = DriverManager.getConnection("jdbc:postgresql://" + hostName + ":" + svcname + "/" + dbName, user, pwd);

            stmt = c.createStatement();

            if (sql.toLowerCase().indexOf("select") == 0) {
                rs = stmt.executeQuery(sql);
                for (int i = 0; i < 100 && rs.next(); ++i) {
                    content.add(resultSet2Bson(rs));
                }
            } else {
                stmt.executeUpdate(sql);
            }
        } catch (Exception e) {
            throw e;
        } finally {
            resultSetClose(rs);
            statementClose(stmt);
            ConnectionClose(c);
        }
        return content;
    }

    private void resultSetClose(ResultSet rs) {
        try {
            if (rs != null) {
                rs.close();
            }
        } catch (Exception e) {
            logger.warn(e.getMessage());
        }
    }

    private void statementClose(Statement stmt) {
        try {
            if (stmt != null) {
                stmt.close();
            }
        } catch (Exception e) {
            logger.warn(e.getMessage());
        }
    }

    private void ConnectionClose(Connection c) {
        try {
            if (c != null) {
                c.close();
            }
        } catch (Exception e) {
            logger.warn(e.getMessage());
        }
    }

    private BSONObject resultSet2Bson(ResultSet rs) throws SQLException {
        BSONObject bson = new BasicBSONObject();
        ResultSetMetaData metaData = rs.getMetaData();
        int columnCount = metaData.getColumnCount();

        for (int i = 1; i <= columnCount; i++) {
            String columnName = metaData.getColumnLabel(i);
            String value = rs.getString(columnName);
            bson.put(columnName, value);
        }

        return bson;
    }
}
