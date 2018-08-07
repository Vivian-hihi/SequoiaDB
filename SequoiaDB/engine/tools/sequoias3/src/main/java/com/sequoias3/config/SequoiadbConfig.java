package com.sequoias3.config;

import org.springframework.boot.context.properties.ConfigurationProperties;
import org.springframework.stereotype.Component;

import java.util.ArrayList;
import java.util.List;

@Component
@ConfigurationProperties(prefix = "sdbs3.sequoiadb")
public class SequoiadbConfig {
    String url;
    List<String> urlList;
    SequoiadbMetaConfig meta;
    SequoiadbDataConfig data;

    public String getMetaCsName() {
        return meta.getCsName();
    }

    public String getMetaClName() {
        return meta.getClName();
    }

    public String getDataCsName() {
        return data.getCsName();
    }

    public String getDataArchiverate() {
        return data.getArchiverate();
    }

    public String getUrl() {
        return url;
    }

    public void setUrl(String url) {
        this.url = url;

        urlList = new ArrayList<>();
        int index = url.indexOf("://");
        String[] ipPorts = url.substring(index + 3).split(",");
        for (int i = 0; i < ipPorts.length; i++) {
            urlList.add(ipPorts[i]);
        }
    }

    public List<String> getUrlList() {
        return urlList;
    }

    public SequoiadbMetaConfig getMeta() {
        return meta;
    }

    public void setMeta(SequoiadbMetaConfig meta) {
        this.meta = meta;
    }

    public SequoiadbDataConfig getData() {
        return data;
    }

    public void setData(SequoiadbDataConfig data) {
        this.data = data;
    }
}
