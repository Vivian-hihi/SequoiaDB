package com.sequoias3.service.impl;

import com.sequoiadb.exception.BaseException;
import com.sequoiadb.exception.SDBError;
import com.sequoias3.common.DBParamDefine;
import com.sequoias3.common.InitAdminUserDefine;
import com.sequoias3.common.RestParamDefine;
import com.sequoias3.core.AccessKeys;
import com.sequoias3.core.User;
import com.sequoias3.dao.UserDao;
import com.sequoias3.exception.*;
import com.sequoias3.service.UserService;
import com.sequoias3.utils.IDUtils;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;

import java.util.regex.Pattern;

@Service
public class UserServiceImpl implements UserService {
    private static final Logger logger = LoggerFactory.getLogger(UserServiceImpl.class);

    @Autowired
    private UserDao userDao;

    @Override
    public AccessKeys createUser(String newUserName,
                                 String role) throws S3ServerException, BaseException {
        int tryTime = DBParamDefine.DB_DUPLICATE_MAX_TIME;

        while (tryTime > 0) {
            tryTime--;
            try {
                //       1.check username & role
                if (!isValidUsername(newUserName)) {
                    logger.info("Username is invalid. username=" + newUserName);
                    throw new S3ServerException(S3Error.USER_CREATE_NAME_INVALID, "Username is invalid. username=" + newUserName);
                }
                String userName = newUserName.toLowerCase();
                if (!isValidRole(role)) {
                    logger.info("role is invalid. role=" + role);
                    throw new S3ServerException(S3Error.USER_CREATE_ROLE_INVALID, "role is invalid. role=" + role);
                }
                if (null != userDao.getUserByName(userName)) {
                    logger.info("The username is exist. username = " + newUserName);
                    throw new S3ServerException(S3Error.USER_CREATE_EXIST, "The username is exist. username = " + userName);
                }

                //       3.generate accesskey etc.
                String accessKeyID = IDUtils.getAccessKeyID();
                String secretAccessKey = IDUtils.getSecretKey();

                //       4.set user attribute
                User u = new User();
                u.setUserName(userName);
                u.setUserId(userDao.getMaxID() + 1);
                u.setRole(null == role ? RestParamDefine.ROLE_NORMAL : role);
                u.setAccessKeyID(accessKeyID);
                u.setSecretAccessKey(secretAccessKey);
                userDao.insertUser(u);
                return new AccessKeys(accessKeyID, secretAccessKey);

            } catch (BaseException e) {
                logger.warn("Create user failed. ", e);
                if (e.getErrorType() == SDBError.SDB_IXM_DUP_KEY.name() && tryTime > 0) {
                    continue;
                } else {
                    throw new S3CreateUserException("create user failed.username=" + newUserName, e);
                }
            } catch (S3ServerException e) {
                throw e;
            } catch (Exception e) {
                logger.error("Create user failed.username = " + newUserName, e);
                throw new S3CreateUserException("create user failed", e);
            }
        }
        logger.error("IDs duplicate too times. username=" + newUserName);
        throw new S3ServerException(S3Error.USER_CREATE_FAILED, "IDs duplicate too times");
    }

    @Override
    public AccessKeys updateUser(String updateUserName) throws S3ServerException {
        try {
            //       1.check username
            String userName = updateUserName.toLowerCase();
            if (null == userDao.getUserByName(userName)) {
                logger.info("The username is not exist. username = " + updateUserName);
                throw new S3ServerException(S3Error.USER_NOT_EXIST, "The username is not exit.");
            }

            //       2.generate new keys
            String accessKeyID = IDUtils.getAccessKeyID();
            if (null != userDao.getUserByAccessKeyID(accessKeyID)){
                accessKeyID = IDUtils.getAccessKeyID();
            }
            String secretAccessKey = IDUtils.getSecretKey();

            userDao.updateUserKeys(userName, accessKeyID, secretAccessKey);

            AccessKeys accessKeys = new AccessKeys(accessKeyID, secretAccessKey);
            return accessKeys;
        } catch (S3ServerException e) {
            throw e;
        } catch (Exception e) {
            logger.error("Create user failed. ", e);
            throw new S3UpdateUserException("update user failed", e);
        }
    }

    @Override
    public AccessKeys getUser(String userName)
            throws S3ServerException {
        try {
            User user = userDao.getUserByName(userName.toLowerCase());
            if (null == user) {
                logger.info("The username is not exist. username=" + userName);
                throw new S3ServerException(S3Error.USER_NOT_EXIST, "The username is not exist.");
            }

            return new AccessKeys(user.getAccessKeyID(), user.getSecretAccessKey());
        } catch (S3ServerException e) {
            throw e;
        } catch (Exception e) {
            logger.error("Get user failed. username=" + userName);
            throw new S3ServerException(S3Error.USER_GET_FAILED, "get user failed");
        }
    }

    @Override
    public void deleteUser(String deleteUserName)
            throws S3ServerException {
        try {
            //       1.check username
            String userName = deleteUserName.toLowerCase();
            User user = userDao.getUserByName(userName);
            if (null == user) {
                logger.info("The username is not exit. username=" + deleteUserName);
                throw new S3ServerException(S3Error.USER_NOT_EXIST, "The username is not exit.");
            }

            if (userName.equals(InitAdminUserDefine.ADMIN_NAME)) {
                logger.info("Init admin user cannot be delete.");
                throw new S3ServerException(S3Error.USER_DELETE_LAST_ADMIN, "Last admin user cannot be delete.");
            }
            //       2.check bucket

            //       3.delete buckets

            //       4.delete user
            userDao.deleteUser(userName);
        } catch (S3ServerException e) {
            throw e;
        } catch (Exception e) {
            logger.error("Delete user failed. username=" + deleteUserName);
            throw new S3DeleteUserException("delete user failed:username=" + deleteUserName, e);
        }
    }

    private Boolean isValidUsername(String userName) {
        if (userName.length() > 64) return false;
        return Pattern.compile("^[a-zA-Z0-9+=_,.@\\-]+$").matcher(userName).matches();
    }

    private Boolean isValidRole(String role) {
        if (null == role) return true;

        if (role.equals("admin") || role.equals("normal")) {
            return true;
        } else {
            return false;
        }
    }

}
