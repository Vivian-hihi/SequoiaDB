package com.sequoias3.core;

import com.fasterxml.jackson.annotation.JsonIgnore;
import com.fasterxml.jackson.annotation.JsonProperty;

public class Bucket {
    public static final String BUCKET_ID               = "ID";
    public static final String BUCKET_NAME             = "Name";
    public static final String BUCKET_OWNERID          = "OwnerID";
    public static final String BUCKET_CREATETIME       = "CreationDate";
    public static final String BUCKET_VERSIONINGSTATUS = "VersioningStatus";
    public static final String BUCKET_DELIMITER        = "Delimiter";
    public static final String BUCKET_REGION           = "Region";

    @JsonIgnore
    private long    bucketId;

    @JsonProperty(BUCKET_NAME)
    private String bucketName;

    @JsonIgnore
    private int    ownerId;

    @JsonProperty(BUCKET_CREATETIME)
    private String formatDate;

    @JsonIgnore
    private long timeMillis;

    @JsonIgnore
    private String versioningStatus;

    @JsonIgnore
    private String delimiter;

    @JsonIgnore
    private String region;

    public void setBucketId(long bucketId){
        this.bucketId = bucketId;
    }

    public long getBucketId(){
        return this.bucketId;
    }

    public void setBucketName(String bucketName){
        this.bucketName = bucketName;
    }

    public String getBucketName(){
        return this.bucketName;
    }

    public void setOwnerId(int ownerId){
        this.ownerId = ownerId;
    }

    public int getOwnerId(){
        return this.ownerId;
    }

    public void setFormatDate(String creationDate){
        this.formatDate = creationDate;
    }

    public String getFormatDate(){
        return this.formatDate;
    }

    public void setTimeMillis(long timeMillis) {
        this.timeMillis = timeMillis;
    }

    public long getTimeMillis() {
        return timeMillis;
    }

    public void setVersioningStatus(String versioningStatus){
        this.versioningStatus = versioningStatus;
    }

    public String getVersioningStatus(){
        return this.versioningStatus;
    }

    public void setDelimiter(String delimiter){
        this.delimiter = delimiter;
    }

    public String getDelimiter(){
        return this.delimiter;
    }

    public void setRegion(String region) {
        this.region = region;
    }

    public String getRegion() {
        return region;
    }
}
