package com.sequoias3.service;

import com.sequoias3.core.Bucket;
import com.sequoias3.exception.S3ServerException;

public interface BucketService {

    void createBucket(int ownerID, String bucketName, String region) throws S3ServerException;

    void deleteBucket(int ownerID, String bucketName) throws S3ServerException;

    void deleteBucketForce(int ownerID, String bucketName) throws S3ServerException;

    Bucket[] getService(int ownerID) throws S3ServerException;
}
