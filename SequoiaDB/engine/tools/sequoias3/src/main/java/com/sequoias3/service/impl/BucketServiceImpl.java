package com.sequoias3.service.impl;

import com.sequoias3.common.DBParamDefine;
import com.sequoias3.common.VersioningStatusType;
import com.sequoias3.config.BucketConfig;
import com.sequoias3.core.*;
import com.sequoias3.dao.BucketDao;
import com.sequoias3.dao.UserDao;
import com.sequoias3.exception.S3Error;
import com.sequoias3.exception.S3ServerException;
import com.sequoias3.model.GetServiceResult;
import com.sequoias3.service.BucketService;
import com.sequoias3.service.ObjectService;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;

import java.util.List;

@Service
public class BucketServiceImpl implements BucketService {
    private static final Logger logger = LoggerFactory.getLogger(BucketServiceImpl.class);

    @Autowired
    BucketDao bucketDao;

    @Autowired
    UserDao userDao;

    @Autowired
    BucketConfig bucketConfig;

    @Autowired
    ObjectService objectService;

    @Override
    public void createBucket(int ownerID, String bucketName, String region) throws S3ServerException {
        int tryTime = DBParamDefine.DB_DUPLICATE_MAX_TIME;

        //check bucketname
        if (!isValidBucketName(bucketName)){
            throw new S3ServerException(S3Error.BUCKET_INVALID_BUCKETNAME,
                    "Invalid bucket name. bucket name = "+bucketName);
        }
        String newBucketName = bucketName.toLowerCase();
        while (tryTime > 0){
            tryTime--;
            try {
                //check duplicate bucket
                Bucket result = bucketDao.getBucketByName(newBucketName);
                if (null != result){
                    if (result.getOwnerId() == ownerID){
                        throw new S3ServerException(S3Error.BUCKET_ALREADY_OWNED_BY_YOU,
                                "Bucket already owned you. bucketname="+bucketName);
                    }else {
                        throw new S3ServerException(S3Error.BUCKET_ALREADY_EXIST,
                                "Bucket already exist. bucketname="+bucketName);
                    }
                }

                //check bucket number
                long bucketLimit = bucketConfig.getLimit();
                long bucketCount = bucketDao.getBucketNumber(ownerID);
                if (bucketCount >= bucketLimit){
                    throw new S3ServerException(S3Error.BUCKET_TOO_MANY_BUCKETS,
                            "You have attempted to create more buckets than allowed. bucket count="
                                    +bucketCount+", bucket limit="+bucketLimit);
                }

                //insert bucket
                Bucket bucket = new Bucket();
                bucket.setBucketId(bucketDao.getMaxID()+1);
                bucket.setBucketName(newBucketName);
                bucket.setOwnerId(ownerID);
                bucket.setTimeMillis(System.currentTimeMillis());
                bucket.setVersioningStatus(VersioningStatusType.NONE.getName());
                bucket.setDelimiter(DBParamDefine.DB_AUTO_DELIMITER);
                bucket.setRegion(region);
                bucketDao.insertBucket(bucket);
                return;
            }catch (S3ServerException e) {
                logger.warn("Create bucket failed. bucketname={}", bucketName, e);
                if (e.getError().getErrIndex() == S3Error.DAO_DUPLICATE_KEY.getErrIndex() && tryTime > 0) {
                    continue;
                } else {
                    throw e;
                }
            }catch (Exception e){
                throw new S3ServerException(S3Error.BUCKET_CREATE_FAILED,
                        "create bucket failed. bucketname="+bucketName, e);
            }
        }
    }

    @Override
    public void deleteBucket(int ownerID, String bucketName) throws S3ServerException {
        try {
            String deleteName = bucketName.toLowerCase();

            //get and check bucket
            Bucket bucket = getBucket(ownerID, bucketName);

            //is bucket empty
            if (!isBucketEmpty(bucket)){
                throw new S3ServerException(S3Error.BUCKET_NOT_EMPTY,
                        "The bucket you tried to delete is not empty. bucket name="+bucketName);
            }

            //delete bucket
            bucketDao.deleteBucket(deleteName);
        }catch (S3ServerException e) {
            throw e;
        }catch (Exception e){
            throw new S3ServerException(S3Error.BUCKET_DELETE_FAILED,
                    "delete bucket error. bucket name = "+bucketName);
        }
    }

    @Override
    public GetServiceResult getService(User owner) throws S3ServerException {
        GetServiceResult result = new GetServiceResult();
        try {
            //get owner
            result.setOwner(owner);

            //get bucket list
            List<Bucket> bucketArrayList = bucketDao.getBucketListByOwnerID(owner.getUserId());
            if (bucketArrayList.size() > 0){
                result.setBuckets(bucketArrayList);
            }

            return result;
        }catch (S3ServerException e) {
            throw e;
        }catch (Exception e){
            throw new S3ServerException(S3Error.BUCKET_GET_SERVICE_FAILED,
                    "Get bucket list error. ownerID="+owner.getUserId());
        }
    }

    @Override
    public Bucket getBucket(int ownerID, String bucketName)
            throws S3ServerException{
        Bucket bucket = bucketDao.getBucketByName(bucketName.toLowerCase());
        if (bucket == null){
            throw new S3ServerException(S3Error.BUCKET_NOT_EXIST,
                    "The specified bucket does not exist. bucket name="+bucketName);
        }
        if (bucket.getOwnerId() != ownerID){
            throw new S3ServerException(S3Error.ACCESS_DENIED,
                    "You are not owned the specified bucket. bucket="+bucketName+"ownerID+"+ownerID);
        }

        return bucket;
    }

    @Override
    public void deleteBucketForce(Bucket bucket) throws S3ServerException {
        try {
            while (!isBucketEmpty(bucket)) {
                //delete objects in the bucket
                objectService.deleteObjectByBucket(bucket);
            }

            //delete bucket
            bucketDao.deleteBucket(bucket.getBucketName());
        }catch (S3ServerException e) {
            throw e;
        }catch (Exception e){
            throw new S3ServerException(S3Error.BUCKET_DELETE_FAILED,
                    "delete bucket error. bucket name = "+bucket.getBucketName());
        }
    }

    private Boolean isValidBucketName(String bucketName){
        if (bucketName.length() < 3 || bucketName.length() > 63){
            return false;
        }
        if (bucketName.equalsIgnoreCase("users")){
            return false;
        }
        return true;
    }

    public Boolean isBucketEmpty(Bucket bucket)throws S3ServerException {
        if (objectService.getObjectNumberByBucketId(bucket) > 0){
            return false;
        }
        return true;
    }
}
