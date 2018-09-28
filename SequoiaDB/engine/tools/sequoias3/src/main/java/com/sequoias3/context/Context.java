package com.sequoias3.context;

import com.sequoiadb.base.DBCursor;
import com.sequoiadb.base.Sequoiadb;
import com.sequoias3.core.QueryDbCursor;

import java.util.Date;

import static com.sequoias3.utils.DataFormatUtils.formatDate;

public class Context {
    private String        token;
    private QueryDbCursor dbCursor;
    private long          bucketId;
    private String        prefix;
    private String        startAfter;
    private String        delimiter;
    private long          lastModified;
    private String        lastCommonPrefix;

    public Context(String token, long bucketId){
        this.token = token;
        this.bucketId = bucketId;
    }

    public void setToken(String token) {
        this.token = token;
    }

    public String getToken() {
        return token;
    }

    public void setDbCursor(QueryDbCursor dbCursor) {
        this.dbCursor = dbCursor;
    }

    public QueryDbCursor getDbCursor() {
        return dbCursor;
    }

    public void setPrefix(String prefix) {
        this.prefix = prefix;
    }

    public String getPrefix() {
        return prefix;
    }

    public void setDelimiter(String delimiter) {
        this.delimiter = delimiter;
    }

    public String getDelimiter() {
        return delimiter;
    }

    public void setBucketId(long bucketId) {
        this.bucketId = bucketId;
    }

    public long getBucketId() {
        return bucketId;
    }

    public void setStartAfter(String startAfter) {
        this.startAfter = startAfter;
    }

    public String getStartAfter() {
        return startAfter;
    }

    public void setLastModified(long lastModified) {
        this.lastModified = lastModified;
    }

    public long getLastModified() {
        return lastModified;
    }

    public void setLastCommonPrefix(String lastCommonPrefix) {
        this.lastCommonPrefix = lastCommonPrefix;
    }

    public String getLastCommonPrefix() {
        return lastCommonPrefix;
    }

    public String toString(){
        return "context{" + "token:" +token + ", bucketId:" + bucketId +
                ", prefix:" + prefix + ", delimiter;" + delimiter + ", startafter:" + startAfter +
                "lastModified" + formatDate(lastModified) + "}";
    }
}
