package com.sequoiadb.plugin.dao;

import com.sequoiadb.base.CollectionSpace;
import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.DBCursor;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.exception.SDBError;
import com.sequoiadb.plugin.Register;
import com.sequoiadb.plugin.config.OmsvcConfig;
import org.bson.BSONObject;
import org.bson.BasicBSONObject;
import org.bson.types.BasicBSONList;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Component;

@Component
public class DbOperations {

    private Sequoiadb db;
    private final Logger logger = LoggerFactory.getLogger(DbOperations.class);

    @Autowired
    private OmsvcConfig omConfig;

    @Autowired
    private Register register;

    public synchronized void getSsqlInfo(String clusterName, String businessName, StringBuilder hostName, StringBuilder svcname) {
        connect();

        CollectionSpace cs = db.getCollectionSpace("SYSDEPLOY");
        DBCollection cl = cs.getCollection("SYSCONFIGURE");

        BSONObject queryCondition = new BasicBSONObject();
        queryCondition.put("ClusterName", clusterName);
        queryCondition.put("BusinessName", businessName);

        DBCursor cur = cl.query(queryCondition, null, null, null);
        if (cur.hasNext()) {
            BSONObject record = cur.getNext();
            hostName.append((String) record.get("HostName"));
            BasicBSONList configs = (BasicBSONList) record.get("Config");
            if (configs.size() > 0) {
                BSONObject config = (BSONObject) configs.get(0);
                svcname.append((String) config.get("port"));
            }
        }
    }

    public synchronized void getSsqlAccountInfo(String clusterName, String businessName, StringBuilder user, StringBuilder passwd, StringBuilder defaultDb) {
        connect();

        CollectionSpace cs = db.getCollectionSpace("SYSDEPLOY");
        DBCollection cl = cs.getCollection("SYSBUSINESSAUTH");

        BSONObject queryCondition = new BasicBSONObject();
        queryCondition.put("BusinessName", businessName);

        DBCursor cur = cl.query(queryCondition, null, null, null);
        if (cur.hasNext()) {
            BSONObject record = cur.getNext();
            user.append((String) record.get("User"));
            passwd.append((String) record.get("Passwd"));
            defaultDb.append((String) record.get("DbName"));
        }

        if (user.length() == 0) {
            getClusterInfo(clusterName, user);
        }
    }

    private void getClusterInfo(String clusterName, StringBuilder user) {
        CollectionSpace cs = db.getCollectionSpace("SYSDEPLOY");
        DBCollection cl = cs.getCollection("SYSCLUSTER");

        BSONObject queryCondition = new BasicBSONObject();
        queryCondition.put("ClusterName", clusterName);

        DBCursor cur = cl.query(queryCondition, null, null, null);
        if (cur.hasNext()) {
            BSONObject record = cur.getNext();
            user.append((String) record.get("SdbUser"));
        }
    }

    private void connect() throws BaseException {
        if (db == null || db.isValid() == false) {

            if (db != null) {
                try {
                    db.disconnect();
                    db = null;
                } catch (BaseException e) {
                    logger.warn(e.getMessage());
                }
            }

            for (int i = 0; ; ++i) {
                try {
                    String hostName = omConfig.getHostName();
                    String svcname = omConfig.getSvcname();
                    String user = omConfig.getUser();
                    String pwd = omConfig.getPasswd();
                    db = new Sequoiadb(hostName + ":" + svcname, user, pwd);
                    break;
                } catch (BaseException e) {
                    if (e.getErrorCode() == SDBError.SDB_AUTH_AUTHORITY_FORBIDDEN.getErrorCode()) {
                        if (i < 5) {
                            register.register();
                            continue;
                        } else {
                            logger.warn(e.getMessage());
                            throw e;
                        }
                    } else {
                        logger.warn(e.getMessage());
                        throw e;
                    }
                }
            }
        }
    }

    protected void finalize() {
        if (db != null) {
            try {
                db.disconnect();
                db = null;
            } catch (BaseException e) {
                logger.warn(e.getMessage());
            }
        }
    }
}
