package com.sequoias3.service;

import com.sequoias3.model.VersioningConfiguration;
import com.sequoias3.model.VersioningConfigurationNull;
import com.sequoias3.exception.S3ServerException;

public interface BucketVersioningService  {
    void putBucketVersioning(int ownerID, String bucketName, String status) throws S3ServerException;

    VersioningConfigurationNull getBucketVersioning(int ownerID, String bucketName) throws S3ServerException;
}
