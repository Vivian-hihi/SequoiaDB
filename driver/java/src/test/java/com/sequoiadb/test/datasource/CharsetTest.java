package com.sequoiadb.test.datasource;

import com.sequoiadb.base.*;
import com.sequoiadb.datasource.ConnectStrategy;
import com.sequoiadb.datasource.DatasourceOptions;
import com.sequoiadb.datasource.SequoiadbDatasource;
import com.sequoiadb.test.common.Constants;
import org.bson.BSONObject;
import org.junit.Assert;
import org.junit.Test;

public class CharsetTest {

    private final static String ADDRESS = Constants.COOR_NODE_CONN;
    private final static String USER_NAME = "";
    private final static String PASSWORD = "";

    @Test
    public void test() throws Exception {
        // case 1: not set charset
        testDS(null);

        // case 2: UTF-8
        testDS(ClientCharsetEnum.UTF_8);

        // case 3: GB18030
        testDS(ClientCharsetEnum.GB18030);
    }

    private void testDS(ClientCharsetEnum charsetEnum) throws Exception {
        String sdbCharsetName;
        ConfigOptions netConf = new ConfigOptions();

        DatasourceOptions options = new DatasourceOptions();
        options.setMaxCount(10);
        options.setConnectStrategy(ConnectStrategy.SERIAL);

        if (charsetEnum == null || charsetEnum == ClientCharsetEnum.UTF_8) {
            sdbCharsetName = "UTF8";
        } else {
            sdbCharsetName = charsetEnum.getName();
        }

        if (charsetEnum != null) {
            ClientCharset charset = new ClientCharset();
            charset.setCharsets(charsetEnum);
            netConf.setCharset(charset);
        }

        SequoiadbDatasource ds = null;
        try {
            ds = SequoiadbDatasource.builder()
                    .serverAddress(ADDRESS)
                    .userConfig(new UserConfig(USER_NAME, PASSWORD))
                    .datasourceOptions(options)
                    .configOptions(netConf)
                    .build();

            // create connection directly
            Sequoiadb db = ds.getConnection();
            BSONObject obj = db.getSessionAttr(false);
            ds.releaseConnection(db);

            Assert.assertEquals(obj.toString(), obj.get("ClientCharset"), sdbCharsetName);
            Assert.assertEquals(obj.toString(), obj.get("ResultsCharset"), sdbCharsetName);

            // wait ds create idle connection
            Thread.sleep(200);

            int idleNum = (options.getMinIdleCount() + options.getMaxIdleCount()) / 2;
            Assert.assertEquals(ds.getIdleConnNum(), idleNum);

            // get connection from idle pool
            db = ds.getConnection();
            obj = db.getSessionAttr(false);
            ds.releaseConnection(db);

            Assert.assertEquals(obj.toString(), obj.get("ClientCharset"), sdbCharsetName);
            Assert.assertEquals(obj.toString(), obj.get("ResultsCharset"), sdbCharsetName);
        } finally {
            if (ds != null) {
                ds.close();
            }
        }
    }
}
