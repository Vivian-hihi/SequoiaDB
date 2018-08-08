package com.sequoias3.utils;

import com.sequoias3.common.RestParamDefine;
import com.sequoias3.core.User;
import com.sequoias3.dao.UserDao;
import com.sequoias3.exception.S3Error;
import com.sequoias3.exception.S3ServerException;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Component;

@Component
public class RestUtils {
    private static final Logger logger = LoggerFactory.getLogger(RestUtils.class);

    @Autowired
    UserDao userDao;

    public User getOperatorByAuthorization(String authorization) throws S3ServerException {
        //       1.get access key id
        String accessKeyId = "";
        int beginIndex = authorization.indexOf(RestParamDefine.REST_CREDENTIAL);
        if (beginIndex == -1) {
            throw new S3ServerException(S3Error.INVALID_ACCESSKEYID, "Invalid accessKeyId. authorization = " + authorization);
        }

        int endIndex = authorization.indexOf(RestParamDefine.REST_DELIMITER, beginIndex);
        if (endIndex != -1) {
            accessKeyId = authorization.substring(beginIndex + RestParamDefine.REST_CREDENTIAL.length(), endIndex);
        } else {
            accessKeyId = authorization.substring(beginIndex + RestParamDefine.REST_CREDENTIAL.length());
        }

        //       2.check access key
        User user = userDao.getUserByAccessKeyID(accessKeyId);
        if (null == user) {
            throw new S3ServerException(S3Error.INVALID_ACCESSKEYID, "Invalid accessKeyId. accessKeyId = " + accessKeyId);
        }

        //       3.check signature

        return user;
    }
}
