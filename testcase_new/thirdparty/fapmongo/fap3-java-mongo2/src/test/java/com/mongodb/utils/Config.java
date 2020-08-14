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
    private String multiUrl;
    private String springDBName;
    private String javaDBName;
    private String username;
    private String password;

    public Config( String multiUrl, String springDBName, String javaDBName,
            String username, String password ) {
        this.multiUrl = multiUrl;
        this.springDBName = springDBName;
        this.javaDBName = javaDBName;
        this.username = username;
        this.password = password;
    }

    public void setDbname2( String dbname2 ) {
        this.javaDBName = dbname2;
    }

    public String getMultiUrl() {
        return multiUrl;
    }

    public void setMultiUrl( String multiUrl ) {
        this.multiUrl = multiUrl;
    }

    public void setJavaDBName( String javaDBName ) {
        this.javaDBName = javaDBName;
    }

    public String getJavaDBName() {
        return javaDBName;
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

    public String getSpringDBName() {
        return springDBName;
    }

    public void setSpringDBName( String springDBName ) {
        this.springDBName = springDBName;
    }

    public String getHost() {
        return getMultiUrl().split( "," )[ 0 ].split( ":" )[ 0 ];
    }

    public Integer getPort() {
        return Integer
                .parseInt( getMultiUrl().split( "," )[ 0 ].split( ":" )[ 1 ] );
    }

    public String[] getUrls() {
        return getMultiUrl().split( "," );
    }

    @Override
    public String toString() {
        return "Config{" + "multiUrl='" + multiUrl + '\'' + ", springDBName='"
                + springDBName + '\'' + ", javaDBName='" + javaDBName + '\''
                + ", username='" + username + '\'' + ", password='" + password
                + '\'' + '}';
    }
}
