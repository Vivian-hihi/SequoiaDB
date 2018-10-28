package com.sequoias3.model;

import com.fasterxml.jackson.annotation.JsonProperty;

public class Version extends RawVersion{
    @JsonProperty("ETag")
    private String eTag;
    @JsonProperty("Size")
    private long   size;

    public void seteTag(String eTag) {
        this.eTag = eTag;
    }

    public String geteTag() {
        return eTag;
    }

    public void setSize(long size) {
        this.size = size;
    }

    public long getSize() {
        return size;
    }
}
