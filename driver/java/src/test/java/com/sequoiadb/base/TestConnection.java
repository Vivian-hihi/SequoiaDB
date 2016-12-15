package com.sequoiadb.base;

import com.sequoiadb.Config;
import org.junit.Test;

public class TestConnection {
    @Test
    public void TestConnect() {
        Sequoiadb sdb = new Sequoiadb(
                Config.getSingleHost(),
                Integer.valueOf(Config.getSinglePort()),
                Config.getSingleUsername(),
                Config.getSinglePassword());
        sdb.disconnect();
    }
}
