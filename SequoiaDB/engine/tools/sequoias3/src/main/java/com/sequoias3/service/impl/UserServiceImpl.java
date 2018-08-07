package com.sequoias3.service.impl;

import com.sequoias3.core.AccessKeys;
import com.sequoias3.exception.*;
import com.sequoias3.utils.IDUtils;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;

import com.sequoias3.core.User;
import com.sequoias3.dao.UserDao;
import com.sequoias3.service.UserService;

import java.util.regex.Pattern;

@Service
public class UserServiceImpl implements UserService {
    @Autowired
    private UserDao userDao;

    @Override
    public AccessKeys createUser(String newUserName,
                                  String role) throws S3ServerException {
        try {
            //       1.check username & role
            if (!isValidUsername(newUserName)){throw new S3ServerException(S3Error.USER_CREATE_NAME_INVALID, "Username is invalid.");            }
            if (!isValidRole(role)){throw new S3ServerException(S3Error.USER_CREATE_ROLE_INVALID, "role is invalid.");            }
            if (null != userDao.getUserByName(newUserName.toLowerCase())){throw new S3ServerException(S3Error.USER_CREATE_EXIST, "The username is exist."); }

            //       3.generate accesskey etc.
            String accessKeyID = IDUtils.getAccessKeyID();
            if (null != userDao.getUserByAccessKeyID(accessKeyID)) accessKeyID = IDUtils.getAccessKeyID();
            String secretAccessKey = IDUtils.getSecretKey();

            //       4.set user attribute
            User u = new User();
            u.setUserName(newUserName.toLowerCase());
            u.setUserId(userDao.getMaxID()+1);
            u.setRole(null == role ? "normal":role);
            u.setAccessKeyID(accessKeyID);
            u.setSecretAccessKey(secretAccessKey);
            userDao.insertUser(u);

            AccessKeys accessKeys = new AccessKeys();
            accessKeys.setAccessKeyID(accessKeyID);
            accessKeys.setSecretAccessKey(secretAccessKey);
            return accessKeys;
        }catch (S3ServerException e) {
            throw e ;
        }catch (Exception e) {
            throw new S3CreateUserException("create user failed", e);
        }
    }

    @Override
    public AccessKeys updateUser(String userName)throws S3ServerException{
        try {
            //       1.check username
            if (null == userDao.getUserByName(userName.toLowerCase())) {
                throw new S3ServerException(S3Error.USER_NOT_EXIST,"The username is not exit.");
            }

            //       2.generate new keys
            String accessKeyID = IDUtils.getAccessKeyID();
            if (null != userDao.getUserByAccessKeyID(accessKeyID)) accessKeyID = IDUtils.getAccessKeyID();
            String secretAccessKey = IDUtils.getSecretKey();

            userDao.updateUserKeys(userName.toLowerCase(), accessKeyID, secretAccessKey);

            AccessKeys accessKeys = new AccessKeys();
            accessKeys.setAccessKeyID(accessKeyID);
            accessKeys.setSecretAccessKey(secretAccessKey);
            return accessKeys;
        }catch (S3ServerException e) {
            throw e ;
        }catch (Exception e) {
            throw new S3UpdateUserException("update user failed", e);
        }
    }

    @Override
    public AccessKeys getUser(String userName)
            throws S3ServerException{
        try {
            //       2.check username
            User user = userDao.getUserByName(userName.toLowerCase());
            if (null == user) {
                throw new S3ServerException(S3Error.USER_NOT_EXIST, "The username is not exit.");
            }

            String accessKeyID = user.getAccessKeyID();
            String secretAccessKey = user.getSecretAccessKey();

            AccessKeys accessKeys = new AccessKeys();
            accessKeys.setAccessKeyID(accessKeyID);
            accessKeys.setSecretAccessKey(secretAccessKey);
            return accessKeys;
        }catch (S3ServerException e) {
            throw e ;
        }catch (Exception e) {
            throw new S3ServerException(S3Error.USER_GET_FAILED, "update user failed");
        }
    }

    @Override
    public void deleteUser(String userName)
            throws S3ServerException{
        try{
            //       1.check username
            if (null == userDao.getUserByName(userName.toLowerCase())){throw new S3ServerException(S3Error.USER_NOT_EXIST, "The username is not exit."); }
            if (isLastAdminUser(userName.toLowerCase())){throw new S3ServerException(S3Error.USER_DELETE_LAST_ADMIN,"Last admin user cannot be delete.");}
            //       2.check bucket

            //       3.delete buckets

            //       4.delete user
            userDao.deleteUser(userName.toLowerCase());
        }catch (S3ServerException e){
            throw e;
        }catch(Exception e) {
            throw new S3DeleteUserException("delete user failed:newUserName=" + userName, e);
        }
    }

    private Boolean isValidUsername(String userName){
        if (userName.length() > 64) return false;
        return Pattern.compile("^[a-zA-Z0-9+=_,.@\\-]+$").matcher(userName).matches();
    }

    private Boolean isValidRole(String role){
        if (null == role) return true;

        if (role.equals("admin") || role.equals("normal")) {
            return true;
        }
        else {
            return false;
        }
    }

    public Boolean isLastAdminUser(String userName) throws S3ServerException{
        User admin = userDao.getUserByName(userName);
        if (!admin.getRole().equals("admin")) return false;

        if (userDao.getCountByRole("admin") <= 1){
            return true;
        }
        else {
            return false;
        }
    }
}
