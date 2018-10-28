package com.sequoias3.model;

public class PutDeleteResult {
    private String  eTag;
    private Long  versionId;
    private Boolean deleteMarker;

    public void seteTag(String eTag) {
        this.eTag = eTag;
    }

    public String geteTag() {
        return eTag;
    }

    public void setVersionId(Long versionId) {
        this.versionId = versionId;
    }

    public Long getVersionId() {
        return versionId;
    }

    public void setDeleteMarker(Boolean deleteMarker) {
        this.deleteMarker = deleteMarker;
    }

    public Boolean getDeleteMarker(){
        return this.deleteMarker;
    }
}
