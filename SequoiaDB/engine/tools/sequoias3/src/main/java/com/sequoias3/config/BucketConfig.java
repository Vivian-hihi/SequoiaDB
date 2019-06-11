package com.sequoias3.config;

import org.springframework.boot.context.properties.ConfigurationProperties;
import org.springframework.stereotype.Component;

@Component
@ConfigurationProperties(prefix = "sdbs3.bucket")
public class BucketConfig {
    private int limit;
    private Boolean allowreput;

    public void setLimit(int limit){
        this.limit = limit;
    }

    public int getLimit(){
        return limit;
    }

    public void setAllowreput(Boolean allowreput) {
        this.allowreput = allowreput;
    }

    public Boolean getAllowreput() {
        return allowreput;
    }
}
