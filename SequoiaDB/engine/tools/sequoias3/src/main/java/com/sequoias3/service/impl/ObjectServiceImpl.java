package com.sequoias3.service.impl;

import com.sequoias3.common.DBParamDefine;
import com.sequoias3.common.RestParamDefine;
import com.sequoias3.common.VersioningStatusType;
import com.sequoias3.context.Context;
import com.sequoias3.context.ContextManager;
import com.sequoias3.core.*;
import com.sequoias3.model.*;
import com.sequoias3.model.Content;
import com.sequoias3.model.ListObjectsResult;
import com.sequoias3.model.Owner;
import com.sequoias3.model.PutDeleteResult;
import com.sequoias3.dao.*;
import com.sequoias3.exception.S3Error;
import com.sequoias3.exception.S3ServerException;
import com.sequoias3.service.BucketService;
import com.sequoias3.service.ObjectService;
import org.apache.commons.codec.binary.Hex;
import org.bson.BSONObject;
import org.bson.types.ObjectId;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;
import sun.misc.BASE64Decoder;

import javax.servlet.ServletOutputStream;
import java.io.InputStream;
import java.io.UnsupportedEncodingException;
import java.net.URLEncoder;
import java.util.*;

import static com.sequoias3.utils.DataFormatUtils.formatDate;
import static com.sequoias3.utils.DataFormatUtils.parseDate;

@Service
public class ObjectServiceImpl implements ObjectService {
    private static final Logger logger = LoggerFactory.getLogger(ObjectServiceImpl.class);

    @Autowired
    BucketDao bucketDao;

    @Autowired
    BucketService bucketService;

    @Autowired
    UserDao userDao;

    @Autowired
    DataDao dataDao;

    @Autowired
    MetaDao metaDao;

    @Autowired
    ContextManager contextManager;

    @Autowired
    DaoMgr daoMgr;

    @Autowired
    Transaction transaction;

    @Override
    public PutDeleteResult putObject(int ownerID, String bucketName, String objectName,
                                     String contentMD5, Map<String, String> headers,
                                     Map<String, String> xMeta, InputStream inputStream)
            throws S3ServerException {
        //check key length
        if (objectName.length() > RestParamDefine.KEY_LENGTH){
            throw new S3ServerException(S3Error.OBJECT_KEY_TOO_LONG,
                    "ObjectName is too long. objectName:"+objectName);
        }

        //check meta length
        if (xMeta.toString().length() > RestParamDefine.X_AMZ_META_LENGTH){
            throw new S3ServerException(S3Error.OBJECT_METADATA_TOO_LARGE,
                    "metadata headers exceed the maximum. xMeta:"+xMeta.toString());
        }

        //get and check bucket
        Bucket bucket = bucketService.getBucket(ownerID, bucketName);

        //get cs and cl
        Date createDate      = new Date();
        String dataCsName    = dataDao.getDataCSName(bucket.getRegion(), createDate);
        String dataClName    = dataDao.getDataClName();
        String metaCsName    = metaDao.getMetaCSName(bucket.getRegion());
        String metaClName    = metaDao.getMetaCurCLName();

        DataAttr insertResult;
        try {//insert lob
            insertResult = dataDao.insertObjectData(dataCsName, dataClName, inputStream);
        }catch (S3ServerException e){
            throw e;
        }catch (Exception e){
            throw new S3ServerException(S3Error.OBJECT_PUT_fAILED, "put object failed.", e);
        }

        try {
            //check md5
            if (null != contentMD5) {
                if (!isMd5EqualWithETag(contentMD5, insertResult.geteTag())) {
                    throw new S3ServerException(S3Error.OBJECT_BAD_DIGEST,
                            "The Content-MD5 you specified does not match what we received.");
                }
            }

            VersioningStatusType versioningStatusType = VersioningStatusType.getVersioningStatus(bucket.getVersioningStatus());

            //build meta
            ObjectMeta objectMeta = buildObjectMeta(objectName, bucket.getBucketId(),
                    headers, xMeta, dataCsName, dataClName, false,
                    generateNoVersionFlag(versioningStatusType));
            objectMeta.seteTag(insertResult.geteTag());
            objectMeta.setSize(insertResult.getSize());
            objectMeta.setLobId(insertResult.getLobId());

            writeObjectMeta(metaCsName, metaClName, objectMeta,
                    objectName, bucket.getBucketId(), versioningStatusType);

            //build response
            PutDeleteResult response = new PutDeleteResult();
            response.seteTag(insertResult.geteTag());
            if (VersioningStatusType.ENABLED == versioningStatusType){
                response.setVersionId(String.valueOf(objectMeta.getVersionId()));
            }else if(VersioningStatusType.SUSPENDED == versioningStatusType){
                response.setVersionId(ObjectMeta.NULL_VERSION_ID);
            }
            return response;
        }catch (S3ServerException e){
            dataDao.deleteObjectDataByLobId(null, dataCsName,
                    dataClName, insertResult.getLobId());
            if (e.getError().getErrIndex() == S3Error.DAO_DUPLICATE_KEY.getErrIndex()) {
                throw new S3ServerException(S3Error.OBJECT_PUT_fAILED,
                        "bucket+key duplicate too times. bucket:"+bucketName+" key:"+objectName);
            } else {
                throw e;
            }
        }catch (Exception e){
            dataDao.deleteObjectDataByLobId(null, dataCsName,
                    dataClName, insertResult.getLobId());
            throw new S3ServerException(S3Error.OBJECT_PUT_fAILED, "put object failed.", e);
        }
    }

