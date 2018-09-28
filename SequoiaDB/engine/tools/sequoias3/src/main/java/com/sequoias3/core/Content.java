package com.sequoias3.core;

import com.fasterxml.jackson.annotation.JsonProperty;

public class Content {
    @JsonProperty("Key")
    private String key;
    @JsonProperty("LastModified")
    private String lastModified;
    @JsonProperty("ETag")
    private String eTag;
    @JsonProperty("Size")
    private long   size;
    @JsonProperty("Owner")
    private Owner owner;

    public void setKey(String key) {
        this.key = key;
    }

    public String getKey() {
        return key;
    }

    public void setLastModified(String lastModified) {
        this.lastModified = lastModified;
    }

    public String getLastModified() {
        return lastModified;
    }

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

    public void setOwner(Owner owner) {
        this.owner = owner;
    }

    public Owner getOwner() {
        return owner;
    }
}
