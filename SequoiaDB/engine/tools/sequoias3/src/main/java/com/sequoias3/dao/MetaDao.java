package com.sequoias3.dao;

import com.sequoias3.core.ObjectMeta;
import com.sequoias3.exception.S3ServerException;
import org.bson.BSONObject;

public interface MetaDao {
    void insertMeta(ConnectionDao connectionDao, String metaCsName, String metaClName,
                    ObjectMeta object, int isIgnoreDup, Boolean isHistory)
            throws S3ServerException;

    QueryDbCursor queryMetaByBucket(String metaCsName, String metaClName, long bucketId,
                                    String prefix, String startAfter, Boolean specifiedVId,
                                    Boolean isIncludeDeleteMarker)
            throws S3ServerException;

    ObjectMeta queryMetaByObjectName(String metaCsName, String metaClName,
                                     long bucketId, String objectName, Long versionId,
                                     Boolean noVersionFlag)
            throws S3ServerException;

    ObjectMeta queryAndRemoveMeta(String metaCsName, String metaClName, long bucketId,
                                  String objectName)
            throws S3ServerException;

    ObjectMeta queryForUpdate(ConnectionDao connectionDao, String metaCsName, String metaClName,
                              long bucketId, String objectName, Long versionId, Boolean noVersionFlag)
            throws S3ServerException;

    void updateMeta(ConnectionDao connectionDao, String metaCsName, String metaClName, long bucketId,
                    String objectName, Long versionId, ObjectMeta object)
            throws S3ServerException;

    void removeMeta(ConnectionDao connectionDao, String metaCsName, String metaClName, long bucketId,
                    String objectName, Long versionId, Boolean noVersionFlag)
            throws S3ServerException;

    long getObjectNumber(String metaCsName, String metaClName, long bucketId)
            throws S3ServerException;

    String getMetaCSName( String region);

    String getMetaCurCLName();

    String getMetaHistoryCLName();

    void releaseQueryDbCursor(QueryDbCursor queryDbCursor);
}