    @Override
    public GetResult getObject(int ownerID, String bucketName, String objectName,
                                Long versionId, Boolean isNoVersion, Map headers,
                                Range range)
            throws S3ServerException {
        try {
            Bucket bucket = bucketService.getBucket(ownerID, bucketName);

            String metaCsName    = metaDao.getMetaCSName(bucket.getRegion());
            String metaClName    = metaDao.getMetaCurCLName();
            String metaHisClName = metaDao.getMetaHistoryCLName();

            int tryTime = DBParamDefine.DB_DUPLICATE_MAX_TIME;
            while (tryTime > 0) {
                tryTime--;
                try {
                    ObjectMeta versionIdMeta;
                    ObjectMeta objectMeta = metaDao.queryMetaByObjectName(metaCsName, metaClName,
                            bucket.getBucketId(), objectName, null, null);
                    if (null == objectMeta) {
                        if (versionId != null || isNoVersion != null){
                            throw new S3ServerException(S3Error.OBJECT_NO_SUCH_VERSION,
                                    "no such version. object:" + objectName + ",version:" + versionId);
                        }else {
                            throw new S3ServerException(S3Error.OBJECT_NO_SUCH_KEY, "no object. object:" + objectName);
                        }
                    }

                    if (versionId != null) {
                        if (versionId == objectMeta.getVersionId() && !objectMeta.getNoVersionFlag()){
                            versionIdMeta = objectMeta;
                        }else {
                            ObjectMeta objectMetaHis = metaDao.queryMetaByObjectName(metaCsName,
                                    metaHisClName, bucket.getBucketId(), objectName, versionId, false);
                            if (null == objectMetaHis) {
                                throw new S3ServerException(S3Error.OBJECT_NO_SUCH_VERSION,
                                        "no such version. object:" + objectName + ",version:" + versionId);
                            }
                            versionIdMeta = objectMetaHis;
                        }
                    }else if(isNoVersion != null) {
                        if (objectMeta.getNoVersionFlag()) {
                            versionIdMeta = objectMeta;
                        } else {
                            ObjectMeta objectMetaHis = metaDao.queryMetaByObjectName(metaCsName,
                                    metaHisClName, bucket.getBucketId(), objectName, null, true);
                            if (null == objectMetaHis) {
                                throw new S3ServerException(S3Error.OBJECT_NO_SUCH_VERSION,
                                        "no such version. object:" + objectName + ",version:" + versionId);
                            }
                            versionIdMeta = objectMetaHis;
                        }
                    }else{
                        versionIdMeta = objectMeta;
                    }

                    DataLob dataLob = null;
                    if (!versionIdMeta.getDeleteMarker()){
                        checkMatchModify(headers, versionIdMeta);
                        dataLob = dataDao.getDataLobForRead(versionIdMeta.getCsName(), versionIdMeta.getClName(),
                                versionIdMeta.getLobId());
                        try {
                            analyseRangeWithLob(range, dataLob);
                        }catch (Exception e){
                            dataDao.releaseDataLob(dataLob);
                            throw e;
                        }
                    }
                    return new GetResult(versionIdMeta, dataLob);
                }catch (S3ServerException e){
                    if (e.getError() == S3Error.DAO_LOB_FNE){
                        continue;
                    }else{
                        throw e;
                    }
                }
            }
            throw new S3ServerException(S3Error.OBJECT_NO_SUCH_KEY,
                    "Lob is not find");
        }catch (S3ServerException e) {
            throw e;
        } catch (Exception e) {
            throw new S3ServerException(S3Error.OBJECT_GET_FAILED,
                    "get object failed. bucket:" + bucketName + ", object=" + objectName, e);
        }
    }

    @Override
    public void readObjectData(DataLob data, ServletOutputStream outputStream, Range range)
            throws S3ServerException{
        if (data == null || outputStream == null){
            throw new S3ServerException(S3Error.OBJECT_GET_FAILED,
                    "get object data failed. ");
        }
        try{
            data.read(outputStream, range);
        }catch (S3ServerException e){
            throw e;
        } catch (Exception e) {
            throw new S3ServerException(S3Error.OBJECT_GET_FAILED,
                    "get object data failed. ");
        }
    }

    @Override
    public void releaseGetResult(GetResult result){
        dataDao.releaseDataLob(result.getData());
    }

    @Override
    public PutDeleteResult deleteObject(int ownerID, String bucketName, String objectName)
            throws S3ServerException {
        try {
            Bucket bucket = bucketService.getBucket(ownerID, bucketName);

            String metaCsName    = metaDao.getMetaCSName(bucket.getRegion());
            String metaClName    = metaDao.getMetaCurCLName();

            VersioningStatusType versioningStatusType = VersioningStatusType.getVersioningStatus(bucket.getVersioningStatus());
            Boolean noVersionFlag = generateNoVersionFlag(versioningStatusType);

            PutDeleteResult response = null;
            switch (versioningStatusType) {
                case NONE:
                    ObjectMeta objectMeta = metaDao.queryAndRemoveMeta(metaCsName, metaClName,
                            bucket.getBucketId(), objectName);
                    deleteObjectLob(objectMeta);
                    return null;
                case SUSPENDED:
                case ENABLED:
                    ObjectMeta deleteMarker = buildObjectMeta(objectName,
                            bucket.getBucketId(), null, null,
                            null, null, true,
                            noVersionFlag);
                    writeObjectMeta(metaCsName, metaClName, deleteMarker,
                                    objectName, bucket.getBucketId(), versioningStatusType);

                    response = new PutDeleteResult();
                    if (deleteMarker.getNoVersionFlag()){
                        response.setVersionId(ObjectMeta.NULL_VERSION_ID);
                    }else {
                        response.setVersionId(String.valueOf(deleteMarker.getVersionId()));
                    }
                    response.setDeleteMarker(true);
                    break;
                default:
                    break;
            }
            return response;
        }catch (S3ServerException e) {
            if (e.getError().getErrIndex() == S3Error.DAO_DUPLICATE_KEY.getErrIndex()) {
                throw new S3ServerException(S3Error.OBJECT_DELETE_FAILED,
                        "bucket+key duplicate too times. bucket:"+bucketName+" key:"+objectName);
            } else {
                throw e;
            }
        } catch (Exception e) {
            throw new S3ServerException(S3Error.OBJECT_DELETE_FAILED,
                    "delete object failed. bucket:" + bucketName + ", object=" + objectName, e);
        }
    }

