package com.sequoias3.model;

import com.fasterxml.jackson.annotation.JsonIgnore;
import com.fasterxml.jackson.annotation.JsonProperty;

public class RawVersion {
    @JsonProperty("Key")
    private String key;
    @JsonProperty("VersionId")
    private String versionId;
    @JsonProperty("IsLatest")
    private Boolean isLatest;
    @JsonProperty("LastModified")
    private String lastModified;
    @JsonProperty("Owner")
    private Owner owner;

    @JsonIgnore
    private Boolean noVersionFlag;

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

    public void setIsLatest(Boolean latest) {
        this.isLatest = latest;
    }

    public Boolean getIsLatest() {
        return this.isLatest;
    }

    public void setVersionId(String versionId) {
        this.versionId = versionId;
    }

    public String getVersionId() {
        return versionId;
    }

    public void setOwner(Owner owner) {
        this.owner = owner;
    }

    public Owner getOwner() {
        return owner;
    }

    public void setNoVersionFlag(Boolean noVersionFlag) {
        this.noVersionFlag = noVersionFlag;
    }

    public Boolean getNoVersionFlag() {
        return noVersionFlag;
    }
}
