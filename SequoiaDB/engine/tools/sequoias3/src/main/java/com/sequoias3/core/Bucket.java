package com.sequoias3.core;

public class Bucket {
    public static final String BUCKET_ID               = "ID";
    public static final String BUCKET_NAME             = "Name";
    public static final String BUCKET_OWNERID          = "OwnerID";
    public static final String BUCKET_CREATETIME       = "CreateDate";
    public static final String BUCKET_REGION           = "Region";
    public static final String BUCKET_VERSIONINGSTATUS = "VersioningStatus";
    public static final String BUCKET_DELIMITER        = "Delimiter";
    private int    bucketId;
    private String bucketName;
    private int    ownerId;
    private String createDate;
    private String region;
    private String versioningStatus;
    private String delimiter;

    public void setBucketId(int bucketId){
        this.bucketId = bucketId;
    }

    public int getBucketId(){
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

    public void setBucketCreateDate(String createDate){
        this.createDate = createDate;
    }

    public String getCreateDate(){
        return this.createDate;
    }

    public void setRegion(String region){
        this.region = region;
    }

    public String getRegion(){
        return region;
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
}
