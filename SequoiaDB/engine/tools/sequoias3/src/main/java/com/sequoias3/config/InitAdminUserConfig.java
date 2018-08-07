package com.sequoias3.config;

import com.sequoias3.common.InitAdminUserDefine;
import com.sequoias3.core.User;
import com.sequoias3.dao.UserDao;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.ApplicationArguments;
import org.springframework.boot.ApplicationRunner;
import org.springframework.stereotype.Component;

@Component
public class InitAdminUserConfig implements ApplicationRunner {

    @Autowired
    private UserDao userDao;

    @Override
    public void run (ApplicationArguments applicationArguments)
            throws Exception{
        if(userDao.getMaxID() == 0) {
            User u = new User();
            u.setUserName(InitAdminUserDefine.ADMIN_NAME);
            u.setUserId(1);
            u.setRole(InitAdminUserDefine.ADMIN_ROLE);
            u.setAccessKeyID(InitAdminUserDefine.ADMIN_ACCESSKEYID);
            u.setSecretAccessKey(InitAdminUserDefine.ADMIN_SECRETACCESSKEY);
            userDao.insertUser(u);
        }
    }
}
