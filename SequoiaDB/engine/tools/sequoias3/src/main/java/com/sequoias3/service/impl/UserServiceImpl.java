package com.sequoias3.service.impl;

import com.sun.istack.internal.NotNull;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;

import com.sequoias3.core.User;
import com.sequoias3.core.UserAuthKey;
import com.sequoias3.dao.UserDao;
import com.sequoias3.exception.S3CreateUserException;
import com.sequoias3.exception.S3ServerException;
import com.sequoias3.service.UserService;

@Service
public class UserServiceImpl implements UserService {
    @Autowired
    private UserDao userDao;

    @Override
    public UserAuthKey createUser(String adminUser, String adminPassword, String newUserName,
            String role) throws S3ServerException {
        try {
            // TODO: 1.check admin user and password
            //       2.check username & role
            

            //       3.generate accesskey etc.
            String accessKeyID = "accessKeyID";
            String accessKeys = "accessKeys";
            String secretAccessKey = "secretAccessKey";

            User u = new User();
            u.setUserName(newUserName);
            u.setRole(role);
            // TODO: 4.set other attributes
            userDao.insertUser(u);

            UserAuthKey authKey = new UserAuthKey();
            authKey.setAccessKeyID(accessKeyID);
            authKey.setSecretAccessKey(secretAccessKey);
            return authKey;
        }
        catch (Exception e) {
            throw new S3CreateUserException("create user failed:newUserName=" + newUserName
                    + ",role=" + role, e);
        }
    }
}