    @Override
    public PutDeleteResult deleteObject(int ownerID, String bucketName, String objectName,
                                        Long versionId, Boolean isNoVersion)
            throws S3ServerException {
        try {
            Bucket bucket = bucketService.getBucket(ownerID, bucketName);

            String metaCsName    = metaDao.getMetaCSName(bucket.getRegion());
            String metaClName    = metaDao.getMetaCurCLName();
            String metaHisClName = metaDao.getMetaHistoryCLName();

            VersioningStatusType versioningStatusType = VersioningStatusType.getVersioningStatus(bucket.getVersioningStatus());
            ObjectMeta deleteObject = null;
            switch (versioningStatusType){
                case NONE:
                    if (isNoVersion) {
                        ObjectMeta objectMeta = metaDao.queryAndRemoveMeta(metaCsName, metaClName,
                                bucket.getBucketId(), objectName);
                        deleteObject = objectMeta;
                    }
                    break;
                case SUSPENDED:
                case ENABLED:
                    ConnectionDao connection = daoMgr.getConnectionDao();
                    transaction.begin(connection);
                    try{
                        ObjectMeta objectMeta = null;
                        if (isNoVersion != null){
                            objectMeta = metaDao.queryForUpdate(connection, metaCsName,
                                    metaClName, bucket.getBucketId(), objectName, null, true);
                        }else if (versionId != null){
                            objectMeta = metaDao.queryForUpdate(connection, metaCsName,
                                    metaClName, bucket.getBucketId(), objectName, versionId, false);
                        }
                        if (objectMeta != null){
                            deleteObject = objectMeta;
                            ObjectMeta objectMeta1 = metaDao.queryForUpdate(connection, metaCsName,
                                    metaHisClName, bucket.getBucketId(), objectName, null, null);
                            if (objectMeta1 != null){
                                metaDao.updateMeta(connection, metaCsName, metaClName, bucket.getBucketId(),
                                        objectName, versionId, objectMeta1);
                                metaDao.removeMeta(connection, metaCsName, metaHisClName, bucket.getBucketId(),
                                        objectName, objectMeta1.getVersionId(), null);
                            }else {
                                metaDao.removeMeta(connection, metaCsName, metaClName, bucket.getBucketId(),
                                        objectName, versionId, null);
                            }
                        }else{
                            ObjectMeta objectMeta2 = null;
                            if (isNoVersion != null){
                                objectMeta2 = metaDao.queryForUpdate(connection, metaCsName,
                                        metaHisClName, bucket.getBucketId(), objectName, null, true);
                            }else if (versionId != null){
                                objectMeta2 = metaDao.queryForUpdate(connection, metaCsName,
                                        metaHisClName, bucket.getBucketId(), objectName, versionId, false);
                            }

                            if (objectMeta2 != null){
                                deleteObject = objectMeta2;
                                metaDao.removeMeta(connection, metaCsName, metaHisClName, bucket.getBucketId(),
                                        objectName, versionId, null);
                            }
                        }
                        transaction.commit(connection);
                    }catch(Exception e){
                        transaction.rollback(connection);
                        throw e;
                    } finally {
                        daoMgr.releaseConnectionDao(connection);
                    }
                    break;
                default:
                    break;
            }

            deleteObjectLob(deleteObject);
            return null;
        }catch (S3ServerException e) {
            throw e;
        } catch (Exception e) {
            throw new S3ServerException(S3Error.OBJECT_DELETE_FAILED,
                    "delete object failed. bucket:" + bucketName + ", object=" + objectName, e);
        }
    }

    @Override
    public ListObjectsResult listObjects(int ownerID, String bucketName, String prefix,
                                         String delimiter, String startAfter, Integer maxKeys,
                                         String continueToken, String encodingType, Boolean fetchOwner)
            throws S3ServerException {
        Context queryContext = null;
        QueryDbCursor dbCursor = null;
        try {
            Bucket bucket = bucketService.getBucket(ownerID, bucketName);
            ListObjectsResult listObjectsResult = new ListObjectsResult(bucketName, maxKeys,
                    encodingType, prefix, startAfter, delimiter, continueToken);

            String metaCsName = metaDao.getMetaCSName(bucket.getRegion());
            String metaClName = metaDao.getMetaCurCLName();

            //get cursor
            if (null != continueToken) {
                queryContext = contextManager.get(continueToken);
                if (null == queryContext) {
                    throw new S3ServerException(S3Error.OBJECT_INVALID_TOKEN,
                            "The continuation token provided is incorrect.");
                }
                if (!IsContextMatch(queryContext, prefix, startAfter, delimiter)){
                    queryContext = null;
                }else{
                    dbCursor = metaDao.queryMetaByBucket(metaCsName, metaClName,
                            bucket.getBucketId(), prefix, queryContext.getLastKey(),
                            false, false);
                }
            }
            if (null == queryContext){
                dbCursor = metaDao.queryMetaByBucket(metaCsName, metaClName,
                        bucket.getBucketId(), prefix, startAfter,
                        false, false);
            }

            if (null == dbCursor) {
                return listObjectsResult;
            }

            Owner owner = null;
            if (fetchOwner){
                owner = userDao.getOwnerByUserID(ownerID);
            }

            int count = 0;
            int maxNumber = Math.min(maxKeys, RestParamDefine.MAX_KEYS_DEFAULT);
            int prefixLen = 0;
            if (null != prefix){
                prefixLen = prefix.length();
            }
            int delimiterLen = 0;
            if (null != delimiter) {
                delimiterLen = delimiter.length();
            }
            //no delimiter
            if (null == delimiter) {
                LinkedHashSet<Content> contentList = listObjectsResult.getContentList();
                while (dbCursor.hasNext() && count < maxNumber) {
                    BSONObject record = dbCursor.getNext();
                    Content content = convertBsonToContent(record, encodingType);
                    content.setOwner(owner);
                    contentList.add(content);
                    count++;
                }
            }else{
                LinkedHashSet<Content> contentList = listObjectsResult.getContentList();
                LinkedHashSet<CommonPrefix> commonPrefixesList = listObjectsResult.getCommonPrefixList();
                if (queryContext != null && queryContext.getLastCommonPrefix() != null) {
                    count = skipLastCommonPrefix(queryContext.getLastCommonPrefix(),
                            dbCursor, delimiter, prefixLen, encodingType, listObjectsResult, count, owner);
                    queryContext.setLastCommonPrefix(null);
                } else if (startAfter != null){
                    count = skipLastCommonPrefix(startAfter,
                            dbCursor, delimiter, prefixLen, encodingType, listObjectsResult, count, owner);
                }
                while (dbCursor.hasNext() && count < maxNumber) {
                    BSONObject record = dbCursor.getNext();
                    String key = record.get(ObjectMeta.META_KEY_NAME).toString();
                    int delimiterIndex = key.indexOf(delimiter, prefixLen);
                    if (-1 != delimiterIndex){
                        //commonprefix
                        CommonPrefix commonPrefix = new CommonPrefix(key.substring(0, delimiterIndex+delimiterLen), encodingType);
                        if (!commonPrefixesList.contains(commonPrefix)){
                            commonPrefixesList.add(commonPrefix);
                            count++;
                        }
                    }else{
                        //contents
                        Content content = convertBsonToContent(record, encodingType);
                        content.setOwner(owner);
                        contentList.add(content);
                        count++;
                    }
                }
            }
            listObjectsResult.setKeyCount(count);

            if (dbCursor.hasNext()) {
                BSONObject record = dbCursor.getCurrent();
                String key = record.get(ObjectMeta.META_KEY_NAME).toString();
                if (null == queryContext){
                    queryContext = contextManager.create(bucket.getBucketId());
                    queryContext.setPrefix(prefix);
                    queryContext.setStartAfter(startAfter);
                    queryContext.setDelimiter(delimiter);
                }

                queryContext.setLastKey(key);
                //record context
                if (null != delimiter) {
                    int delimiterIndex = key.indexOf(delimiter, prefixLen);
                    if (-1 != delimiterIndex) {
                        queryContext.setLastCommonPrefix(key.substring(0, delimiterIndex+delimiterLen));
                    }
                }
                listObjectsResult.setIsTruncated(true);
                listObjectsResult.setNextContinueToken(queryContext.getToken());
            } else {
                contextManager.release(queryContext);
            }
            return listObjectsResult;
        } catch (S3ServerException e){
            contextManager.release(queryContext);
            throw e;
        } catch (Exception e){
            contextManager.release(queryContext);
            throw new S3ServerException(S3Error.OBJECT_LIST_FAILED, "error message:"+e.getMessage());
        }finally {
            metaDao.releaseQueryDbCursor(dbCursor);
        }
    }

