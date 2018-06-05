package com.sequoiadb.om.plugin.dao;

import org.springframework.stereotype.Component;

@Component
public class MySQLOperations extends SequoiaSQLOperations {

    MySQLOperations() throws ClassNotFoundException {
        className = "com.mysql.jdbc.Driver";
        scheme = "jdbc:mysql";
        defaultDBName = "mysql";
    }
}
