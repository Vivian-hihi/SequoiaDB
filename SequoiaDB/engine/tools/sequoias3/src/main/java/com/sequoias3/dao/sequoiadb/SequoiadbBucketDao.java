package com.sequoias3.dao.sequoiadb;

import com.sequoiadb.base.CollectionSpace;
import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.DBCursor;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.exception.BaseException;
import com.sequoias3.config.SequoiadbConfig;
import com.sequoias3.core.Bucket;
import com.sequoias3.dao.BucketDao;
import com.sequoias3.dao.DaoCollectionDefine;
import com.sequoias3.exception.S3ServerException;
import org.bson.BSONObject;
import org.bson.BasicBSONObject;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Repository;


@Repository("BucketDao")
public class SequoiadbBucketDao implements BucketDao {
    private static final Logger logger = LoggerFactory.getLogger(SequoiadbUserDao.class);

    @Autowired
    SdbDataSourceWrapper sdbDatasourceWrapper;

    @Autowired
    SequoiadbConfig config;

    @Override
    public void insertBucket(Bucket bucket) throws S3ServerException {

        Sequoiadb sdb = null;
        try {
            sdb = sdbDatasourceWrapper.getSequoiadb();
            CollectionSpace cs = sdb.getCollectionSpace(config.getMetaCsName());
            DBCollection cl = cs.getCollection(DaoCollectionDefine.BUCKET_LIST_COLLECTION);

            BSONObject newBucket = new BasicBSONObject();
            newBucket.put(Bucket.BUCKET_ID, bucket.getBucketId());
            newBucket.put(Bucket.BUCKET_NAME, bucket.getBucketName());
            newBucket.put(Bucket.BUCKET_OWNERID, bucket.getOwnerId());
            newBucket.put(Bucket.BUCKET_CREATETIME, bucket.getCreateDate());
            newBucket.put(Bucket.BUCKET_REGION, bucket.getRegion());
            newBucket.put(Bucket.BUCKET_VERSIONINGSTATUS, bucket.getVersioningStatus());
            newBucket.put(Bucket.BUCKET_DELIMITER, bucket.getDelimiter());

            cl.insert(newBucket);
        }catch (BaseException e) {
            logger.warn("insertBucket failed. errorMessage = " + e.getMessage(), e);
            throw e;
        } finally {
            sdbDatasourceWrapper.releaseSequoiadb(sdb);
        }
    }

    @Override
    public void deleteBucket(String bucketName) throws S3ServerException {
        Sequoiadb sdb = null;
        try {
            sdb = sdbDatasourceWrapper.getSequoiadb();
            CollectionSpace cs = sdb.getCollectionSpace(config.getMetaCsName());
            DBCollection cl = cs.getCollection(DaoCollectionDefine.BUCKET_LIST_COLLECTION);

            BSONObject deleteBucket = new BasicBSONObject();
            deleteBucket.put(Bucket.BUCKET_NAME, bucketName);

            cl.delete(deleteBucket);
        }catch (BaseException e) {
            logger.error("deleteBucket failed. errorMessage = " + e.getMessage(), e);
            throw e;
        } finally {
            sdbDatasourceWrapper.releaseSequoiadb(sdb);
        }
    }

    @Override
    public Bucket[] getBucketListByOwnerID(int ownerId) throws S3ServerException {
        Sequoiadb sdb = null;
        Bucket[] bucketList = null;
        try {
            sdb = sdbDatasourceWrapper.getSequoiadb();
            CollectionSpace cs = sdb.getCollectionSpace(config.getMetaCsName());
            DBCollection cl = cs.getCollection(DaoCollectionDefine.BUCKET_LIST_COLLECTION);

            BSONObject matcher = new BasicBSONObject();
            matcher.put(Bucket.BUCKET_OWNERID, ownerId);

            DBCursor cursor = cl.query(matcher, null,null,null);
            while (cursor.hasNext()){
                BSONObject record = cursor.getNext();
                Bucket bucket = convertBsonToBucket(record);

            }

            return bucketList;
        }catch (BaseException e) {
            logger.error("deleteBucket failed. errorMessage = " + e.getMessage(), e);
            throw e;
        } finally {
            sdbDatasourceWrapper.releaseSequoiadb(sdb);
        }
    }


    private Bucket convertBsonToBucket(BSONObject bsonObject) {
        Bucket bucket = new Bucket();
        if (bsonObject.containsField(Bucket.BUCKET_ID)) {
            bucket.setBucketId((int)bsonObject.get(Bucket.BUCKET_ID));
        }
        if (bsonObject.containsField(Bucket.BUCKET_NAME)){
            bucket.setBucketName(bsonObject.get(Bucket.BUCKET_NAME).toString());
        }
        if (bsonObject.containsField(Bucket.BUCKET_OWNERID)){
            bucket.setOwnerId((int)bsonObject.get(Bucket.BUCKET_OWNERID));
        }
        if (bsonObject.containsField(Bucket.BUCKET_CREATETIME)){
            bucket.setBucketCreateDate(bsonObject.get(Bucket.BUCKET_CREATETIME).toString());
        }
        if (bsonObject.containsField(Bucket.BUCKET_REGION)){
            bucket.setRegion(bsonObject.get(Bucket.BUCKET_REGION).toString());
        }
        if (bsonObject.containsField(Bucket.BUCKET_VERSIONINGSTATUS)){
            bucket.setVersioningStatus(bsonObject.get(Bucket.BUCKET_VERSIONINGSTATUS).toString());
        }
        if (bsonObject.containsField(Bucket.BUCKET_DELIMITER)){
            bucket.setDelimiter(bsonObject.get(Bucket.BUCKET_DELIMITER).toString());
        }

        return bucket;
    }
}