package com.sequoias3.service.impl;

import com.sequoiadb.base.DBCursor;
import com.sequoias3.common.DBParamDefine;
import com.sequoias3.common.RestParamDefine;
import com.sequoias3.config.SequoiadbConfig;
import com.sequoias3.context.Context;
import com.sequoias3.context.ContextManager;
import com.sequoias3.core.*;
import com.sequoias3.dao.*;
import com.sequoias3.exception.S3Error;
import com.sequoias3.exception.S3ServerException;
import com.sequoias3.service.BucketService;
import com.sequoias3.service.ObjectService;
import org.apache.commons.codec.binary.Hex;
import org.bson.BSONObject;
import org.bson.BasicBSONObject;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;
import sun.misc.BASE64Decoder;

import java.io.InputStream;
import java.io.OutputStream;
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
    SequoiadbConfig config;

    @Autowired
    ContextManager contextManager;

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

        try {
            //get and check bucket
            Bucket bucket = bucketService.getBucket(ownerID, bucketName);

            //get cs and cl
            Date createDate = new Date();
            String dataCsName = dataDao.getDataCSName(bucket.getRegion(), createDate);
            String dataClName = dataDao.getDataClName();
            String metaCsName = metaDao.getMetaCSName(bucket.getRegion());
            String metaClName = metaDao.getMetaCurCLName();

            //build meta
            ObjectMeta objectMeta = buildObjectMeta(objectName, bucket.getBucketId(),
                    headers, xMeta, dataCsName, dataClName);

            //insert lob
            InsertResult insertResult =dataDao.insertObjectData(dataCsName, dataClName, inputStream);
            String eTag = insertResult.geteTag();
            objectMeta.seteTag(insertResult.geteTag());
            objectMeta.setSize(insertResult.getSize());
            objectMeta.setLobId(insertResult.getLobId());

            //check md5
            if (null != contentMD5) {
                if (!isMd5EqualWithETag(contentMD5, eTag)) {
                    dataDao.deleteObjectDataByLobId(dataCsName,
                            dataClName, objectMeta.getLobId());
                    throw new S3ServerException(S3Error.OBJECT_BAD_DIGEST,
                            "The Content-MD5 you speciﬁed did not match what we received.");
                }
            }

            //build response
            PutDeleteResult result = new PutDeleteResult();
            result.seteTag(eTag);

            if (bucket.getVersioningStatus().equals(DBParamDefine.DB_VERSIONING_STATUS_NULL)) {
                //no versioning
                objectMeta.setVersionId(ObjectMeta.NULL_VERSION_ID);

                int tryTime = DBParamDefine.DB_DUPLICATE_MAX_TIME;
                while (tryTime > 0) {
                    tryTime--;
                    try {
                        ObjectMeta queryMeta = metaDao.queryAndUpdateMeta(metaCsName, metaClName,
                                bucket.getBucketId(), objectMeta, objectName);
                        if (null == queryMeta) {
                            metaDao.insertMeta(metaCsName, metaClName,
                                    objectMeta, objectName);
                        } else {
                            dataDao.deleteObjectDataByLobId(queryMeta.getCsName(),
                                    queryMeta.getClName(), queryMeta.getLobId());
                        }
                        return result;
                    } catch (S3ServerException e) {
                        logger.warn("Create user failed. bucketname=" + bucketName, e);
                        if (e.getError().getErrIndex() == S3Error.DAO_DUPLICATE_KEY.getErrIndex() && tryTime > 0) {
                            continue;
                        }else if(e.getError().getErrIndex() == S3Error.OBJECT_IS_IN_USE.getErrIndex()){
                            logger.error("lob is in user. remove failed. error:{}", e.toString());
                            return result;
                        }
                        else {
                            throw e;
                        }
                    }
                }
                throw new S3ServerException(S3Error.OBJECT_PUT_fAILED,
                        "bucket+key duplicate too times. bucket:"+bucketName+" key:"+objectName);
            }
            return null;
        }catch (S3ServerException e){
            throw e;
        }catch (Exception e){
            throw new S3ServerException(S3Error.OBJECT_PUT_fAILED, "put object failed.", e);
        }
    }

    @Override
    public ObjectMeta getObject(int ownerID, String bucketName, String objectName,
                                String versionId, Map headers, Range range, OutputStream outputStream)
            throws S3ServerException {
        try {
            Bucket bucket = bucketService.getBucket(ownerID, bucketName);
            String metaCsName = metaDao.getMetaCSName(bucket.getRegion());

            //if (null == versionId)
            {
                String metaClName = metaDao.getMetaCurCLName();

                int tryTime = DBParamDefine.DB_DUPLICATE_MAX_TIME;
                while (tryTime > 0) {
                    tryTime--;
                    try {
                        ObjectMeta objectMeta = metaDao.queryMetaByObjectName(metaCsName, metaClName,
                                bucket.getBucketId(), objectName);
                        if (null == objectMeta) {
                            throw new S3ServerException(S3Error.OBJECT_NO_SUCH_KEY, "no object");
                        }

                        checkMatchModify(headers, objectMeta);
                        dataDao.getObjectDataByLobId(objectMeta.getCsName(), objectMeta.getClName(),
                                objectMeta.getLobId(), range, outputStream);
                        return objectMeta;
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
            }
        }catch (S3ServerException e) {
            throw e;
        } catch (Exception e) {
            throw new S3ServerException(S3Error.OBJECT_GET_FAILED,
                    "get object failed. bucket:" + bucketName + ", object=" + objectName, e);
        }
    }

    @Override
    public PutDeleteResult deleteObject(int ownerID, String bucketName, String objectName, String versionId)
            throws S3ServerException {
        try {
            Bucket bucket = bucketService.getBucket(ownerID, bucketName);

            String metaCsName = metaDao.getMetaCSName(bucket.getRegion());
            String metaClName = metaDao.getMetaCurCLName();

            //versioning is null ,return null
            //if (bucket.getVersioningStatus().equals(DBParamDefine.DB_VERSIONING_STATUS_NULL))
            {
                ObjectMeta objectMeta = metaDao.queryAndRemoveMeta(metaCsName, metaClName,
                        bucket.getBucketId(), objectName);
                if (null != objectMeta) {
                    int tryTime = DBParamDefine.DB_DUPLICATE_MAX_TIME;
                    while (tryTime > 0) {
                        tryTime--;
                        try {
                            dataDao.deleteObjectDataByLobId(objectMeta.getCsName(), objectMeta.getClName(),
                                    objectMeta.getLobId());
                            break;
                        }catch (S3ServerException e){
                            if (e.getError() == S3Error.OBJECT_IS_IN_USE && tryTime > 0){
                                if (tryTime  == DBParamDefine.DB_DUPLICATE_MAX_TIME-1){
                                    logger.error("delete object, lob is in use.lob id:{}",objectMeta.getLobId());
                                }
                                continue;
                            }else {
                                throw e;
                            }
                        }
                    }

                }
                return null;
            }
            //versioning is enabled, return versionId, delete marker
        }catch (S3ServerException e) {
            throw e;
        } catch (Exception e) {
            throw new S3ServerException(S3Error.OBJECT_GET_FAILED,
                    "get object failed. bucket:" + bucketName + ", object=" + objectName, e);
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
                    dbCursor = queryContext.getDbCursor();
                }
            }
            if (null == queryContext){
                //get cs,cl
                String metaCsName = metaDao.getMetaCSName(bucket.getRegion());
                String metaClName = metaDao.getMetaCurCLName();
                //get cursor
                dbCursor = metaDao.queryMetaByBucket(metaCsName, metaClName,
                        bucket.getBucketId(), prefix, startAfter);
            }

            if (null == dbCursor || null == dbCursor.getCursor()) {
                return listObjectsResult;
            }

            Owner owner = null;
            if (fetchOwner){
                owner = userDao.getOwnerByUserID(ownerID);
            }

            DBCursor cursor = dbCursor.getCursor();
            int count = 0;
            int maxNumber = Math.min(maxKeys, RestParamDefine.MAX_KEYS_DEFAULT);
            int prefixLen = 0;
            if (null != prefix){
                prefixLen = prefix.length();
            }
            //no delimiter
            if (null == delimiter) {
                LinkedHashSet<Content> contentList = listObjectsResult.getContentList();;
                while (cursor.hasNext() && count < maxNumber) {
                    BSONObject record = cursor.getNext();
                    Content content = convertBsonToContent(record, encodingType);
                    content.setOwner(owner);
                    contentList.add(content);
                    count++;
                }
            }else{
            //else if(!delimiter.equals(bucket.getDelimiter())){
                LinkedHashSet<Content> contentList = listObjectsResult.getContentList();
                LinkedHashSet<CommonPrefix> commonPrefixesList = listObjectsResult.getCommonPrefixList();
                if (queryContext != null && queryContext.getLastCommonPrefix() != null) {
                    count = skipLastCommonPrefix(queryContext.getLastCommonPrefix(),
                            cursor, delimiter, prefixLen, encodingType, listObjectsResult, owner);
                    queryContext.setLastCommonPrefix(null);
                }
                while (cursor.hasNext() && count < maxNumber) {
                    BSONObject record = cursor.getNext();
                    String key = record.get(ObjectMeta.META_KEY_NAME).toString();
                    int delimiterIndex = key.indexOf(delimiter, prefixLen);
                    if (-1 != delimiterIndex){
                        //commonprefix
                        CommonPrefix commonPrefix = new CommonPrefix(key.substring(0, delimiterIndex+1), encodingType);
                        if (!commonPrefixesList.contains(commonPrefix)){
                            commonPrefix.setOwner(owner);
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

            if (cursor.hasNext()) {
                if (null == queryContext){
                    queryContext = contextManager.create(bucket.getBucketId());
                    queryContext.setPrefix(prefix);
                    queryContext.setStartAfter(startAfter);
                    queryContext.setDelimiter(delimiter);
                    queryContext.setDbCursor(dbCursor);
                }

                //record context
                if (null != delimiter) {
                    BSONObject record = cursor.getCurrent();
                    String key = record.get(ObjectMeta.META_KEY_NAME).toString();
                    int delimiterIndex = key.indexOf(delimiter, prefixLen);
                    if (-1 != delimiterIndex) {
                        queryContext.setLastCommonPrefix(key.substring(0, delimiterIndex+1));
                    }
                }
                listObjectsResult.setIsTruncated(true);
                listObjectsResult.setNextContinueToken(queryContext.getToken());
            } else {
                metaDao.releaseDBAndCursor(dbCursor);
                contextManager.release(queryContext);
            }
            return listObjectsResult;
        } catch (S3ServerException e){
            metaDao.releaseDBAndCursor(dbCursor);
            contextManager.release(queryContext);
            throw e;
        } catch (Exception e){
            metaDao.releaseDBAndCursor(dbCursor);
            contextManager.release(queryContext);
            throw new S3ServerException(S3Error.OBJECT_LIST_FAILED, "error message:"+e.getMessage());
        }
    }

    @Override
    public long getObjectNumberByBucketId(Bucket bucket) throws S3ServerException{
        try {
            String metaCsName = metaDao.getMetaCSName(bucket.getRegion());
            String metaClName = metaDao.getMetaCurCLName();

            return metaDao.getObjectNumber(metaCsName, metaClName, bucket.getBucketId());
        }catch (S3ServerException e){
            throw e;
        }
    }

    private ObjectMeta buildObjectMeta(String objectName, long bucketId, Map headers,
                                       Map xMeta, String dataCsName, String dataClName) {
        ObjectMeta objectMeta = new ObjectMeta();
        objectMeta.setKey(objectName);
        objectMeta.setBucketId(bucketId);
        objectMeta.setCsName(dataCsName);
        objectMeta.setClName(dataClName);
        objectMeta.setLastModified(System.currentTimeMillis());

        if (headers.containsKey(RestParamDefine.PutObjectHeader.CACHE_CONTROL)){
            objectMeta.setCacheControl(headers.get(RestParamDefine.PutObjectHeader.CACHE_CONTROL).toString());
        }

        if (headers.containsKey(RestParamDefine.PutObjectHeader.CONTENT_DISPOSITION)){
            objectMeta.setContentDisposition(headers.get(RestParamDefine.PutObjectHeader.CONTENT_DISPOSITION).toString());
        }

        if (headers.containsKey(RestParamDefine.PutObjectHeader.CONTENT_ENCODING)){
            objectMeta.setContentEncoding(headers.get(RestParamDefine.PutObjectHeader.CONTENT_ENCODING).toString());
        }

        if (headers.containsKey(RestParamDefine.PutObjectHeader.CONTENT_TYPE)){
            objectMeta.setContentType(headers.get(RestParamDefine.PutObjectHeader.CONTENT_TYPE).toString());
        }

        if (headers.containsKey(RestParamDefine.PutObjectHeader.EXPIRES)){
            objectMeta.setExpires(headers.get(RestParamDefine.PutObjectHeader.EXPIRES).toString());
        }

        if (headers.containsKey(RestParamDefine.PutObjectHeader.CONTENT_LANGUAGE)){
            objectMeta.setContentLanguage(headers.get(RestParamDefine.PutObjectHeader.CONTENT_LANGUAGE).toString());
        }

        objectMeta.setMetaList(xMeta);

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

    private Boolean isMd5EqualWithETag(String contentMd5, String eTag) throws S3ServerException{
        try {
            BASE64Decoder decoder = new BASE64Decoder();
            String textMD5 = new String(Hex.encodeHex(decoder.decodeBuffer(contentMd5)));
            if (textMD5.equals(eTag)){
                return true;
            }else {
                return false;
            }
        }catch (Exception e){
            //返回false，交给外面处理，当做不匹配处理
            throw new S3ServerException(S3Error.UNKNOWN_ERROR,
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
                        "if match isNotMatch: matchetag：" + matchEtag.toString() + ", etag:" + eTag);
            }else{
                isMatch = true;
            }
        }

        Object noneMatchEtag = headers.get(RestParamDefine.GetObjectReqHeader.REQ_IF_NONE_MATCH);
        if (null != noneMatchEtag){
            if (noneMatchEtag.toString().equals(eTag)){
                throw new S3ServerException(S3Error.OBJECT_IF_NONE_MATCH_FAILED,
                        "if none match isNotMatch: matchetag：" + matchEtag.toString() + ", etag:" + eTag);
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
                            "if modified since date: last：" + date.getTime() +
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
                            "if modified since date: last：" + date.getTime() +
                                    ", lastModifiedTime:" + lastModifiedTime);
                }
            }
        }

        return true;
    }

    private Boolean IsContextMatch(Context queryContext, String prefix, String startAfter,
                                 String delimiter) throws S3ServerException{
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

    private int skipLastCommonPrefix(String lastCommonPrefix, DBCursor cursor,
                                      String delimiter, int prefixLen, String encodingType,
                                      ListObjectsResult listObjectsResult, Owner owner)
            throws S3ServerException{
        int count = 0;
        LinkedHashSet<Content> contentList = listObjectsResult.getContentList();
        LinkedHashSet<CommonPrefix> commonPrefixesList = listObjectsResult.getCommonPrefixList();
        while (cursor.hasNext()) {
            BSONObject record = cursor.getNext();
            String key = record.get(ObjectMeta.META_KEY_NAME).toString();
            int delimiterIndex = key.indexOf(delimiter, prefixLen);
            if (-1 != delimiterIndex) {
                String keyCommonPrefix = key.substring(0, delimiterIndex + 1);
                if (keyCommonPrefix.equals(lastCommonPrefix)) {
                    continue;
                }
                CommonPrefix commonPrefix = new CommonPrefix(keyCommonPrefix, encodingType);
                if (!commonPrefixesList.contains(commonPrefix)) {
                    commonPrefix.setOwner(owner);
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




}
