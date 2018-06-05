package com.sequoiadb.om.plugin.config;

import org.springframework.boot.context.embedded.EmbeddedServletContainerInitializedEvent;
import org.springframework.context.ApplicationListener;
import org.springframework.stereotype.Component;

import java.net.InetAddress;
import java.net.UnknownHostException;

@Component
public class PostgreSQLConfig extends PluginConfig implements ApplicationListener<EmbeddedServletContainerInitializedEvent> {

    @Override
    public void onApplicationEvent(EmbeddedServletContainerInitializedEvent event) {
        svcname = Integer.toString(event.getEmbeddedServletContainer().getPort());
    }

    PostgreSQLConfig() throws UnknownHostException {
        InetAddress addr = InetAddress.getLocalHost();
        hostName = addr.getHostName().toString();
    }
}