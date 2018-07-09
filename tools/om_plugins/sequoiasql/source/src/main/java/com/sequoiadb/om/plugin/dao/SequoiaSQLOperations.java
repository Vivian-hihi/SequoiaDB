package com.sequoiadb.om.plugin.dao;

import org.bson.BSONObject;
import org.bson.BasicBSONObject;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.sql.*;
import java.util.ArrayList;
import java.util.List;

public abstract class SequoiaSQLOperations {

    protected final Logger logger = LoggerFactory.getLogger(PostgreSQLOperations.class);
    protected String className = "";
    protected String scheme = "";
    protected String defaultDBName = "";
    protected String defaultUser = "";

    public List<BSONObject> query(String hostName, String svcname,
                                  String user, String pwd,
                                  String dbName, String sql) throws Exception {
        Connection c = null;
        Statement stmt = null;
        ResultSet rs = null;
        List<BSONObject> content = new ArrayList<BSONObject>();

        Class.forName(className);

        if (dbName == null || dbName.trim().length() == 0) {
            dbName = defaultDBName;
        }

        try {
            c = DriverManager.getConnection(scheme + "://" + hostName + ":" + svcname + "/" + dbName, user, pwd);

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

    public String getDefaultUser(){
        return defaultUser;
    }

    protected void resultSetClose(ResultSet rs) {
        try {
            if (rs != null) {
                rs.close();
            }
        } catch (Exception e) {
            logger.warn(e.getMessage());
        }
    }

    protected void statementClose(Statement stmt) {
        try {
            if (stmt != null) {
                stmt.close();
            }
        } catch (Exception e) {
            logger.warn(e.getMessage());
        }
    }

    protected void ConnectionClose(Connection c) {
        try {
            if (c != null) {
                c.close();
            }
        } catch (Exception e) {
            logger.warn(e.getMessage());
        }
    }

    protected BSONObject resultSet2Bson(ResultSet rs) throws SQLException {
        BSONObject bson = new BasicBSONObject();
        ResultSetMetaData metaData = rs.getMetaData();
        int columnCount = metaData.getColumnCount();

        for (int i = 1; i <= columnCount; i++) {
            String columnName = metaData.getColumnLabel(i);
            String value = rs.getString(i);
            bson.put(columnName, value);
        }

        return bson;
    }
}