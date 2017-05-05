package com.sequoiadb.metaopr.comm;

import com.sequoiadb.commlib.SdbTestBase;
import com.sequoiadb.datasource.DatasourceOptions;
import com.sequoiadb.datasource.SequoiadbDatasource;

/**
 * @FileName
 * @Author laojingtang
 * @Date 17-5-4
 * @Version 1.00
 */
public class MyDataSource {
    private static SequoiadbDatasource _ds;

    private MyDataSource() {
    }

    public static SequoiadbDatasource getDataSource() {
        if (_ds != null)
            return _ds;
        DatasourceOptions dsOpt = new DatasourceOptions();
        dsOpt.setMaxCount(500);
        dsOpt.setDeltaIncCount(20);
        dsOpt.setMaxIdleCount(20);
        dsOpt.setCheckInterval(60 * 1000);
        _ds = new SequoiadbDatasource(SdbTestBase.coordUrl, "", "", dsOpt);
        return _ds;
    }
}
