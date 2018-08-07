package com.sequoias3.dao.sequoiadb;

import org.bson.BSONObject;
import org.bson.BasicBSONObject;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Repository;

import com.sequoiadb.base.CollectionSpace;
import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.Sequoiadb;
import com.sequoias3.config.SequoiadbConfig;
import com.sequoias3.core.User;
import com.sequoias3.dao.UserDao;
import com.sequoias3.exception.S3ServerException;

@Repository("UserDao")
public class SequoiadbUserDao implements UserDao {
    @Autowired
    SdbDataSourceWrapper sdbDatasourceWrapper;

    @Autowired
    SequoiadbConfig config;

    @Override
    public void insertUser(User user) throws S3ServerException {
        Sequoiadb sdb = null;
        try {
            sdb = sdbDatasourceWrapper.getSequoiadb();
            CollectionSpace cs = sdb.getCollectionSpace(config.getMetaCsName());
            DBCollection cl = cs.getCollection(config.getMetaClName());

            BSONObject newUser = new BasicBSONObject();
            newUser.put(User.JSON_KEY_USERNAME, user.getUserName());
            newUser.put(User.JSON_KEY_USERID, user.getUserId());
            newUser.put(User.JSON_KEY_PASSWORD, user.getPassword());
            newUser.put(User.JSON_KEY_ROLE, user.getRole());
            newUser.put(User.JSON_KEY_ACCESS_KEY_ID, user.getAccessKeyID());
            newUser.put(User.JSON_KEY_SECRET_ACCESS_KEY, user.getSecretAccessKey());

            cl.insert(newUser);
        }
        finally {
            sdbDatasourceWrapper.releaseSequoiadb(sdb);
        }
    }

    @Override
    public void deleteUser(String userName) throws S3ServerException {
        // TODO Auto-generated method stub

    }

    @Override
    public User getUser(String userName) throws S3ServerException {
        // TODO Auto-generated method stub
        return null;
    }
}
