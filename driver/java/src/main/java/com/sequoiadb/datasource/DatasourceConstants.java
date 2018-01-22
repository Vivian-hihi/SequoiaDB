package com.sequoiadb.datasource;

/**
 * Created by tanzhaobo on 2018/1/20.
 */
final class DatasourceConstants {
    private DatasourceConstants() {
    }

    final static String FIELD_NAME_PREFERED_INSTANCE = "PreferedInstance";
    final static String FIELD_NAME_PREFERED_INSTANCE_MODE = "PreferedInstanceMode";
    final static String FIELD_NAME_SESSION_TIMEOUT = "Timeout";

    final static String PREFERED_INSTANCE_MODE_RANDON = "random";
    final static String PREFERED_INSTANCE_MODE_ORDERED = "ordered";
}