    @Override
    public ListVersionsResult listVersions(int ownerID, String bucketName, String prefix,
                                           String delimiter, String keyMarker, String versionIdMarker,
                                           Integer maxKeys, String encodingType)
            throws S3ServerException {
        QueryDbCursor queryDbCursorCur = null;
        QueryDbCursor queryDbCursorHis = null;
        try {
            Bucket bucket = bucketService.getBucket(ownerID, bucketName);
            ListVersionsResult listVersionsResult = new ListVersionsResult(bucketName, maxKeys,
                    encodingType, prefix, delimiter, keyMarker, versionIdMarker);

            String metaCsName    = metaDao.getMetaCSName(bucket.getRegion());
            String metaClName    = metaDao.getMetaCurCLName();
            String metaHisClName = metaDao.getMetaHistoryCLName();
            //get sdb and cursor
            Boolean isSpecifiedVId = false;
            Boolean isExistKeyVersion = false;
            if (versionIdMarker != null){
                isSpecifiedVId = true;
                isExistKeyVersion = isExistKeyVersion(metaCsName, metaClName, metaHisClName,
                        keyMarker, versionIdMarker,bucket);
            }
            queryDbCursorCur = metaDao.queryMetaByBucket(metaCsName, metaClName,
                    bucket.getBucketId(), prefix, keyMarker, isSpecifiedVId, true);
            if (queryDbCursorCur == null){
                return listVersionsResult;
            }

            queryDbCursorHis = metaDao.queryMetaByBucket(metaCsName, metaHisClName,
                    bucket.getBucketId(), prefix, keyMarker, isSpecifiedVId, true);
            if (queryDbCursorHis != null){
                if (queryDbCursorHis.hasNext()){
                    queryDbCursorHis.getNext();
                }else {
                    metaDao.releaseQueryDbCursor(queryDbCursorHis);
                    queryDbCursorHis = null;
                }
            }

            int count = 0;
            int maxNumber = Math.min(maxKeys, RestParamDefine.MAX_KEYS_DEFAULT);
            Owner owner = userDao.getOwnerByUserID(ownerID);
            int prefixLen = 0;
            if (null != prefix){
                prefixLen = prefix.length();
            }

            if (keyMarker != null){
                count = skipKeyMarker(queryDbCursorCur, queryDbCursorHis, keyMarker, versionIdMarker, listVersionsResult,
                        delimiter, maxNumber, encodingType, owner, prefixLen, isExistKeyVersion);
                if (count >= maxNumber){
                    return listVersionsResult;
                }
            }

            if (delimiter == null){
                recordVersionsWithNoDelimiter(queryDbCursorCur, queryDbCursorHis,
                        listVersionsResult, count, maxNumber, encodingType, owner);
            }else {
                recordVersionsWithDelimiter(queryDbCursorCur, queryDbCursorHis,
                        listVersionsResult, delimiter, count, maxNumber, prefixLen,
                        encodingType, owner);
            }

            return listVersionsResult;
        }catch (S3ServerException e){
            throw e;
        }catch (Exception e){
            throw new S3ServerException(S3Error.OBJECT_LIST_VERSIONS_FAILED,
                    "List versions failed. bucket:"+bucketName);
        }finally {
            metaDao.releaseQueryDbCursor(queryDbCursorCur);
            metaDao.releaseQueryDbCursor(queryDbCursorHis);
        }
    }

    @Override
    public long getObjectNumberByBucketId(Bucket bucket) throws S3ServerException{
        try {
            String metaCsName = metaDao.getMetaCSName(bucket.getRegion());
            String metaClName = metaDao.getMetaCurCLName();
            String metaHisClName = metaDao.getMetaHistoryCLName();

            long curCount = metaDao.getObjectNumber(metaCsName, metaClName, bucket.getBucketId());
            long hisCount = metaDao.getObjectNumber(metaCsName, metaHisClName, bucket.getBucketId());
            return curCount+hisCount;
        }catch (S3ServerException e){
            throw e;
        }catch (Exception e){
            throw new S3ServerException(S3Error.DAO_DB_ERROR, "unknown error", e);
        }
    }

    @Override
    public void deleteObjectByBucket(Bucket bucket) throws S3ServerException {
        try {
            String metaCsName    = metaDao.getMetaCSName(bucket.getRegion());
            String metaCurClName = metaDao.getMetaCurCLName();
            String metaHisClName = metaDao.getMetaHistoryCLName();
            long bucketId = bucket.getBucketId();

            deleteObjectByClBucket(metaCsName, metaHisClName, bucketId);
            deleteObjectByClBucket(metaCsName, metaCurClName, bucketId);
        }catch (S3ServerException e){
            throw e;
        }catch (Exception e){
            throw new S3ServerException(S3Error.DAO_DB_ERROR, "unknown error", e);
        }
    }

    private void deleteObjectByClBucket(String metaCsName, String metaClName, long bucketId)
            throws S3ServerException{
        QueryDbCursor queryDbCursorHis = metaDao.queryMetaByBucket(metaCsName, metaClName,
                bucketId, null, null, false, true);
        if (queryDbCursorHis == null){
            return;
        }
        ConnectionDao connection = daoMgr.getConnectionDao();
        try {
            while (queryDbCursorHis.hasNext()) {
                BSONObject record = queryDbCursorHis.getNext();
                String key = record.get(ObjectMeta.META_KEY_NAME).toString();
                Long versionId = (Long)record.get(ObjectMeta.META_VERSION_ID);
                metaDao.removeMeta(connection, metaCsName, metaClName, bucketId,
                        key, versionId, null);
                if (record.get(ObjectMeta.META_CS_NAME) != null
                        && record.get(ObjectMeta.META_CL_NAME) != null
                        && record.get(ObjectMeta.META_LOB_ID) != null) {
                    String dataCsName = record.get(ObjectMeta.META_CS_NAME).toString();
                    String dataClName = record.get(ObjectMeta.META_CL_NAME).toString();
                    ObjectId lobId = (ObjectId)record.get(ObjectMeta.META_LOB_ID);
                    dataDao.deleteObjectDataByLobId(connection, dataCsName, dataClName, lobId);
                }
            }
        } catch (Exception e) {
            throw e;
        }finally {
            daoMgr.releaseConnectionDao(connection);
        }
    }

