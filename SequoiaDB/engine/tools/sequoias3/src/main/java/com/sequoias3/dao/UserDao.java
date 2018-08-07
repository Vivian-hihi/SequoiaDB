package com.sequoias3.dao;

import com.sequoias3.core.User;
import com.sequoias3.exception.S3ServerException;

public interface UserDao {
    void insertUser(User user) throws S3ServerException;

    void deleteUser(String userName) throws S3ServerException;

    User getUser(String userName) throws S3ServerException;
}
