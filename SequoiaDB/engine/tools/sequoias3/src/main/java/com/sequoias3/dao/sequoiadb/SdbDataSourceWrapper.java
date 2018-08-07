package com.sequoias3.dao.sequoiadb;

import java.util.List;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Repository;

import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.base.SequoiadbDatasource;
import com.sequoiadb.datasource.ConnectStrategy;
import com.sequoiadb.datasource.DatasourceOptions;
import com.sequoiadb.net.ConfigOptions;
import com.sequoias3.config.SequoiadbConfig;
import com.sequoias3.exception.S3DaoGetConnException;
import com.sequoias3.exception.S3ServerException;

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

        dsOpt.setMaxCount(500);
        dsOpt.setDeltaIncCount(20);
        dsOpt.setMaxIdleCount(20);
        dsOpt.setKeepAliveTimeout(0);
        dsOpt.setCheckInterval(60 * 1000);
        dsOpt.setSyncCoordInterval(0);
        dsOpt.setValidateConnection(false);
        dsOpt.setConnectStrategy(ConnectStrategy.BALANCE);

        sdbDatasource = new SequoiadbDatasource(addrs, null, null, nwOpt, dsOpt);
    }

    public Sequoiadb getSequoiadb() throws S3ServerException {
        try {
            return sdbDatasource.getConnection();
        }
        catch (Exception e) {
            throw new S3DaoGetConnException("get connection from Sequoiadb failed", e);
        }
    }

    public void releaseSequoiadb(Sequoiadb sdb) {
        if (null == sdb) {
            return;
        }

        try {
            sdbDatasource.releaseConnection(sdb);
        }
        catch (Exception e) {
            logger.warn("release connection failed:sdb={}" + sdb, e);
            try {
                sdb.disconnect();
            }
            catch (Exception e1) {
                logger.warn("disconnect sequoiadb failed", e1);
            }
        }
    }
}
