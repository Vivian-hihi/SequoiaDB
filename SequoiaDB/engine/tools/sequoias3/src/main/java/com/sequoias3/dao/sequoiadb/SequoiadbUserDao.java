package com.sequoias3.dao.sequoiadb;

import com.sequoias3.common.RestParamDefine;
import com.sequoias3.dao.DaoCollectionDefine;
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

import javax.jws.soap.SOAPBinding;

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
            DBCollection cl = cs.getCollection(DaoCollectionDefine.USER_LIST_COLLECTION);

            BSONObject newUser = new BasicBSONObject();
            newUser.put(User.JSON_KEY_USERID, user.getUserId());
            newUser.put(User.JSON_KEY_USERNAME, user.getUserName());
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
        Sequoiadb sdb = null;
        try {
            sdb = sdbDatasourceWrapper.getSequoiadb();
            CollectionSpace cs = sdb.getCollectionSpace(config.getMetaCsName());
            DBCollection cl = cs.getCollection(DaoCollectionDefine.USER_LIST_COLLECTION);

            BSONObject deleteUser = new BasicBSONObject();
            deleteUser.put(User.JSON_KEY_USERNAME, userName);

            cl.delete(deleteUser);
        }
        finally {
            sdbDatasourceWrapper.releaseSequoiadb(sdb);
        }
    }

    @Override
    public void updateUserKeys(String userName, String accessKeyId, String secretAccessKey)
        throws S3ServerException{
        Sequoiadb sdb = null;
        try {
            sdb = sdbDatasourceWrapper.getSequoiadb();
            CollectionSpace cs = sdb.getCollectionSpace(config.getMetaCsName());
            DBCollection cl = cs.getCollection(DaoCollectionDefine.USER_LIST_COLLECTION);

            BSONObject matcher = new BasicBSONObject();
            matcher.put(User.JSON_KEY_USERNAME, userName);
            BSONObject modifier = new BasicBSONObject();
            modifier.put(User.JSON_KEY_ACCESS_KEY_ID, accessKeyId);
            modifier.put(User.JSON_KEY_SECRET_ACCESS_KEY, secretAccessKey);
            BSONObject setModifier = new BasicBSONObject();
            setModifier.put(DaoCollectionDefine.MODIFY_PREFIX, modifier);

            cl.update(matcher,setModifier,null);
        }
        finally {
            sdbDatasourceWrapper.releaseSequoiadb(sdb);
        }
    }

    @Override
    public User getUserByName(String userName) throws S3ServerException {
        // TODO Auto-generated method stub
        Sequoiadb sdb = null;
        try {
            sdb = sdbDatasourceWrapper.getSequoiadb();
            CollectionSpace cs = sdb.getCollectionSpace(config.getMetaCsName());
            DBCollection cl = cs.getCollection(DaoCollectionDefine.USER_LIST_COLLECTION);

            BSONObject matcher = new BasicBSONObject();
            matcher.put(User.JSON_KEY_USERNAME, userName);
            BSONObject queryResult = cl.queryOne(matcher,null,null,null,0);
            User user = new User();

            if (null == queryResult){
                return  null;
            }

            if (queryResult.containsField(User.JSON_KEY_USERNAME)){user.setUserName(queryResult.get(User.JSON_KEY_USERNAME).toString());}
            if (queryResult.containsField(User.JSON_KEY_USERID)){user.setUserId((int)(queryResult.get(User.JSON_KEY_USERID)));}
            if (queryResult.containsField(User.JSON_KEY_ROLE)){user.setRole(queryResult.get(User.JSON_KEY_ROLE).toString());}
            if (queryResult.containsField(User.JSON_KEY_ACCESS_KEY_ID)){user.setAccessKeyID(queryResult.get(User.JSON_KEY_ACCESS_KEY_ID).toString());}
            if (queryResult.containsField(User.JSON_KEY_SECRET_ACCESS_KEY)){user.setSecretAccessKey(queryResult.get(User.JSON_KEY_SECRET_ACCESS_KEY).toString());}
            return user;
        }
        finally {
            sdbDatasourceWrapper.releaseSequoiadb(sdb);
        }
    }

    @Override
    public User getUserByAccessKeyID(String accessKeyID) throws S3ServerException{
        Sequoiadb sdb = null;
        try {
            sdb = sdbDatasourceWrapper.getSequoiadb();
            CollectionSpace cs = sdb.getCollectionSpace(config.getMetaCsName());
            DBCollection cl = cs.getCollection(DaoCollectionDefine.USER_LIST_COLLECTION);

            BSONObject matcher = new BasicBSONObject();
            matcher.put(User.JSON_KEY_ACCESS_KEY_ID, accessKeyID);
            BSONObject queryResult = cl.queryOne(matcher,null,null,null,0);
            User user = new User();

            if (null == queryResult){
                return  null;
            }

            if (queryResult.containsField(User.JSON_KEY_USERNAME)){user.setUserName(queryResult.get(User.JSON_KEY_USERNAME).toString());}
            if (queryResult.containsField(User.JSON_KEY_USERID)){user.setUserId((int)(queryResult.get(User.JSON_KEY_USERID)));}
            if (queryResult.containsField(User.JSON_KEY_ROLE)){user.setRole(queryResult.get(User.JSON_KEY_ROLE).toString());}
            if (queryResult.containsField(User.JSON_KEY_ACCESS_KEY_ID)){user.setRole(queryResult.get(User.JSON_KEY_ACCESS_KEY_ID).toString());}
            if (queryResult.containsField(User.JSON_KEY_SECRET_ACCESS_KEY)){user.setSecretAccessKey(queryResult.get(User.JSON_KEY_SECRET_ACCESS_KEY).toString());}
            return user;
        }
        finally {
            sdbDatasourceWrapper.releaseSequoiadb(sdb);
        }
    }

    @Override
    public int getMaxID() throws S3ServerException{
        Sequoiadb sdb = null;
        try {
            sdb = sdbDatasourceWrapper.getSequoiadb();
            CollectionSpace cs = sdb.getCollectionSpace(config.getMetaCsName());
            DBCollection cl = cs.getCollection(DaoCollectionDefine.USER_LIST_COLLECTION);

            BSONObject selector = new BasicBSONObject();
            selector.put(User.JSON_KEY_USERID, "");
            BSONObject orderBy = new BasicBSONObject();
            orderBy.put(User.JSON_KEY_USERID,-1);
            BSONObject queryResult = cl.queryOne(null,selector,orderBy,null,0);

            if (null != queryResult) {
                return (int)(queryResult.get(User.JSON_KEY_USERID));
            }
            else {
                return 0;
            }

        }finally {
            sdbDatasourceWrapper.releaseSequoiadb(sdb);
        }

    }

    @Override
    public long getCountByRole(String role) throws S3ServerException{
        Sequoiadb sdb = null;
        try {
            sdb = sdbDatasourceWrapper.getSequoiadb();
            CollectionSpace cs = sdb.getCollectionSpace(config.getMetaCsName());
            DBCollection cl = cs.getCollection(DaoCollectionDefine.USER_LIST_COLLECTION);

            BSONObject matcher = new BasicBSONObject();
            matcher.put(User.JSON_KEY_ROLE, role);
            long count = cl.getCount(matcher);

            return count;
        }
        finally {
            sdbDatasourceWrapper.releaseSequoiadb(sdb);
        }
    }
}
