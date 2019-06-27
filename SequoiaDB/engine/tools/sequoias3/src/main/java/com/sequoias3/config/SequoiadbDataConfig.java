package com.sequoias3.config;

public class SequoiadbDataConfig {
    String csName;
    String domain;
    int lobPageSize;
    int replSize;

    public String getCsName() {
        return csName;
    }

    public void setCsName(String csName) {
        this.csName = csName;
    }

    public void setDomain(String domain) {
        this.domain = domain;
    }

    public String getDomain() {
        return domain;
    }

    public int getLobPageSize() {
        return lobPageSize;
    }

    public void setLobPageSize(int lobPageSize) {
        this.lobPageSize = lobPageSize;
    }

    public void setReplSize(int replSize) {
        this.replSize = replSize;
    }

    public int getReplSize() {
        return replSize;
    }
}
