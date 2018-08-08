package com.sequoias3.dao;

import com.sequoias3.core.Bucket;
import com.sequoias3.exception.S3ServerException;

public interface BucketDao {
    void insertBucket(Bucket bucket) throws S3ServerException;

    void deleteBucket(String bucketName) throws S3ServerException;

    Bucket[] getBucketListByOwnerID(int ownerId) throws S3ServerException;
}
