package com.sequoiadb.om.plugin.dao;

import org.springframework.stereotype.Component;

@Component
public class PostgreSQLOperations extends SequoiaSQLOperations {

    PostgreSQLOperations() throws ClassNotFoundException {
        className = "org.postgresql.Driver";
        scheme = "jdbc:postgresql";
        defaultDBName = "postgres";
    }
}
