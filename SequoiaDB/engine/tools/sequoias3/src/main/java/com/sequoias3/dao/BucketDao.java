package com.sequoias3.dao;

import com.sequoias3.core.Bucket;
import com.sequoias3.exception.S3ServerException;

import java.text.ParseException;
import java.util.ArrayList;
import java.util.List;

public interface BucketDao {
    void insertBucket(Bucket bucket) throws S3ServerException,ParseException;

    void deleteBucket(String bucketName) throws S3ServerException;

    Bucket getBucketByName(String bucketName) throws S3ServerException;

    List<Bucket> getBucketListByOwnerID(int ownerId) throws S3ServerException;

    long getMaxID() throws S3ServerException;

    long getBucketNumber(int ownerID) throws S3ServerException;
}
