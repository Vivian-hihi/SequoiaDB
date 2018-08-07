package com.sequoias3.service;

import com.sequoias3.core.UserAuthKey;
import com.sequoias3.exception.S3ServerException;

public interface UserService {
    UserAuthKey createUser(String adminUser, String adminPassword, String newUserName, String role)
            throws S3ServerException;
}
