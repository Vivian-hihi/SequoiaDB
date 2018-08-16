package com.sequoias3.service.impl;

import com.sequoiadb.exception.BaseException;
import com.sequoiadb.exception.SDBError;
import com.sequoias3.common.DBParamDefine;
import com.sequoias3.config.BucketConfig;
import com.sequoias3.core.*;
import com.sequoias3.dao.BucketDao;
import com.sequoias3.dao.UserDao;
import com.sequoias3.exception.S3Error;
import com.sequoias3.exception.S3ServerException;
import com.sequoias3.service.BucketService;
import com.sequoias3.utils.DataFormatUtils;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;

import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.List;
import java.util.TimeZone;

@Service
public class BucketServiceImpl implements BucketService {
    private static final Logger logger = LoggerFactory.getLogger(UserServiceImpl.class);

    @Autowired
    BucketDao bucketDao;

    @Autowired
    UserDao userDao;

    @Autowired
    BucketConfig bucketConfig;

    @Override
    public void createBucket(int ownerID, String bucketName) throws S3ServerException {
        int tryTime = DBParamDefine.DB_DUPLICATE_MAX_TIME;

        while (tryTime > 0){
            tryTime--;
            try {
                //check bucketname
                if (!isValidBucketName(bucketName)){
                    throw new S3ServerException(S3Error.BUCKET_INVALID_BUCKETNAME,
                            "Invalid bucket name. bucket name = "+bucketName);
                }
                String newBucketName = bucketName.toLowerCase();

                //check duplicate bucket
                Bucket result = bucketDao.getBucketByName(newBucketName);
                if (null != result){
                    if (result.getOwnerId() == ownerID){
                        throw new S3ServerException(S3Error.BUCKET_ALREADY_OWNEDYOU,
                                "bucket already owned you. bucketname="+bucketName);
                    }else {
                        throw new S3ServerException(S3Error.BUCKET_ALREADY_EXIST,
                                "Bucket already exist. bucketname="+bucketName);
                    }
                }

                //check bucket number
                long bucketLimit=bucketConfig.getLimit();
                long bucketCount = bucketDao.getBucketNumber(ownerID);
                if (bucketCount >= bucketLimit){
                    throw new S3ServerException(S3Error.BUCKET_TOO_MANY_BUCKETS,
                            "You have attempted to create more buckets than allowed. bucket count="
                                    +bucketCount+",bucket limit="+bucketLimit);
                }

                //insert bucket
                Bucket bucket = new Bucket();
                bucket.setBucketId(bucketDao.getMaxID()+1);
                bucket.setBucketName(newBucketName);
                bucket.setOwnerId(ownerID);
                bucket.setDate(new Date());
                bucket.setVersioningStatus(DBParamDefine.DB_AUTO_VERSIONING_STATUS);
                bucket.setDelimiter(DBParamDefine.DB_AUTO_DELIMITER);
                bucketDao.insertBucket(bucket);
                return;
            }catch (BaseException e){
                logger.warn("Create user failed. ", e);
                if (e.getErrorType() == SDBError.SDB_IXM_DUP_KEY.name() && tryTime > 0) {
                    continue;
                } else {
                    throw new S3ServerException(S3Error.BUCKET_CREATE_FAILED,
                            "create bucket failed. bucketname="+bucketName, e);
                }
            }catch (S3ServerException e) {
                throw e;
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

            //check bucket
            Bucket result = bucketDao.getBucketByName(deleteName);
            if (result == null){
                throw new S3ServerException(S3Error.BUCKET_NOT_EXIST,
                        "The specified bucket does not exist. bucket name="+bucketName);
            }
            if (result.getOwnerId() != ownerID){
                throw new S3ServerException(S3Error.ACCESS_DENIED,
                        "You are not owned the specified bucket. bucket="+bucketName+"ownerID+"+ownerID);
            }

            //is bucket empty
            if (!isBucketEmpty(result.getBucketId())){
                throw new S3ServerException(S3Error.BUCKET_NOT_EMPTY,
                        "The bucket you tried to delete is not empty. bucket name="+bucketName);
            }

            //delete bucket
            bucketDao.deleteBucket(deleteName);
        }catch (S3ServerException e) {
            throw e;
        }catch (Exception e){
            throw new S3ServerException(S3Error.BUCKET_DELETE_FAILED,
                    "delete bucket error bucket name = "+bucketName);
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

    private Boolean isValidBucketName(String bucketName){
        if (bucketName.length() < 3 || bucketName.length() > 63){
            return false;
        }
        return true;
    }

    public Boolean isBucketEmpty(long bucketId)throws S3ServerException {
        return true;
    }

}