    private void recordVersionsTruncated(ListVersionsResult listVersionsResult,
                                         String nextKey, String nextVersionId) {
        listVersionsResult.setIsTruncated(true);
        listVersionsResult.setNextVersionIdMarker(nextVersionId);
        listVersionsResult.setNextKeyMarker(nextKey);
    }

    private ObjectMeta buildObjectMeta(String objectName, long bucketId, Map headers,
                                       Map xMeta, String dataCsName, String dataClName,
                                       Boolean isDeleteMarker, Boolean noVersionFlag) {
        ObjectMeta objectMeta = new ObjectMeta();
        objectMeta.setKey(objectName);
        objectMeta.setBucketId(bucketId);
        objectMeta.setCsName(dataCsName);
        objectMeta.setClName(dataClName);
        objectMeta.setLastModified(System.currentTimeMillis());
        objectMeta.setMetaList(xMeta);
        objectMeta.setDeleteMarker(isDeleteMarker);
        objectMeta.setNoVersionFlag(noVersionFlag);

        if (headers != null) {
            if (headers.containsKey(RestParamDefine.PutObjectHeader.CACHE_CONTROL)) {
                objectMeta.setCacheControl(headers.get(RestParamDefine.PutObjectHeader.CACHE_CONTROL).toString());
            }

            if (headers.containsKey(RestParamDefine.PutObjectHeader.CONTENT_DISPOSITION)) {
                objectMeta.setContentDisposition(headers.get(RestParamDefine.PutObjectHeader.CONTENT_DISPOSITION).toString());
            }

            if (headers.containsKey(RestParamDefine.PutObjectHeader.CONTENT_ENCODING)) {
                objectMeta.setContentEncoding(headers.get(RestParamDefine.PutObjectHeader.CONTENT_ENCODING).toString());
            }

            if (headers.containsKey(RestParamDefine.PutObjectHeader.CONTENT_TYPE)) {
                objectMeta.setContentType(headers.get(RestParamDefine.PutObjectHeader.CONTENT_TYPE).toString());
            }

            if (headers.containsKey(RestParamDefine.PutObjectHeader.EXPIRES)) {
                objectMeta.setExpires(headers.get(RestParamDefine.PutObjectHeader.EXPIRES).toString());
            }

            if (headers.containsKey(RestParamDefine.PutObjectHeader.CONTENT_LANGUAGE)) {
                objectMeta.setContentLanguage(headers.get(RestParamDefine.PutObjectHeader.CONTENT_LANGUAGE).toString());
            }
        }

        return objectMeta;
    }

    private Content convertBsonToContent(BSONObject bsonObject, String encodingType) throws S3ServerException{
        try {
            Content content = new Content();
            if (null != encodingType) {
                content.setKey(URLEncoder.encode(bsonObject.get(ObjectMeta.META_KEY_NAME).toString(), "UTF-8"));
            } else {
                content.setKey(bsonObject.get(ObjectMeta.META_KEY_NAME).toString());
            }
            content.setLastModified(formatDate((long) bsonObject.get(ObjectMeta.META_LAST_MODIFIED)));
            content.seteTag(bsonObject.get(ObjectMeta.META_ETAG).toString());
            content.setSize((long) bsonObject.get(ObjectMeta.META_SIZE));
            return content;
        }catch (UnsupportedEncodingException e){
            logger.error("Encode object name failed. e", e);
            throw new S3ServerException(S3Error.UNKNOWN_ERROR,
                    "encode object name failed."+e.getMessage());
        }
    }

    private void recordVersionsWithDelimiter(QueryDbCursor queryDbCursorCur, QueryDbCursor queryDbCursorHis,
                                            ListVersionsResult listVersionsResult, String delimiter, int count,
                                            int maxNumber, int prefixLen, String encodingType, Owner owner)
            throws S3ServerException{
        int delimiterLen = delimiter.length();
        LinkedHashSet<CommonPrefix> commonPrefixesList = listVersionsResult.getCommonPrefixList();

        Cur:
        while(queryDbCursorCur.hasNext()){
            Boolean isCommonPrefix = false;
            CommonPrefix commonPrefix = null;
            BSONObject recordA = queryDbCursorCur.getNext();
            String keyA = recordA.get(ObjectMeta.META_KEY_NAME).toString();
            int delimiterIndex = keyA.indexOf(delimiter, prefixLen);
            if (-1 != delimiterIndex){
                isCommonPrefix = true;
                commonPrefix = new CommonPrefix(keyA.substring(0, delimiterIndex+delimiterLen), encodingType);
                if (!commonPrefixesList.contains(commonPrefix)){
                    commonPrefixesList.add(commonPrefix);
                    count++;
                }
            }else{
                recordVersionDeleteMarker(recordA, listVersionsResult,
                        encodingType, owner, true);
                count++;
            }

            if (count >= maxNumber) {
                if (isCommonPrefix && queryDbCursorCur.hasNext()){
                    recordVersionsTruncated(listVersionsResult, commonPrefix.getPrefix(), null);
                }else if (!isCommonPrefix && (queryDbCursorCur.hasNext() || queryDbCursorHis != null)){
                    Version version = convertBsonToVersion(recordA, encodingType);
                    recordVersionsTruncated(listVersionsResult, version.getKey(), version.getVersionId());
                }
                break;
            }

            while(queryDbCursorHis != null){
                BSONObject recordB = queryDbCursorHis.getCurrent();
                String keyB = recordB.get(ObjectMeta.META_KEY_NAME).toString();
                int result = keyB.compareTo(keyA);
                if (0 == result) {
                    if (!isCommonPrefix) {
                        recordVersionDeleteMarker(recordB, listVersionsResult,
                                encodingType, owner, false);
                        count++;
                    }
                }else if (result > 0){
                    break;
                }

                if (count >= maxNumber) {
                    if (queryDbCursorCur.hasNext() || queryDbCursorHis.hasNext()) {
                        Version version = convertBsonToVersion(recordB, encodingType);
                        recordVersionsTruncated(listVersionsResult, version.getKey(), version.getVersionId());
                    }
                    break Cur;
                }
                if (queryDbCursorHis.hasNext()){
                    queryDbCursorHis.getNext();
                }else {
                    metaDao.releaseQueryDbCursor(queryDbCursorHis);
                    queryDbCursorHis = null;
                }
            }
        }
    }

