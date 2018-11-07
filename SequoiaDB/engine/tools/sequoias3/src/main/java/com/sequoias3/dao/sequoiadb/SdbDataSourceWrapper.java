package com.sequoias3.dao.sequoiadb;

import com.sequoiadb.base.*;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.datasource.ConnectStrategy;
import com.sequoiadb.datasource.DatasourceOptions;
import com.sequoiadb.datasource.SequoiadbDatasource;
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.exception.SDBError;
import com.sequoiadb.base.ConfigOptions;
import com.sequoias3.config.SequoiadbConfig;
import com.sequoias3.exception.S3DaoGetConnException;
import com.sequoias3.exception.S3Error;
import com.sequoias3.exception.S3ServerException;
import org.bson.BSONObject;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Repository;

import java.util.List;

@Repository
public class SdbDataSourceWrapper {
    private static final Logger logger = LoggerFactory.getLogger(SdbDataSourceWrapper.class);

    private SequoiadbDatasource sdbDatasource;

    @Autowired
    public SdbDataSourceWrapper(SequoiadbConfig config) {
        List<String> addrs = config.getUrlList();
        ConfigOptions nwOpt = new ConfigOptions();
        DatasourceOptions dsOpt = new DatasourceOptions();

        nwOpt.setConnectTimeout(500);
        nwOpt.setMaxAutoConnectRetryTime(0);

        dsOpt.setMaxCount(config.getMaxConnectionNum());
        dsOpt.setDeltaIncCount(config.getDeltaIncCount());
        dsOpt.setMaxIdleCount(config.getMaxIdleNum());
        dsOpt.setKeepAliveTimeout(config.getKeepAliveTime());
        dsOpt.setCheckInterval(config.getCheckInterval());
        dsOpt.setSyncCoordInterval(0);
        dsOpt.setValidateConnection(config.getValidateConnection());
        dsOpt.setConnectStrategy(ConnectStrategy.BALANCE);

        sdbDatasource = new SequoiadbDatasource(addrs, null, null, nwOpt, dsOpt);
    }

    public Sequoiadb getSequoiadb() throws S3ServerException {
        try {
            return sdbDatasource.getConnection();
        } catch (Exception e) {
            throw new S3DaoGetConnException("get connection from Sequoiadb failed", e);
        }
    }

    public void releaseSequoiadb(Sequoiadb sdb) {
        if (null == sdb) {
            return;
        }

        try {
            sdbDatasource.releaseConnection(sdb);
        } catch (Exception e) {
            logger.warn("release connection failed:sdb={}" + sdb, e);
            try {
                sdb.close();
            } catch (Exception e1) {
                logger.warn("disconnect sequoiadb failed", e1);
            }
        }
    }

    public void releaseDBCursor(DBCursor cursor) {
        if (null == cursor) {
            return;
        }

        try {
            cursor.close();
        } catch (Exception e) {
            logger.warn("release connection failed:sdb={}" + cursor, e);
        }
    }

    public static void createCS(Sequoiadb sdb, String csName, BSONObject options)
            throws S3ServerException  {
        try {
            logger.info("creating cs:csName={}", csName);
            sdb.createCollectionSpace(csName, options);
        }
        catch (BaseException e) {
            if (e.getErrorCode() != SDBError.SDB_DMS_CS_EXIST.getErrorCode()) {
                throw new S3ServerException(S3Error.DAO_DB_ERROR,
                        "createcl failed:cs=" + csName +
                                ",options=" + options.toString()+ e.getMessage());
            }
        }
        catch (Exception e) {
            throw new S3ServerException(S3Error.DAO_DB_ERROR,
                    "createcl failed:cs=" + csName +
                            ",options=" + options.toString()+ e.getMessage());
        }
    }

    public static void createCL(Sequoiadb sdb, String csName, String clName, BSONObject options)
            throws S3ServerException {
        try {
            logger.info("creating cl:clName={}.{}",csName, clName);
            CollectionSpace cs = sdb.getCollectionSpace(csName);
            cs.createCollection(clName, options);
        }
        catch (BaseException e) {
            if (e.getErrorCode() != SDBError.SDB_DMS_EXIST.getErrorCode()) {
                throw new S3ServerException(S3Error.DAO_DB_ERROR,
                        "createcl failed:cs=" + csName + ",cl=" + clName +
                                ",options=" + options.toString()+ e.getMessage());
            }
        }
        catch (Exception e) {
            throw new S3ServerException(S3Error.DAO_DB_ERROR,
                    "createcl failed:cs=" + csName + ",cl=" + clName +
                            ",options=" + options.toString()+ e.getMessage());
        }
    }

    public static void createIndex(Sequoiadb sdb, String csName, String clName,
                                   String indexName, BSONObject key, boolean isUnique,
                                   boolean enforced)throws S3ServerException {
        try {
            logger.info("creating cl index :clName= {}.{}, indexname={}", csName, clName, indexName);
            CollectionSpace cs = sdb.getCollectionSpace(csName);
            DBCollection cl = cs.getCollection(clName);
            cl.createIndex(indexName, key, isUnique, enforced);
        }
        catch (BaseException e) {
            if (e.getErrorCode() != SDBError.SDB_IXM_DUP_KEY.getErrorCode()) {
                throw new S3ServerException(S3Error.DAO_DB_ERROR,
                        "createIndex failed:cs=" + csName + ",cl=" + clName +
                                ",indexName=" + indexName + e.getMessage());
            }
        }
        catch (Exception e) {
            throw new S3ServerException(S3Error.DAO_DB_ERROR,
                    "createcl failed:cs=" + csName + ",cl=" + clName +
                            ",options=" + indexName + e.getMessage());
        }
    }
}
