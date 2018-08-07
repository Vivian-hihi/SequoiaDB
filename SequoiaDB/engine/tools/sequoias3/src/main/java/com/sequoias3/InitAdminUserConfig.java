package com.sequoias3;

import com.sequoias3.common.InitAdminUserDefine;
import com.sequoias3.core.User;
import com.sequoias3.dao.UserDao;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.ApplicationArguments;
import org.springframework.boot.ApplicationRunner;
import org.springframework.stereotype.Component;

@Component
public class InitAdminUserConfig implements ApplicationRunner {
    private static final Logger logger = LoggerFactory.getLogger(InitAdminUserConfig.class);
    @Autowired
    private UserDao userDao;

    @Override
    public void run(ApplicationArguments applicationArguments)
            throws Exception {
        try {
            if (userDao.getMaxID() == 0) {
                User u = new User();
                u.setUserName(InitAdminUserDefine.ADMIN_NAME);
                u.setUserId(1);
                u.setRole(InitAdminUserDefine.ADMIN_ROLE);
                u.setAccessKeyID(InitAdminUserDefine.ADMIN_ACCESSKEYID);
                u.setSecretAccessKey(InitAdminUserDefine.ADMIN_SECRETACCESSKEY);
                userDao.insertUser(u);
                logger.info("Insert init administrator into db.");
            }
        } catch (Exception e) {
            logger.error("Init admin user failed.");
        }
    }
}