    private void recordVersionsWithNoDelimiter(QueryDbCursor queryDbCursorCur, QueryDbCursor queryDbCursorHis,
                                            ListVersionsResult listVersionsResult, int count, int maxNumber,
                                              String encodingType, Owner owner)
            throws S3ServerException{

        Cur:
        while(queryDbCursorCur.hasNext()){
            BSONObject recordA = queryDbCursorCur.getNext();
            String keyA = recordA.get(ObjectMeta.META_KEY_NAME).toString();
            recordVersionDeleteMarker(recordA, listVersionsResult,
                    encodingType, owner, true);
            count++;

            if (count >= maxNumber) {
                if (queryDbCursorCur.hasNext() || queryDbCursorHis != null){
                    Version version = convertBsonToVersion(recordA, encodingType);
                    recordVersionsTruncated(listVersionsResult, version.getKey(), version.getVersionId());
                }
                break;
            }

            while(queryDbCursorHis != null){
                BSONObject recordB = queryDbCursorHis.getCurrent();
                String keyB = recordB.get(ObjectMeta.META_KEY_NAME).toString();
                int result = keyB.compareTo(keyA);
                if (0 == result) {
                    recordVersionDeleteMarker(recordB, listVersionsResult,
                            encodingType, owner, false);
                    count++;
                }else if (result > 0){
                    break;
                }

                if (count >= maxNumber) {
                    if (queryDbCursorCur.hasNext() || queryDbCursorHis.hasNext()) {
                        Version version = convertBsonToVersion(recordB, encodingType);
                        recordVersionsTruncated(listVersionsResult, version.getKey(), version.getVersionId());
                    }
                    break Cur;
                }
                if (queryDbCursorHis.hasNext()){
                    queryDbCursorHis.getNext();
                }else {
                    metaDao.releaseQueryDbCursor(queryDbCursorHis);
                    queryDbCursorHis = null;
                }
            }
        }
    }

    private void recordVersionDeleteMarker(BSONObject bsonObject, ListVersionsResult result,
                                           String encodingType, Owner owner, Boolean isLatest)
            throws S3ServerException{
        Version version = convertBsonToVersion(bsonObject, encodingType);
        if ((Boolean)bsonObject.get(ObjectMeta.META_DELETE_MARKER)) {
            RawVersion deleteMarker = new RawVersion();
            deleteMarker.setKey(version.getKey());
            deleteMarker.setLastModified(version.getLastModified());
            deleteMarker.setVersionId(version.getVersionId());
            deleteMarker.setLatest(isLatest);
            deleteMarker.setOwner(owner);
            result.getDeleteMarkerList().add(deleteMarker);
        }else {
            version.setLatest(isLatest);
            version.setOwner(owner);
            result.getVersionList().add(version);
        }
    }

    private Version convertBsonToVersion(BSONObject bsonObject, String encodingType)throws S3ServerException{
        try{
            Version version = new Version();
            if (null != encodingType) {
                version.setKey(URLEncoder.encode(bsonObject.get(ObjectMeta.META_KEY_NAME).toString(), "UTF-8"));
            } else {
                version.setKey(bsonObject.get(ObjectMeta.META_KEY_NAME).toString());
            }
            version.setLastModified(formatDate((long) bsonObject.get(ObjectMeta.META_LAST_MODIFIED)));
            if ((Boolean)bsonObject.get(ObjectMeta.META_NO_VERSION_FLAG)) {
                version.setVersionId(ObjectMeta.NULL_VERSION_ID);
            }else{
                version.setVersionId(bsonObject.get(ObjectMeta.META_VERSION_ID).toString());
            }

            if (bsonObject.get(ObjectMeta.META_ETAG) != null) {
                version.seteTag(bsonObject.get(ObjectMeta.META_ETAG).toString());
            }
            if (bsonObject.get(ObjectMeta.META_SIZE) != null) {
                version.setSize((long) bsonObject.get(ObjectMeta.META_SIZE));
            }
            return version;
        }catch (UnsupportedEncodingException e){
            logger.error("Encode object name failed. e", e);
            throw new S3ServerException(S3Error.UNKNOWN_ERROR,
                    "encode object name failed."+e.getMessage());
        }
    }

    private Boolean isMd5EqualWithETag(String contentMd5, String eTag) throws S3ServerException{
        try {
            if(contentMd5.length() % 4 != 0){
                throw new S3ServerException(S3Error.OBJECT_INVALID_DIGEST,
                        "decode md5 failed, contentMd5:"+contentMd5);
            }
            BASE64Decoder decoder = new BASE64Decoder();
            String textMD5 = new String(Hex.encodeHex(decoder.decodeBuffer(contentMd5)));
            if (textMD5.equals(eTag)){
                return true;
            }else {
                return false;
            }
        }catch (Exception e){
            throw new S3ServerException(S3Error.OBJECT_INVALID_DIGEST,
                    "decode md5 failed, contentMd5:"+contentMd5);
        }
    }

    private Boolean checkMatchModify(Map headers, ObjectMeta objectMeta) throws S3ServerException{
        String eTag           = objectMeta.geteTag();
        long lastModifiedTime = objectMeta.getLastModified();
        boolean isMatch     = false;
        boolean isNoneMatch = false;

        Object matchEtag = headers.get(RestParamDefine.GetObjectReqHeader.REQ_IF_MATCH);
        if (null != matchEtag){
            if (!matchEtag.toString().equals(eTag)){
                throw new S3ServerException(S3Error.OBJECT_IF_MATCH_FAILED,
                        "if match isNotMatch: match eTag:" + matchEtag.toString() + ", etag:" + eTag);
            }else{
                isMatch = true;
            }
        }

        Object noneMatchEtag = headers.get(RestParamDefine.GetObjectReqHeader.REQ_IF_NONE_MATCH);
        if (null != noneMatchEtag){
            if (noneMatchEtag.toString().equals(eTag)){
                throw new S3ServerException(S3Error.OBJECT_IF_NONE_MATCH_FAILED,
                        "if none match isNotMatch: match eTag:" + noneMatchEtag.toString() + ", etag:" + eTag);
            }else{
                isNoneMatch = true;
            }
        }

        Object unModifiedSince = headers.get(RestParamDefine.GetObjectReqHeader.REQ_IF_UNMODIFIED_SINCE);
        if (null != unModifiedSince){
            Date date = parseDate(unModifiedSince.toString());
            if (date.getTime() < lastModifiedTime) {
                if (!isMatch) {
                    throw new S3ServerException(S3Error.OBJECT_IF_UNMODIFIED_SINCE_FAILED,
                            "if modified since date: last:" + date.getTime() +
                                    ", lastModifiedTime:" + lastModifiedTime);
                }
            }
        }

        Object modifiedSince = headers.get(RestParamDefine.GetObjectReqHeader.REQ_IF_MODIFIED_SINCE);
        if (null != modifiedSince){
            Date date = parseDate(modifiedSince.toString());
            if (date.getTime() >= lastModifiedTime) {
                if (!isNoneMatch) {
                    throw new S3ServerException(S3Error.OBJECT_IF_MODIFIED_SINCE_FAILED,
                            "if modified since date: last:" + date.getTime() +
                                    ", lastModifiedTime:" + lastModifiedTime);
                }
            }
        }

        return true;
    }

