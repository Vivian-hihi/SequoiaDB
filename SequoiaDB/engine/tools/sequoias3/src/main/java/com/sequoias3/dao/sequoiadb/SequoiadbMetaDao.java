package com.sequoias3.dao.sequoiadb;

import com.sequoiadb.base.*;
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.exception.SDBError;
import com.sequoias3.common.DBParamDefine;
import com.sequoias3.config.SequoiadbConfig;
import com.sequoias3.context.Context;
import com.sequoias3.core.ObjectMeta;
import com.sequoias3.core.QueryDbCursor;
import com.sequoias3.dao.DaoCollectionDefine;
import com.sequoias3.dao.MetaDao;
import com.sequoias3.exception.S3Error;
import com.sequoias3.exception.S3ServerException;
import org.bson.BSON;
import org.bson.BSONObject;
import org.bson.BasicBSONObject;
import org.bson.types.ObjectId;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Repository;

@Repository("MetaDao")
public class SequoiadbMetaDao implements MetaDao {
    private static final Logger logger = LoggerFactory.getLogger(SequoiadbMetaDao.class);

    @Autowired
    SdbDataSourceWrapper sdbDatasourceWrapper;

    @Autowired
    SequoiadbConfig config;

    @Override
    public String insertMeta(String csMetaName, String clMetaName, ObjectMeta objectMeta, String objectName) throws S3ServerException {
        Sequoiadb sdb = null;
        try {
            sdb = sdbDatasourceWrapper.getSequoiadb();
            insert(sdb, csMetaName, clMetaName, objectMeta, objectName);
            return null;
        }catch (BaseException e){
            if (e.getErrorCode() == SDBError.SDB_DMS_CS_NOTEXIST.getErrorCode()) {
                sdbDatasourceWrapper.createCS(sdb, csMetaName, null);
                sdbDatasourceWrapper.createCL(sdb, csMetaName, clMetaName, null);
                BSONObject indexKey = new BasicBSONObject();
                indexKey.put(ObjectMeta.META_BUCKET_ID, 1);
                indexKey.put(ObjectMeta.META_KEY_NAME, 1);
                sdbDatasourceWrapper.createIndex(sdb, csMetaName, clMetaName,
                        ObjectMeta.META_BUCKET_ID+"+"+ObjectMeta.META_KEY_NAME,
                        indexKey, true,true);
                insert(sdb, csMetaName, clMetaName, objectMeta, objectName);
                return null;
            } else if (e.getErrorCode() == SDBError.SDB_DMS_NOTEXIST.getErrorCode()) {
                sdbDatasourceWrapper.createCL(sdb, csMetaName, clMetaName, null);
                BSONObject indexKey = new BasicBSONObject();
                indexKey.put(ObjectMeta.META_BUCKET_ID, 1);
                indexKey.put(ObjectMeta.META_KEY_NAME, 1);
                sdbDatasourceWrapper.createIndex(sdb, csMetaName, clMetaName,
                        ObjectMeta.META_BUCKET_ID+"+"+ObjectMeta.META_KEY_NAME,
                        indexKey, true,true);
                insert(sdb, csMetaName, clMetaName, objectMeta, objectName);
                return null;
            } else if (e.getErrorCode() == SDBError.SDB_IXM_DUP_KEY.getErrorCode()) {
                throw new S3ServerException(S3Error.DAO_DUPLICATE_KEY, "Duplicate key.");
            }else {
                logger.error("insert meta failed. error:",e);
                throw e;
            }
        } finally {
            sdbDatasourceWrapper.releaseSequoiadb(sdb);
        }
    }

    public String insert(Sequoiadb sdb, String csMetaName, String clMetaName, ObjectMeta objectMeta, String objectName) {
        try {
            CollectionSpace cs = sdb.getCollectionSpace(csMetaName);
            DBCollection cl = cs.getCollection(clMetaName);

            BSONObject insertData = convertMetaToBson(objectMeta);
            cl.insert(insertData);
            return null;
        }catch (Exception e){
            logger.info("Insert object meta failed");
            throw e;
        }
    }

