package com.sequoias3.dao.sequoiadb;

import com.sequoiadb.base.*;
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.exception.SDBError;
import com.sequoias3.common.DBParamDefine;
import com.sequoias3.config.SequoiadbConfig;
import com.sequoias3.core.ObjectMeta;
import com.sequoias3.core.Region;
import com.sequoias3.dao.*;
import com.sequoias3.exception.S3Error;
import com.sequoias3.exception.S3ServerException;
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

    @Autowired
    SequoiadbRegionSpaceDao sequoiadbRegionSpaceDao;

    @Override
    public void insertMeta(ConnectionDao connection, String csMetaName, String clMetaName,
                           ObjectMeta objectMeta, Boolean isHistory, Region region)
            throws S3ServerException {
        Sequoiadb sdb = null;
        try {
            sdb = ((SdbConnectionDao)connection).getConnection();
            insert(sdb, csMetaName, clMetaName, objectMeta, isHistory);
        }catch (BaseException e){
            if (e.getErrorCode() == SDBError.SDB_DMS_CS_NOTEXIST.getErrorCode()
                    || e.getErrorCode() == SDBError.SDB_DMS_NOTEXIST.getErrorCode()) {
                createMetaCSCL(sdb, region, csMetaName, clMetaName, isHistory);
                insert(sdb, csMetaName, clMetaName, objectMeta, isHistory);
            } else {
                logger.error("insert meta failed. error:",e);
                throw e;
            }
        }
    }

    private void insert(Sequoiadb sdb, String csMetaName, String clMetaName,
                       ObjectMeta objectMeta, Boolean isHistory)
            throws S3ServerException{
        try {
            CollectionSpace cs = sdb.getCollectionSpace(csMetaName);
            DBCollection cl = cs.getCollection(clMetaName);

            BSONObject insertData = convertMetaToBson(objectMeta);
            cl.insert(insertData);
        }catch (BaseException e){
            if (e.getErrorCode() == SDBError.SDB_IXM_DUP_KEY.getErrorCode()) {
                logger.error("duplicate key. csname:{}, clname:{}, key:{}, versionId:{}, isHistory:{}",
                        csMetaName, clMetaName, objectMeta.getKey(), objectMeta.getVersionId(), isHistory);
                if (!isHistory) {
                    throw new S3ServerException(S3Error.DAO_DUPLICATE_KEY,
                            "Duplicate key. csname:" + csMetaName + ", clname:" + clMetaName
                                    + ", key:" + objectMeta.getKey()
                                    + ", versionId:" + objectMeta.getVersionId());
                }
            }else{
                throw e;
            }
        }catch (Exception e){
            logger.error("Insert object meta failed");
            throw e;
        }
    }

    @Override
    public QueryDbCursor queryMetaByBucket(String metaCsName, String metaClName, long bucketId,
                                           String prefix, String startAfter, Boolean specifiedVId,
                                           Boolean isIncludeDeleteMarker) throws S3ServerException{
        Sequoiadb sdb = null;
        DBCursor dbCursor = null;
        try {
            sdb = sdbDatasourceWrapper.getSequoiadb();
            CollectionSpace cs = sdb.getCollectionSpace(metaCsName);
            DBCollection cl = cs.getCollection(metaClName);

            BSONObject matcher = new BasicBSONObject();
            matcher.put(ObjectMeta.META_BUCKET_ID, bucketId);
            if (!isIncludeDeleteMarker) {
                matcher.put(ObjectMeta.META_DELETE_MARKER, false);
            }
            BSONObject keyMatcher = new BasicBSONObject();
            if (prefix != null){
                keyMatcher.put(DBParamDefine.REGEX, "^" + prefix + ".*");
            }
            if (startAfter != null){
                if (specifiedVId) {
                    keyMatcher.put(DBParamDefine.NOT_SMALL, startAfter);
                }else{
                    keyMatcher.put(DBParamDefine.GREATER, startAfter);
                }
            }
            if (!keyMatcher.isEmpty()) {
                matcher.put(ObjectMeta.META_KEY_NAME, keyMatcher);
            }

            BSONObject selector = new BasicBSONObject();
            selector.put(ObjectMeta.META_KEY_NAME, "");
            selector.put(ObjectMeta.META_VERSION_ID, "");
            selector.put(ObjectMeta.META_LAST_MODIFIED, 0);
            selector.put(ObjectMeta.META_ETAG, "");
            selector.put(ObjectMeta.META_SIZE, "");
            selector.put(ObjectMeta.META_DELETE_MARKER, 0);
            selector.put(ObjectMeta.META_CS_NAME, 0);
            selector.put(ObjectMeta.META_CL_NAME, 0);
            selector.put(ObjectMeta.META_LOB_ID, 0);
            selector.put(ObjectMeta.META_NO_VERSION_FLAG, 0);

            BSONObject orderBy = new BasicBSONObject();
            orderBy.put(ObjectMeta.META_KEY_NAME, 1);
            orderBy.put(ObjectMeta.META_VERSION_ID, -1);

            dbCursor = cl.query(matcher, selector, orderBy,null, 0);
            return new SdbQueryDbCursor(sdb, dbCursor);
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
    public ObjectMeta queryMetaByObjectName(String metaCsName, String metaClName,
                                            long bucketId, String objectName,
                                            Long versionId, Boolean noVersionFlag)
            throws S3ServerException {
        Sequoiadb sdb = null;
        try {
            sdb = sdbDatasourceWrapper.getSequoiadb();
            CollectionSpace cs = sdb.getCollectionSpace(metaCsName);
            DBCollection cl = cs.getCollection(metaClName);

            BSONObject matcher = new BasicBSONObject();
            matcher.put(ObjectMeta.META_BUCKET_ID, bucketId);
            matcher.put(ObjectMeta.META_KEY_NAME, objectName);
            if (versionId != null){
                matcher.put(ObjectMeta.META_VERSION_ID, versionId);
            }
            if (noVersionFlag != null){
                matcher.put(ObjectMeta.META_NO_VERSION_FLAG, noVersionFlag);
            }

            BSONObject queryResult = cl.queryOne(matcher, null, null, null, 0);
            return convertBsonToMeta(queryResult);
        }catch (BaseException e){
            if (e.getErrorCode() == SDBError.SDB_DMS_CS_NOTEXIST.getErrorCode() ||
                    e.getErrorCode() == SDBError.SDB_DMS_NOTEXIST.getErrorCode()) {
                //no cs or cl ,return null
                return null;
            } else {
                logger.error("query meta by name failed. error:"+e.getMessage());
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
    public ObjectMeta queryAndRemoveMeta(String metaCsName, String metaClName, long bucketId, String objectName) throws S3ServerException {
        Sequoiadb sdb = null;
        DBCursor queryResult = null;
        try {
            sdb = sdbDatasourceWrapper.getSequoiadb();
            CollectionSpace cs = sdb.getCollectionSpace(metaCsName);
            DBCollection cl = cs.getCollection(metaClName);

            BSONObject matcher = new BasicBSONObject();
            matcher.put(ObjectMeta.META_BUCKET_ID, bucketId);
            matcher.put(ObjectMeta.META_KEY_NAME, objectName);

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
    public ObjectMeta queryForUpdate(ConnectionDao connection, String metaCsName, String metaClName,
                                     long bucketId, String objectName, Long versionId, Boolean noVersionFlag)
            throws S3ServerException {
        try {
            Sequoiadb sdb = ((SdbConnectionDao)connection).getConnection();
            CollectionSpace cs = sdb.getCollectionSpace(metaCsName);
            DBCollection cl = cs.getCollection(metaClName);

            BSONObject matcher = new BasicBSONObject();
            matcher.put(ObjectMeta.META_BUCKET_ID, bucketId);
            matcher.put(ObjectMeta.META_KEY_NAME, objectName);
            if (versionId != null){
                matcher.put(ObjectMeta.META_VERSION_ID, versionId);
            }
            if (noVersionFlag != null){
                matcher.put(ObjectMeta.META_NO_VERSION_FLAG, noVersionFlag);
            }

            BSONObject order = new BasicBSONObject();
            order.put(ObjectMeta.META_VERSION_ID, -1);

            BSONObject queryResult = cl.queryOne(matcher, null, order, null, DBQuery.FLG_QUERY_FOR_UPDATE);
            return convertBsonToMeta(queryResult);
        }catch (BaseException e){
            if (e.getErrorCode() == SDBError.SDB_DMS_CS_NOTEXIST.getErrorCode() ||
                    e.getErrorCode() == SDBError.SDB_DMS_NOTEXIST.getErrorCode()) {
                //no cs or cl ,return null
                return null;
            } else {
                logger.error("query meta by name failed. error:",e);
                throw new S3ServerException(S3Error.DAO_DB_ERROR, "query meta by name failed");
            }
        } catch (Exception e){
            logger.error("query meta by name failed.");
            throw new S3ServerException(S3Error.DAO_DB_ERROR, "query meta by name failed");
        }
    }

    @Override
    public void updateMeta(ConnectionDao connection, String metaCsName, String metaClName, long bucketId,
                           String objectName, Long versionId, ObjectMeta objectMeta)
            throws S3ServerException {
        try {
            Sequoiadb sdb = ((SdbConnectionDao)connection).getConnection();
            CollectionSpace cs = sdb.getCollectionSpace(metaCsName);
            DBCollection cl = cs.getCollection(metaClName);

            BSONObject matcher = new BasicBSONObject();
            matcher.put(ObjectMeta.META_BUCKET_ID, bucketId);
            matcher.put(ObjectMeta.META_KEY_NAME, objectName);
            if (versionId != null){
                matcher.put(ObjectMeta.META_VERSION_ID, versionId);
            }

            BSONObject updateData = convertMetaToBson(objectMeta);
            BSONObject setUpdate = new BasicBSONObject();
            setUpdate.put(DBParamDefine.MODIFY_SET, updateData);

            cl.update(matcher, setUpdate, null);
        } catch (Exception e){
            logger.error("update meta failed. error:"+e.getMessage());
            throw new S3ServerException(S3Error.DAO_DB_ERROR, "db error.", e);
        }
    }

    @Override
    public void removeMeta(ConnectionDao connection, String metaCsName, String metaClName, long bucketId,
                           String objectName, Long versionId, Boolean noVersionFlag)
            throws S3ServerException {
        try {
            Sequoiadb sdb = ((SdbConnectionDao)connection).getConnection();
            CollectionSpace cs = sdb.getCollectionSpace(metaCsName);
            DBCollection cl = cs.getCollection(metaClName);

            BSONObject matcher = new BasicBSONObject();
            matcher.put(ObjectMeta.META_BUCKET_ID, bucketId);
            matcher.put(ObjectMeta.META_KEY_NAME, objectName);
            if (versionId != null){
                matcher.put(ObjectMeta.META_VERSION_ID, versionId);
            }
            if (noVersionFlag != null){
                matcher.put(ObjectMeta.META_NO_VERSION_FLAG, noVersionFlag);
            }

            cl.delete(matcher);
        } catch (Exception e){
            logger.error("remove meta failed. error:"+e.getMessage());
            throw new S3ServerException(S3Error.DAO_DB_ERROR, "db error.", e);
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
                logger.error("query object number failed. error:"+e.getMessage());
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
    public String getMetaCurCSName( Region region){
        StringBuilder csName = new StringBuilder();

        if (null != region){
            if (region.getMetaLocation() != null){
                csName.append(region.getMetaCSLocation());
            }else {
                csName.append(DBParamDefine.CS_S3);
                csName.append(region.getName());
                csName.append(DBParamDefine.CS_META);
            }
        }else {
            csName.append(config.getMetaCsName());
        }

        return csName.toString();
    }

    @Override
    public String getMetaCurCLName(Region region){
        if (region != null && region.getMetaLocation() != null){
            return region.getMetaCLLocation();
        }else {
            return DaoCollectionDefine.OBJECT_META_LIST;
        }
    }

    @Override
    public String getMetaHisCSName( Region region){
        StringBuilder csName = new StringBuilder();

        if (null != region){
            if (region.getMetaHisLocation() != null){
                csName.append(region.getMetaHisCSLocation());
            }else {
                csName.append(DBParamDefine.CS_S3);
                csName.append(region.getName());
                csName.append(DBParamDefine.CS_META);
            }
        }else {
            csName.append(config.getMetaCsName());
        }

        return csName.toString();
    }

    @Override
    public String getMetaHisCLName(Region region){
        if (region != null && region.getMetaHisLocation() != null){
            return region.getMetaHisCLLocation();
        }else {
            return DaoCollectionDefine.OBJECT_META_LIST_HISTORY;
        }
    }

    private ObjectMeta convertBsonToMeta(BSONObject bsonObject){
        if (null == bsonObject){
            return null;
        }
        ObjectMeta object = new ObjectMeta();
        object.setBucketId((long)bsonObject.get(ObjectMeta.META_BUCKET_ID));
        object.setKey(bsonObject.get(ObjectMeta.META_KEY_NAME).toString());
        object.setLobId((ObjectId)bsonObject.get(ObjectMeta.META_LOB_ID));
        object.setVersionId((Long)bsonObject.get(ObjectMeta.META_VERSION_ID));
        object.setNoVersionFlag((Boolean)bsonObject.get(ObjectMeta.META_NO_VERSION_FLAG));
        if (bsonObject.get(ObjectMeta.META_CS_NAME) != null){
            object.setCsName(bsonObject.get(ObjectMeta.META_CS_NAME).toString());
        }
        if (bsonObject.get(ObjectMeta.META_CL_NAME) != null) {
            object.setClName(bsonObject.get(ObjectMeta.META_CL_NAME).toString());
        }
        object.setLastModified((long)bsonObject.get(ObjectMeta.META_LAST_MODIFIED));
        if (bsonObject.get(ObjectMeta.META_ETAG) != null) {
            object.seteTag(bsonObject.get(ObjectMeta.META_ETAG).toString());
        }
        object.setSize((long) bsonObject.get(ObjectMeta.META_SIZE));
        object.setDeleteMarker((Boolean)bsonObject.get(ObjectMeta.META_DELETE_MARKER));
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
        objectMeta.put(ObjectMeta.META_NO_VERSION_FLAG, meta.getNoVersionFlag());
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
        if (meta.getMetaList() == null){
            objectMeta.put(ObjectMeta.META_LIST, null);
        } else {
            BSONObject xMetaList = new BasicBSONObject(meta.getMetaList());
            objectMeta.put(ObjectMeta.META_LIST, xMetaList);
        }

        return objectMeta;
    }

    @Override
    public void releaseQueryDbCursor(QueryDbCursor queryDbCursor) {
        if (queryDbCursor != null) {
            SdbQueryDbCursor sdbQueryDbCursor = (SdbQueryDbCursor)queryDbCursor;
            sdbDatasourceWrapper.releaseDBCursor(sdbQueryDbCursor.getCursor());
            sdbDatasourceWrapper.releaseSequoiadb(sdbQueryDbCursor.getSdb());
            sdbQueryDbCursor.setCursor(null);
            sdbQueryDbCursor.setSdb(null);
        }
    }

    private void createMetaCSCL(Sequoiadb sdb, Region region, String csMetaName,
                                String clMetaName, Boolean isHistory)
            throws S3ServerException{
        if (region != null && region.getMetaLocation() != null) {
            throw new S3ServerException(S3Error.REGION_LOCATION_NOT_EXIST,
                    "location not exist. csName="+csMetaName+", clName="+clMetaName);
        }else{
            if (!sdb.isCollectionSpaceExist(csMetaName)) {
                BSONObject option = null;
                if (region != null && region.getMetaDomain() != null){
                    option = new BasicBSONObject();
                    option.put("Domain", region.getMetaDomain());
                }
                if(DBParamDefine.CREATE_OK == sdbDatasourceWrapper.createCS(sdb, csMetaName, option)){
                    sequoiadbRegionSpaceDao.insertRegionCSList(sdb, csMetaName, region.getName());
                }
            }

            BSONObject option = generateMetaCLOption();
            sdbDatasourceWrapper.createCL(sdb, csMetaName, clMetaName, option);
            BSONObject indexKey = new BasicBSONObject();
            String indexName = ObjectMeta.META_BUCKET_ID + "+" + ObjectMeta.META_KEY_NAME;
            indexKey.put(ObjectMeta.META_BUCKET_ID, 1);
            indexKey.put(ObjectMeta.META_KEY_NAME, 1);
            if (isHistory) {
                indexKey.put(ObjectMeta.META_VERSION_ID, 1);
                indexName = indexName + "+" + ObjectMeta.META_VERSION_ID;
            }
            sdbDatasourceWrapper.createIndex(sdb, csMetaName, clMetaName,
                    indexName, indexKey, true, true);
        }
    }

    private BSONObject generateMetaCLOption(){
        BSONObject clOption = new BasicBSONObject();

        BSONObject shardingKey = new BasicBSONObject(ObjectMeta.META_KEY_NAME, 1);
        clOption.put("ShardingKey", shardingKey);
        clOption.put("ShardingType", "hash");
        clOption.put("ReplSize", -1);
        clOption.put("AutoSplit", true);

        return clOption;
    }
}
