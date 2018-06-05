package com.sequoiadb.om.plugin.config;

public class PluginConfig {

    protected final String role = "plugin";
    protected String hostName;
    protected String name;
    protected String svcname;
    protected String type;
    protected boolean isRegister = false;

    public String getHostName() {
        return hostName;
    }

    public String getName() {
        return name;
    }

    public void setName(String name) {
        this.name = name;
    }

    public String getSvcname() {
        return svcname;
    }

    public String getRole() {
        return role;
    }

    public String getType() {
        return type;
    }

    public void setType(String type) {
        this.type = type;
    }

    public boolean getIsRegister(){ return isRegister; }

    public void setIsRegister(Boolean isRegister){ this.isRegister = isRegister; }
}