    @Override
    public QueryDbCursor queryMetaByBucket(String metaCsName, String metaClName, long bucketId,
                                  String prefix, String startAfter) throws S3ServerException{
        Sequoiadb sdb = null;
        DBCursor dbCursor = null;
        try {
            sdb = sdbDatasourceWrapper.getSequoiadb();
            CollectionSpace cs = sdb.getCollectionSpace(metaCsName);
            DBCollection cl = cs.getCollection(metaClName);

            BSONObject matcher = new BasicBSONObject();
            matcher.put(ObjectMeta.META_BUCKET_ID, bucketId);
            matcher.put(ObjectMeta.META_DELETE_MARKER, false);
            BSONObject keyMatcher = new BasicBSONObject();
            if (prefix != null){
                keyMatcher.put(DBParamDefine.REGEX, "^" + prefix + ".*");
            }
            if (startAfter != null){
                keyMatcher.put(DBParamDefine.GREATER_THAN, startAfter);
            }
            if (!keyMatcher.isEmpty()) {
                matcher.put(ObjectMeta.META_KEY_NAME, keyMatcher);
            }

            BSONObject selector = new BasicBSONObject();
            selector.put(ObjectMeta.META_KEY_NAME, "");
            selector.put(ObjectMeta.META_LAST_MODIFIED, 0);
            selector.put(ObjectMeta.META_ETAG, "");
            selector.put(ObjectMeta.META_SIZE, "");

            BSONObject orderBy = new BasicBSONObject();
            orderBy.put(ObjectMeta.META_KEY_NAME, 1);

            dbCursor = cl.query(matcher, selector,orderBy,null, 0);
            return new QueryDbCursor(sdb, dbCursor);
        }catch (BaseException e){
            sdbDatasourceWrapper.releaseDBCursor(dbCursor);
            sdbDatasourceWrapper.releaseSequoiadb(sdb);
            if (e.getErrorCode() == SDBError.SDB_DMS_CS_NOTEXIST.getErrorCode() ||
                    e.getErrorCode() == SDBError.SDB_DMS_NOTEXIST.getErrorCode()) {
                //no cs or cl ,return null
                return null;
            } else {
                logger.error("query metalist by bucket failed. error:",e);
                throw e;
            }
        } catch (Exception e){
            sdbDatasourceWrapper.releaseDBCursor(dbCursor);
            sdbDatasourceWrapper.releaseSequoiadb(sdb);
            logger.error("query metalist by bucket failed.");
            throw e;
        }
    }

    @Override
    public void releaseDBAndCursor(QueryDbCursor dbCursor){
        if (null != dbCursor){
            sdbDatasourceWrapper.releaseDBCursor(dbCursor.getCursor());
            sdbDatasourceWrapper.releaseSequoiadb(dbCursor.getSdb());
            dbCursor.setCursor(null);
            dbCursor.setSdb(null);
        }
    }

    @Override
    public ObjectMeta queryMetaByObjectName(String metaCsName, String metaClName, long bucketId, String objectName) throws S3ServerException {
        Sequoiadb sdb = null;
        try {
            sdb = sdbDatasourceWrapper.getSequoiadb();
            CollectionSpace cs = sdb.getCollectionSpace(metaCsName);
            DBCollection cl = cs.getCollection(metaClName);

            BSONObject matcher = new BasicBSONObject();
            matcher.put("BucketId", bucketId);
            matcher.put("Key", objectName);

            BSONObject queryResult = cl.queryOne(matcher, null, null, null, 0);
            return convertBsonToMeta(queryResult);
        }catch (BaseException e){
            if (e.getErrorCode() == SDBError.SDB_DMS_CS_NOTEXIST.getErrorCode() ||
                    e.getErrorCode() == SDBError.SDB_DMS_NOTEXIST.getErrorCode()) {
                //no cs or cl ,return null
                return null;
            } else {
                logger.error("query meta by name failed. error:",e);
                throw e;
            }
        } catch (Exception e){
            logger.error("query meta by name failed.");
            throw e;
        }finally {
            sdbDatasourceWrapper.releaseSequoiadb(sdb);
        }
    }

