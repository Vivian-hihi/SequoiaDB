package com.sequoias3.service;

import com.sequoias3.exception.S3ServerException;

public interface AuthorizationService {
    String getNameByAuthorization(String authorization) throws S3ServerException;

    Boolean isAdminUser(String userName) throws S3ServerException;
}
