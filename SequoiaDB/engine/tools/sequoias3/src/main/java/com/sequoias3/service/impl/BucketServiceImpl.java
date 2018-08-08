package com.sequoias3.service.impl;

import com.sequoias3.core.Bucket;
import com.sequoias3.exception.S3ServerException;
import com.sequoias3.service.BucketService;

public class BucketServiceImpl implements BucketService {
    @Override
    public void createBucket(int ownerID, String bucketName, String region) throws S3ServerException {

    }

    @Override
    public void deleteBucket(int ownerID, String bucketName) throws S3ServerException {

    }

    @Override
    public void deleteBucketForce(int ownerID, String bucketName) throws S3ServerException {

    }

    @Override
    public Bucket[] getService(int ownerID) throws S3ServerException {
        return new Bucket[0];
    }
}