    private Boolean IsContextMatch(Context queryContext, String prefix, String startAfter,
                                 String delimiter){
        if(queryContext.getDelimiter() != null){
            if (!(queryContext.getDelimiter().equals(delimiter))) {
                return false;
            }
        }else if (delimiter != null){
            return false;
        }

        if (queryContext.getPrefix() != null) {
            if (!(queryContext.getPrefix().equals(prefix))) {
                return false;
            }
        }else if (prefix != null){
            return false;
        }

        if (queryContext.getStartAfter() != null) {
            if (!(queryContext.getStartAfter().equals(startAfter))) {
                return false;
            }
        }else if (startAfter != null){
            return false;
        }

        return true;
    }

    private int skipLastCommonPrefix(String lastCommonPrefix, QueryDbCursor dbCursor,
                                      String delimiter, int prefixLen, String encodingType,
                                      ListObjectsResult listObjectsResult, int count, Owner owner)
            throws S3ServerException{
        int delimiterLen = 0;
        if (null != delimiter){
            delimiterLen = delimiter.length();
        }
        LinkedHashSet<Content> contentList = listObjectsResult.getContentList();
        LinkedHashSet<CommonPrefix> commonPrefixesList = listObjectsResult.getCommonPrefixList();
        while (dbCursor.hasNext()) {
            BSONObject record = dbCursor.getNext();
            String key = record.get(ObjectMeta.META_KEY_NAME).toString();
            int delimiterIndex = key.indexOf(delimiter, prefixLen);
            if (-1 != delimiterIndex) {
                String keyCommonPrefix = key.substring(0, delimiterIndex + delimiterLen);
                int comResult = keyCommonPrefix.compareTo(lastCommonPrefix);
                if (comResult <= 0) {
                    continue;
                }
                CommonPrefix commonPrefix = new CommonPrefix(keyCommonPrefix, encodingType);
                if (!commonPrefixesList.contains(commonPrefix)) {
                    commonPrefixesList.add(commonPrefix);
                    count++;
                }
                break;
            } else {
                Content content = convertBsonToContent(record, encodingType);
                content.setOwner(owner);
                contentList.add(content);
                count++;
                break;
            }
        }

        return count;
    }

    private int skipKeyMarker(QueryDbCursor cursorCur, QueryDbCursor cursorHis, String keyMarker, String versionIdMarker,
                              ListVersionsResult listVersionsResult, String delimiter, int maxNumber,
                              String encodingType, Owner owner, int prefixLen, Boolean isExist)
            throws S3ServerException{
        try {
            int count = 0;
            Boolean isFindNext = false;
            LinkedHashSet<CommonPrefix> commonPrefixesList = listVersionsResult.getCommonPrefixList();

            int delimiterLen = 0;
            if (null != delimiter) {
                delimiterLen = delimiter.length();
            }

            Loop:
            while (cursorCur.hasNext() && !isFindNext) {
                Boolean isCommonPrefix = false;
                CommonPrefix commonPrefix = null;
                String curPrefix = null;
                BSONObject recordA = cursorCur.getNext();
                String keyA = recordA.get(ObjectMeta.META_KEY_NAME).toString();
                int delimiterIndex = -1;
                if (null != delimiter) {
                    delimiterIndex = keyA.indexOf(delimiter, prefixLen);
                }
                if (-1 != delimiterIndex) {
                    isCommonPrefix = true;
                    curPrefix = keyA.substring(0, delimiterIndex + delimiterLen);
                    int comResult = curPrefix.compareTo(keyMarker);
                    if (comResult > 0) {
                        isFindNext = true;
                        commonPrefix = new CommonPrefix(curPrefix, encodingType);
                        if (!commonPrefixesList.contains(commonPrefix)) {
                            commonPrefixesList.add(commonPrefix);
                            count++;
                        }
                    }
                } else {
                    if (!isExist) {
                        isFindNext = true;
                    }
                    if (keyA.compareTo(keyMarker) > 0){
                        isFindNext = true;
                    }
                    if (isFindNext) {
                        recordVersionDeleteMarker(recordA, listVersionsResult,
                                encodingType, owner, true);
                        count++;
                    } else {
                        Version cvtVersion = convertBsonToVersion(recordA, encodingType);
                        if (keyA.equals(keyMarker) && cvtVersion.getVersionId().equals(versionIdMarker)) {
                            isFindNext = true;
                        }
                    }
                }
                if (count >= maxNumber) {
                    if (isCommonPrefix && (cursorCur.hasNext() || cursorHis != null)) {
                        if (commonPrefix != null) {
                            recordVersionsTruncated(listVersionsResult, commonPrefix.getPrefix(), null);
                        }
                    } else if (!isCommonPrefix && (cursorCur.hasNext() || cursorHis != null)) {
                        Version version = convertBsonToVersion(recordA, encodingType);
                        recordVersionsTruncated(listVersionsResult, version.getKey(), version.getVersionId());
                    }
                    break;
                }

                while (cursorHis != null) {
                    BSONObject recordB = cursorHis.getCurrent();
                    String keyB = recordB.get(ObjectMeta.META_KEY_NAME).toString();
                    int result = keyB.compareTo(keyA);
                    if (0 == result) {
                        if (!isCommonPrefix) {
                            if (isFindNext) {
                                recordVersionDeleteMarker(recordB, listVersionsResult,
                                        encodingType, owner, false);
                                count++;
                            } else {
                                Version cvtVersion = convertBsonToVersion(recordB, encodingType);
                                if (keyB.equals(keyMarker) && cvtVersion.getVersionId().equals(versionIdMarker)) {
                                    isFindNext = true;
                                }
                            }
                        }
                    } else if (result > 0) {
                        break;
                    }

                    if (count >= maxNumber) {
                        if (cursorCur.hasNext() || cursorHis.hasNext()) {
                            Version version = convertBsonToVersion(recordB, encodingType);
                            recordVersionsTruncated(listVersionsResult, version.getKey(), version.getVersionId());
                        }
                        break Loop;
                    }
                    if (cursorHis.hasNext()) {
                        cursorHis.getNext();
                    } else {
                        metaDao.releaseQueryDbCursor(cursorHis);
                        cursorHis = null;
                    }
                }
            }
            return count;
        }catch (S3ServerException e){
            throw e;
        }
    }