    @Override
    public ObjectMeta queryAndUpdateMeta(String metaCsName, String metaClName, long bucketId, ObjectMeta objectMeta, String objectName) throws S3ServerException {
        Sequoiadb sdb = null;
        try {
            sdb = sdbDatasourceWrapper.getSequoiadb();
            CollectionSpace cs = sdb.getCollectionSpace(metaCsName);
            DBCollection cl = cs.getCollection(metaClName);

            BSONObject matcher = new BasicBSONObject();
            matcher.put(ObjectMeta.META_BUCKET_ID, bucketId);
            matcher.put(ObjectMeta.META_KEY_NAME, objectName);

            BSONObject updateData = convertMetaToBson(objectMeta);
            BSONObject setUpdate = new BasicBSONObject();
            setUpdate.put(DBParamDefine.MODIFY_SET, updateData);

            DBCursor queryResult = cl.queryAndUpdate(matcher, null, null, null, setUpdate, 0, 1, DBQuery.FLG_QUERY_WITH_RETURNDATA, false);
            if (queryResult.hasNext()){
                return convertBsonToMeta(queryResult.getNext());
            }
            return null;
        }catch (BaseException e){
            if (e.getErrorCode() == SDBError.SDB_DMS_CS_NOTEXIST.getErrorCode() ||
                    e.getErrorCode() == SDBError.SDB_DMS_NOTEXIST.getErrorCode()) {
                //no cs or cl ,return null
                return null;
            } else {
                logger.error("queryandupdate meta failed. error:",e);
                throw e;
            }
        }catch (Exception e){
            logger.error("queryandupdate meta failed.");
            throw e;
        }finally {
            sdbDatasourceWrapper.releaseSequoiadb(sdb);
        }
    }

    @Override
    public ObjectMeta queryAndRemoveMeta(String metaCsName, String metaClName, long bucketId, String objectName) throws S3ServerException {
        Sequoiadb sdb = null;
        DBCursor queryResult = null;
        try {
            sdb = sdbDatasourceWrapper.getSequoiadb();
            CollectionSpace cs = sdb.getCollectionSpace(metaCsName);
            DBCollection cl = cs.getCollection(metaClName);

            BSONObject matcher = new BasicBSONObject();
            matcher.put("BucketId", bucketId);
            matcher.put("Key", objectName);

            queryResult = cl.queryAndRemove(matcher, null, null, null, 0, 1, 0);
            if (queryResult.hasNext()){
                return convertBsonToMeta(queryResult.getNext());
            }else {
                return null;
            }
        }catch (BaseException e){
            if (e.getErrorCode() == SDBError.SDB_DMS_CS_NOTEXIST.getErrorCode() ||
                    e.getErrorCode() == SDBError.SDB_DMS_NOTEXIST.getErrorCode()) {
                //no cs or cl ,return null
                return null;
            } else {
                logger.error("queryandremove meta failed. error:",e);
                throw e;
            }
        }catch (Exception e){
            logger.error("query meta and remove failed.");
            throw e;
        }finally {
            sdbDatasourceWrapper.releaseDBCursor(queryResult);
            sdbDatasourceWrapper.releaseSequoiadb(sdb);

        }
    }

    @Override
    public long getObjectNumber(String metaCsName, String metaClName, long bucketId)
            throws S3ServerException{
        Sequoiadb sdb = null;
        try {
            sdb = sdbDatasourceWrapper.getSequoiadb();
            CollectionSpace cs = sdb.getCollectionSpace(metaCsName);
            DBCollection cl = cs.getCollection(metaClName);

            BSONObject matcher = new BasicBSONObject();
            matcher.put(ObjectMeta.META_BUCKET_ID, bucketId);

            return cl.getCount(matcher);
        }catch (BaseException e){
            if (e.getErrorCode() == SDBError.SDB_DMS_CS_NOTEXIST.getErrorCode() ||
                    e.getErrorCode() == SDBError.SDB_DMS_NOTEXIST.getErrorCode()) {
                //no cs or cl ,return null
                return 0;
            } else {
                logger.error("query object number failed. error:",e);
                throw e;
            }
        } catch (Exception e){
            logger.error("query object number failed.");
            throw e;
        }finally {
            sdbDatasourceWrapper.releaseSequoiadb(sdb);
        }
    }

    @Override
    public String getMetaCSName( String region){
        StringBuilder csName = new StringBuilder();

        if (null != region){
            csName.append(region);
            csName.append(DBParamDefine.CS_META);
        }else {
            csName.append(config.getMetaCsName());
        }

        return csName.toString();
    }

    @Override
    public String getMetaCLName(){
        return DaoCollectionDefine.OBJECT_META_LIST;
    }

    @Override
    public String getMetaCurCLName(){
        return DaoCollectionDefine.OBJECT_META_LIST;
    }

    @Override
    public String getMetaHistoryCLName(){
        return DaoCollectionDefine.OBJECT_META_LIST_HISTORY;
    }

