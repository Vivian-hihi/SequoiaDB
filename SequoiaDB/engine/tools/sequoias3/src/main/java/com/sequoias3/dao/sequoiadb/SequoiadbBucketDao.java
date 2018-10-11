package com.sequoias3.dao.sequoiadb;

import com.sequoiadb.base.CollectionSpace;
import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.DBCursor;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.exception.SDBError;
import com.sequoias3.config.SequoiadbConfig;
import com.sequoias3.core.Bucket;
import com.sequoias3.dao.BucketDao;
import com.sequoias3.dao.DaoCollectionDefine;
import com.sequoias3.exception.S3Error;
import com.sequoias3.exception.S3ServerException;
import com.sequoias3.utils.DataFormatUtils;
import org.bson.BSONObject;
import org.bson.BasicBSONObject;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Repository;

import java.util.ArrayList;
import java.util.List;

@Repository("BucketDao")
public class SequoiadbBucketDao implements BucketDao {
    private static final Logger logger = LoggerFactory.getLogger(SequoiadbBucketDao.class);

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
            newBucket.put(Bucket.BUCKET_CREATETIME, bucket.getTimeMillis());
            newBucket.put(Bucket.BUCKET_VERSIONINGSTATUS, bucket.getVersioningStatus());
            newBucket.put(Bucket.BUCKET_DELIMITER, bucket.getDelimiter());

            cl.insert(newBucket);
        }catch (BaseException e){
            if (e.getErrorType() == SDBError.SDB_IXM_DUP_KEY.name()) {
                throw new S3ServerException(S3Error.DAO_DUPLICATE_KEY, "Duplicate key.");
            } else {
                throw e;
            }
        }
        catch (Exception e) {
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
    public List<Bucket> getBucketListByOwnerID(int ownerId) throws S3ServerException {
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
            cursor.close();
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
                return (long) (queryResult.get(Bucket.BUCKET_ID));
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
        bucket.setBucketId((long)bsonObject.get(Bucket.BUCKET_ID));
        bucket.setBucketName(bsonObject.get(Bucket.BUCKET_NAME).toString());
        bucket.setOwnerId((int)bsonObject.get(Bucket.BUCKET_OWNERID));
        bucket.setTimeMillis((long)bsonObject.get(Bucket.BUCKET_CREATETIME));
        bucket.setFormatDate(DataFormatUtils.formatDate((long)bsonObject.get(Bucket.BUCKET_CREATETIME)));
        bucket.setVersioningStatus(bsonObject.get(Bucket.BUCKET_VERSIONINGSTATUS).toString());
        bucket.setDelimiter(bsonObject.get(Bucket.BUCKET_DELIMITER).toString());
        return bucket;
    }
}