    private Boolean generateNoVersionFlag(VersioningStatusType status){
        if (null == status){
            return false;
        }
        switch(status){
            case ENABLED:
                return false;
            default:
                return true;
        }
    }

    private void deleteObjectLob(ObjectMeta deleteObject) throws S3ServerException{
        if (deleteObject != null && !deleteObject.getDeleteMarker()) {
            int tryTime = DBParamDefine.DB_DUPLICATE_MAX_TIME;
            while (tryTime > 0) {
                tryTime--;
                try {
                    dataDao.deleteObjectDataByLobId(null, deleteObject.getCsName(),
                            deleteObject.getClName(), deleteObject.getLobId());
                    break;
                }catch (S3ServerException e){
                    if (e.getError() == S3Error.OBJECT_IS_IN_USE && tryTime > 0){
                        if (tryTime  == DBParamDefine.DB_DUPLICATE_MAX_TIME-1){
                            logger.error("lob is in use.lob is:{}",deleteObject);
                        }
                        continue;
                    }else {
                        throw e;
                    }
                }
            }
        }
    }

    private void writeObjectMeta(String metaCsName, String metaClName,
                                 ObjectMeta objectMeta, String objectName, long bucketId,
                                 VersioningStatusType versioningStatusType)
            throws S3ServerException{
        String metaHisClName = metaDao.getMetaHistoryCLName();

        int tryTime = DBParamDefine.DB_DUPLICATE_MAX_TIME;
        while (tryTime > 0) {
            tryTime--;
            ConnectionDao connection = daoMgr.getConnectionDao();
            transaction.begin(connection);
            try {
                ObjectMeta metaResult = metaDao.queryForUpdate(connection, metaCsName, metaClName,
                        bucketId, objectName, null, null);
                if (null == metaResult) {
                    objectMeta.setVersionId(0);
                    metaDao.insertMeta(connection, metaCsName, metaClName, objectMeta,
                            0, false);
                    transaction.commit(connection);
                } else {
                    objectMeta.setVersionId(metaResult.getVersionId() + 1);
                    if (VersioningStatusType.NONE == versioningStatusType
                            || (VersioningStatusType.SUSPENDED == versioningStatusType
                            && metaResult.getNoVersionFlag())) {
                        metaDao.updateMeta(connection, metaCsName, metaClName, bucketId,
                                objectName, null, objectMeta);
                        transaction.commit(connection);
                        deleteObjectLob(metaResult);
                    } else {
                        metaDao.insertMeta(connection, metaCsName, metaHisClName,
                                metaResult, 1, true);
                        metaDao.updateMeta(connection, metaCsName, metaClName, bucketId,
                                objectName, null, objectMeta);
                        ObjectMeta nullMeta = null;
                        if (VersioningStatusType.SUSPENDED == versioningStatusType) {
                            nullMeta = metaDao.queryForUpdate(connection, metaCsName, metaHisClName,
                                    bucketId, objectName, null, true);
                            if (null != nullMeta) {
                                metaDao.removeMeta(connection, metaCsName, metaHisClName, bucketId,
                                        objectName, null, true);
                            }
                        }
                        transaction.commit(connection);
                        deleteObjectLob(nullMeta);
                    }
                }
                return;
            } catch (S3ServerException e) {
                transaction.rollback(connection);
                if (e.getError().getErrIndex() == S3Error.DAO_DUPLICATE_KEY.getErrIndex() && tryTime > 0) {
                    continue;
                } else {
                    throw e;
                }
            } catch (Exception e) {
                transaction.rollback(connection);
                throw e;
            } finally {
                daoMgr.releaseConnectionDao(connection);
            }
        }
        throw new S3ServerException(S3Error.DAO_DUPLICATE_KEY,
                "bucket+key duplicate too times. bucketId:"+bucketId+", key:"+objectName);
    }

    private Boolean isExistKeyVersion(String metaCsName, String metaClName, String metaHisClName,
                                            String keyMarker, String versionIdMarker, Bucket bucket){
        Boolean isExistKeyVersion = false;
        try {
            if (versionIdMarker.equals(ObjectMeta.NULL_VERSION_ID)) {
                if (metaDao.queryMetaByObjectName(metaCsName, metaClName, bucket.getBucketId(), keyMarker, null, true) != null
                        || metaDao.queryMetaByObjectName(metaCsName, metaHisClName, bucket.getBucketId(), keyMarker, null, true) != null) {
                    isExistKeyVersion = true;
                }
            } else {
                Long specifiedVersionId = Long.parseLong(versionIdMarker);
                if (metaDao.queryMetaByObjectName(metaCsName, metaClName, bucket.getBucketId(), keyMarker, specifiedVersionId, false) != null
                        || metaDao.queryMetaByObjectName(metaCsName, metaHisClName, bucket.getBucketId(), keyMarker, specifiedVersionId, false) != null) {
                    isExistKeyVersion = true;
                }
            }
        }catch (NumberFormatException e){
            logger.error("Parse versionIdMarker failed, versionIdMarker:{}",
                    versionIdMarker);
        }catch (Exception e){
            logger.error("isExistSpecifiedVersion failed, versionIdMarker:{}",
                    versionIdMarker);
        }finally {
            return isExistKeyVersion;
        }
    }

    private void analyseRangeWithLob(Range range, DataLob dataLob) throws S3ServerException{
        if (null == range){
            return;
        }

        long contentLength = dataLob.getSize();
        if (range.getStart() >= contentLength){
            throw new S3ServerException(S3Error.OBJECT_RANGE_INVALID,
                    "start > contentlength. start:" + range.getStart() +
                            ", contentlength:" + contentLength);
        }

        //final bytes
        if (range.getStart() == -1){
            if(range.getEnd() < contentLength) {
                range.setStart(contentLength - range.getEnd());
                range.setEnd(contentLength-1);
            }else {
                range.setStart(0);
                range.setEnd(contentLength-1);
            }
        }

        //from start to the final of Lob
        if (range.getEnd() == -1 || range.getEnd() >= contentLength){
            range.setEnd(contentLength - 1);
        }

        //from 0 - final of Lob
        if (range.getStart() == 0 && range.getEnd() == contentLength - 1){
            range.setContentLength(contentLength);
            return;
        }

        long readLength  = range.getEnd() - range.getStart() + 1;
        range.setContentLength(readLength);
    }
}