    private ObjectMeta convertBsonToMeta(BSONObject bsonObject){
        if (null == bsonObject){
            return null;
        }
        ObjectMeta object = new ObjectMeta();
        object.setBucketId((long)bsonObject.get(ObjectMeta.META_BUCKET_ID));
        object.setKey(bsonObject.get(ObjectMeta.META_KEY_NAME).toString());
        object.setLobId((ObjectId)bsonObject.get(ObjectMeta.META_LOB_ID));
        object.setVersionId(bsonObject.get(ObjectMeta.META_VERSION_ID).toString());
        object.setCsName(bsonObject.get(ObjectMeta.META_CS_NAME).toString());
        object.setClName(bsonObject.get(ObjectMeta.META_CL_NAME).toString());
        object.setLastModified((long)bsonObject.get(ObjectMeta.META_LAST_MODIFIED));
        object.seteTag(bsonObject.get(ObjectMeta.META_ETAG).toString());
        object.setSize((long)bsonObject.get(ObjectMeta.META_SIZE));
        if (bsonObject.get(ObjectMeta.META_EXPIRES) != null) {
            object.setExpires(bsonObject.get(ObjectMeta.META_EXPIRES).toString());
        }
        if (bsonObject.get(ObjectMeta.META_CONTENT_TYPE) != null) {
            object.setContentType(bsonObject.get(ObjectMeta.META_CONTENT_TYPE).toString());
        }
        if (bsonObject.get(ObjectMeta.META_CONTENT_ENCODING) != null) {
            object.setContentEncoding(bsonObject.get(ObjectMeta.META_CONTENT_ENCODING).toString());
        }
        if (bsonObject.get(ObjectMeta.META_CONTENT_DISPOSITION) != null) {
            object.setContentDisposition(bsonObject.get(ObjectMeta.META_CONTENT_DISPOSITION).toString());
        }
        if (bsonObject.get(ObjectMeta.META_CACHE_CONTROL) != null) {
            object.setCacheControl(bsonObject.get(ObjectMeta.META_CACHE_CONTROL).toString());
        }
        if (bsonObject.get(ObjectMeta.META_CONTENT_LANGUAGE) != null) {
            object.setContentLanguage(bsonObject.get(ObjectMeta.META_CONTENT_LANGUAGE).toString());
        }
        object.setDeleteMarker((Boolean)bsonObject.get(ObjectMeta.META_DELETE_MARKER));
        if (bsonObject.get(ObjectMeta.META_LIST) != null){
            BSONObject xMeta = (BSONObject)bsonObject.get(ObjectMeta.META_LIST);
            object.setMetaList(xMeta.toMap());
        }
        return object;
    }

    private BSONObject convertMetaToBson(ObjectMeta meta){
        if (null == meta){
            return null;
        }

        BSONObject objectMeta = new BasicBSONObject();
        objectMeta.put(ObjectMeta.META_KEY_NAME, meta.getKey());
        objectMeta.put(ObjectMeta.META_BUCKET_ID, meta.getBucketId());
        objectMeta.put(ObjectMeta.META_CS_NAME, meta.getCsName());
        objectMeta.put(ObjectMeta.META_CL_NAME, meta.getClName());
        objectMeta.put(ObjectMeta.META_LOB_ID, meta.getLobId());
        objectMeta.put(ObjectMeta.META_VERSION_ID, meta.getVersionId());
        objectMeta.put(ObjectMeta.META_ETAG, meta.geteTag());
        objectMeta.put(ObjectMeta.META_LAST_MODIFIED, meta.getLastModified());
        objectMeta.put(ObjectMeta.META_SIZE, meta.getSize());
        objectMeta.put(ObjectMeta.META_CACHE_CONTROL, meta.getCacheControl());
        objectMeta.put(ObjectMeta.META_CONTENT_DISPOSITION, meta.getContentDisposition());
        objectMeta.put(ObjectMeta.META_CONTENT_ENCODING, meta.getContentEncoding());
        objectMeta.put(ObjectMeta.META_CONTENT_TYPE, meta.getContentType());
        objectMeta.put(ObjectMeta.META_DELETE_MARKER, meta.getDeleteMarker());
        objectMeta.put(ObjectMeta.META_EXPIRES, meta.getExpires());
        objectMeta.put(ObjectMeta.META_CONTENT_LANGUAGE, meta.getContentLanguage());
        if (meta.getMetaList().isEmpty()){
            objectMeta.put(ObjectMeta.META_LIST, null);
        }else {
            BSONObject xMetaList = new BasicBSONObject(meta.getMetaList());
            objectMeta.put(ObjectMeta.META_LIST, xMetaList);
        }

        return objectMeta;
    }
}
