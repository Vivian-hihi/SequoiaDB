package com.sequoias3.service;

import com.sequoias3.core.*;
import com.sequoias3.exception.S3ServerException;
import com.sequoias3.model.ListObjectsResult;
import com.sequoias3.model.ListVersionsResult;
import com.sequoias3.model.PutDeleteResult;

import java.io.InputStream;
import java.io.OutputStream;
import java.util.Map;

public interface ObjectService {
    PutDeleteResult putObject(int ownerID, String bucketName, String objectName,
                              String contentMD5, Map<String, String> requestHeaders,
                              Map<String, String> xMeta, InputStream inputStream)
            throws S3ServerException;

    ObjectMeta getObject(int ownerID, String bucketName, String objectName,
                         Long versionId, Boolean isNoVersion, Map matchers, Range range, OutputStream outputStream)
            throws S3ServerException;

    PutDeleteResult deleteObject(int ownerID, String bucketName, String objectName)
            throws S3ServerException;

    PutDeleteResult deleteObject(int ownerID, String bucketName,
                                              String objectName, Long versionId, Boolean isNoVersion)
            throws S3ServerException;

    ListObjectsResult listObjects(int ownerID, String bucketName, String prefix,
                                  String delimiter, String startAfter, Integer maxKeys,
                                  String continueToken, String encodingType, Boolean fetchOwner)
            throws S3ServerException;

    ListVersionsResult listVersions(int ownerID, String bucketName, String prefix,
                                    String delimiter, String keyMarker, String versionIdMarker,
                                    Integer maxKeys, String encodingType)
            throws S3ServerException;

    long getObjectNumberByBucketId(Bucket bucket) throws S3ServerException;

    void deleteObjectByBucket(Bucket bucket) throws S3ServerException;
}
