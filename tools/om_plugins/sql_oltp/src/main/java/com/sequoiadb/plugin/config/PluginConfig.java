package com.sequoiadb.plugin.config;

import org.springframework.boot.context.embedded.EmbeddedServletContainerInitializedEvent;
import org.springframework.context.ApplicationListener;
import org.springframework.stereotype.Component;

import java.net.InetAddress;
import java.net.UnknownHostException;

@Component
public class PluginConfig implements ApplicationListener<EmbeddedServletContainerInitializedEvent> {

    private final String role = "plugin";
    private String hostName;
    private String name;
    private String svcname;
    private String type;

    @Override
    public void onApplicationEvent(EmbeddedServletContainerInitializedEvent event) {
        svcname = Integer.toString(event.getEmbeddedServletContainer().getPort());
    }

    PluginConfig() throws UnknownHostException {
        InetAddress addr = InetAddress.getLocalHost();
        hostName = addr.getHostName().toString();
    }

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

}

