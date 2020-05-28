package com.mongodb.utils;

import org.springframework.stereotype.Component;

/**
 * @Description 配置类
 * @author fanyu
 * @version 1.00
 * @Date 2020/4/22
 */
@Component
public class Config {
    private String host;
    private Integer port;
    private String dbname1;
    private String dbname2;
    private String username;
    private String password;

    public Config( String host, Integer port, String dbname1, String dbname2,
            String username, String password ) {
        this.host = host;
        this.port = port;
        this.dbname1 = dbname1;
        this.dbname2 = dbname2;
        this.username = username;
        this.password = password;
    }

    public void setDbname2( String dbname2 ) {
        this.dbname2 = dbname2;
    }

    public String getHost() {
        return host;
    }

    public void setHost( String host ) {
        this.host = host;
    }

    public Integer getPort() {
        return port;
    }

    public void setPort( Integer port ) {
        this.port = port;
    }

    public String getDbName() {
        return dbname2;
    }

    public String getUsername() {
        return username;
    }

    public void setUsername( String username ) {
        this.username = username;
    }

    public String getPassword() {
        return password;
    }

    public void setPassword( String password ) {
        this.password = password;
    }

    public String getDbname1() {
        return dbname1;
    }

    public void setDbname1( String dbname1 ) {
        this.dbname1 = dbname1;
    }

    @Override
    public String toString() {
        return "Config{" + "host='" + host + '\'' + ", port=" + port
                + ", dbname1='" + dbname1 + '\'' + ", dbname2='" + dbname2
                + '\'' + ", username='" + username + '\'' + ", password='"
                + password + '\'' + '}';
    }
}
