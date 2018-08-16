package com.sequoias3.dao.sequoiadb;

import com.sequoiadb.base.CollectionSpace;
import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.DBCursor;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.exception.BaseException;
import com.sequoias3.common.DBParamDefine;
import com.sequoias3.config.SequoiadbConfig;
import com.sequoias3.core.Bucket;
import com.sequoias3.dao.BucketDao;
import com.sequoias3.dao.DaoCollectionDefine;
import com.sequoias3.exception.S3ServerException;
import com.sequoias3.utils.DataFormatUtils;
import org.bson.BSONObject;
import org.bson.BasicBSONObject;
import org.bson.types.BSONTimestamp;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Repository;

import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Date;

@Repository("BucketDao")
public class SequoiadbBucketDao implements BucketDao {
    private static final Logger logger = LoggerFactory.getLogger(SequoiadbUserDao.class);

    @Autowired
    SdbDataSourceWrapper sdbDatasourceWrapper;

    @Autowired
    SequoiadbConfig config;

    @Override
    public void insertBucket(Bucket bucket) throws S3ServerException,ParseException {

        Sequoiadb sdb = null;
        try {
            sdb = sdbDatasourceWrapper.getSequoiadb();
            CollectionSpace cs = sdb.getCollectionSpace(config.getMetaCsName());
            DBCollection cl = cs.getCollection(DaoCollectionDefine.BUCKET_LIST_COLLECTION);

            BSONObject newBucket = new BasicBSONObject();
            newBucket.put(Bucket.BUCKET_ID, bucket.getBucketId());
            newBucket.put(Bucket.BUCKET_NAME, bucket.getBucketName());
            newBucket.put(Bucket.BUCKET_OWNERID, bucket.getOwnerId());
            BSONTimestamp timestamp = new BSONTimestamp(bucket.getDate());
            newBucket.put(Bucket.BUCKET_CREATETIME, timestamp);
            newBucket.put(Bucket.BUCKET_VERSIONINGSTATUS, bucket.getVersioningStatus());
            newBucket.put(Bucket.BUCKET_DELIMITER, bucket.getDelimiter());

            cl.insert(newBucket);
        }catch (Exception e) {
            logger.error("insertBucket failed. errorMessage = " + e.getMessage(), e);
            throw e;
        }finally {
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
        }catch (Exception e) {
            logger.error("deleteBucket failed. errorMessage = " + e.getMessage(), e);
            throw e;
        }finally {
            sdbDatasourceWrapper.releaseSequoiadb(sdb);
        }
    }

    @Override
    public Bucket getBucketByName(String bucketName) throws S3ServerException {
        Sequoiadb sdb = null;
        try {
            sdb = sdbDatasourceWrapper.getSequoiadb();
            CollectionSpace cs = sdb.getCollectionSpace(config.getMetaCsName());
            DBCollection cl = cs.getCollection(DaoCollectionDefine.BUCKET_LIST_COLLECTION);

            BSONObject matcher = new BasicBSONObject();
            matcher.put(Bucket.BUCKET_NAME, bucketName);

            BSONObject queryResult = cl.queryOne(matcher,null,null,null,0);

            if (null == queryResult) {
                return null;
            }

            return convertBsonToBucket(queryResult);
        }catch (Exception e) {
            logger.error("getBucketByName failed. errorMessage = " + e.getMessage(), e);
            throw e;
        }  finally {
            sdbDatasourceWrapper.releaseSequoiadb(sdb);
        }
    }

    @Override
    public ArrayList<Bucket> getBucketListByOwnerID(int ownerId) throws S3ServerException {
        Sequoiadb sdb = null;
        ArrayList<Bucket> bucketList = new ArrayList<Bucket>();
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
                bucketList.add(bucket);
            }

            bucketList.trimToSize();
            return bucketList;
        }catch (Exception e) {
            logger.error("deleteBucket failed. errorMessage = " + e.getMessage(), e);
            throw e;
        } finally {
            sdbDatasourceWrapper.releaseSequoiadb(sdb);
        }
    }

    @Override
    public long getMaxID() throws S3ServerException {
        Sequoiadb sdb = null;
        try {
            sdb = sdbDatasourceWrapper.getSequoiadb();
            CollectionSpace cs = sdb.getCollectionSpace(config.getMetaCsName());
            DBCollection cl = cs.getCollection(DaoCollectionDefine.BUCKET_LIST_COLLECTION);

            BSONObject selector = new BasicBSONObject();
            selector.put(Bucket.BUCKET_ID, "");
            BSONObject orderBy = new BasicBSONObject();
            orderBy.put(Bucket.BUCKET_ID, -1);
            BSONObject queryResult = cl.queryOne(null, selector, orderBy, null, 0);

            if (null != queryResult) {
                long number = (long) (queryResult.get(Bucket.BUCKET_ID));
                return number;
            } else {
                return 0;
            }
        }catch (Exception e) {
            logger.error("getMaxID failed. errorMessage = " + e.getMessage(), e);
            throw e;
        }finally{
            sdbDatasourceWrapper.releaseSequoiadb(sdb);
        }
    }

    @Override
    public long getBucketNumber(int ownerID) throws S3ServerException{
        Sequoiadb sdb = null;
        try {
            sdb = sdbDatasourceWrapper.getSequoiadb();
            CollectionSpace cs = sdb.getCollectionSpace(config.getMetaCsName());
            DBCollection cl = cs.getCollection(DaoCollectionDefine.BUCKET_LIST_COLLECTION);

            BSONObject matcher = new BasicBSONObject();
            matcher.put(Bucket.BUCKET_OWNERID, ownerID);

            return cl.getCount(matcher);

        }catch (Exception e) {
            logger.error("getBucketNumber failed. errorMessage = " + e.getMessage(), e);
            throw e;
        }finally {
            sdbDatasourceWrapper.releaseSequoiadb(sdb);
        }
    }

    private Bucket convertBsonToBucket(BSONObject bsonObject) {
        Bucket bucket = new Bucket();
        if (bsonObject.containsField(Bucket.BUCKET_ID)) {
            bucket.setBucketId((long)bsonObject.get(Bucket.BUCKET_ID));
        }
        if (bsonObject.containsField(Bucket.BUCKET_NAME)){
            bucket.setBucketName(bsonObject.get(Bucket.BUCKET_NAME).toString());
        }
        if (bsonObject.containsField(Bucket.BUCKET_OWNERID)){
            bucket.setOwnerId((int)bsonObject.get(Bucket.BUCKET_OWNERID));
        }
        if (bsonObject.containsField(Bucket.BUCKET_CREATETIME)){
            BSONTimestamp createDate= (BSONTimestamp) bsonObject.get(Bucket.BUCKET_CREATETIME);
            bucket.setDate(createDate.getDate());
            bucket.setFormatDate(DataFormatUtils.formatDate(createDate.getDate()));
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