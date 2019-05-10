package com.sequoias3.config;

public class SequoiadbDataConfig {
    String csName;
    String domain;
    Integer lobPageSize;
    Integer replSize;

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

    public Integer getLobPageSize() {
        return lobPageSize;
    }

    public void setLobPageSize(Integer lobPageSize) {
        this.lobPageSize = lobPageSize;
    }

    public void setReplSize(Integer replSize) {
        this.replSize = replSize;
    }

    public Integer getReplSize() {
        return replSize;
    }
}
