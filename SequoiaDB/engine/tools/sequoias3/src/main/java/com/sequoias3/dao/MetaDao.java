package com.sequoias3.dao;

import com.sequoias3.context.Context;
import com.sequoias3.core.ObjectMeta;
import com.sequoias3.core.QueryDbCursor;
import com.sequoias3.exception.S3ServerException;

public interface MetaDao {
    String insertMeta(String metaCsName, String metaClName,
                            ObjectMeta object, String objectName)
            throws S3ServerException;

    QueryDbCursor queryMetaByBucket(String metaCsName, String metaClName, long bucketId,
                                    String prefix, String startAfter)
            throws S3ServerException;

    void releaseDBAndCursor(QueryDbCursor dbCursor);

    ObjectMeta queryMetaByObjectName(String metaCsName, String metaClName,
                                     long bucketId, String objectName)
            throws S3ServerException;

    ObjectMeta queryAndUpdateMeta(String metaCsName, String metaClName,
                                  long bucketId, ObjectMeta object, String objectName)
            throws S3ServerException;

    ObjectMeta queryAndRemoveMeta(String metaCsName, String metaClName, long bucketId,
                                  String objectName)
            throws S3ServerException;

    long getObjectNumber(String metaCsName, String metaClName, long bucketId)
            throws S3ServerException;

    String getMetaCSName( String region);

    String getMetaCLName();

    String getMetaCurCLName();

    String getMetaHistoryCLName();
}
