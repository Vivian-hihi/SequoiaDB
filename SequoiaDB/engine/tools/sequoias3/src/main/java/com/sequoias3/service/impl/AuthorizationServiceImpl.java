package com.sequoias3.service.impl;

import com.sequoias3.core.User;
import com.sequoias3.dao.UserDao;
import com.sequoias3.exception.S3Error;
import com.sequoias3.exception.S3ServerException;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;
import com.sequoias3.service.AuthorizationService;

@Service
public class AuthorizationServiceImpl implements AuthorizationService{
    private static final Logger logger = LoggerFactory.getLogger(AuthorizationServiceImpl.class);
    @Autowired
    UserDao userDao;

    @Override
    public String getNameByAuthorization(String authorization) throws S3ServerException{
        //       1.get access key id
        String accessKeyId = "";
        int beginIndex = authorization.indexOf("Credential=");
        if(beginIndex == -1)  throw new S3ServerException(S3Error.INVALID_AUTHORIZATION, "Invalid user");

        int endIndex = authorization.indexOf("/",beginIndex);
        if(endIndex != -1) {
            accessKeyId = authorization.substring(beginIndex+11,endIndex);
        }else {
            accessKeyId = authorization.substring(beginIndex+11);
        }

        //       2.check access key
        User user = userDao.getUserByAccessKeyID(accessKeyId);
        if (null == user){  throw new S3ServerException(S3Error.INVALID_AUTHORIZATION, "Invalid user");}

        //       3.check signature

        return user.getUserName();
    }

    @Override
    public Boolean isAdminUser(String userName) throws S3ServerException{
        User admin = userDao.getUserByName(userName);
        return admin.getRole().equals("admin") ? true:false;
    }

}
