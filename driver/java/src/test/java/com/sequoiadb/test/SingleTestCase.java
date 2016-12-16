package com.sequoiadb.test;

import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.net.ConfigOptions;
import org.junit.AfterClass;
import org.junit.BeforeClass;

/*
 * Super class of single test class
 * */
public abstract class SingleTestCase extends TestCase {
    protected static Sequoiadb sdb;

    @BeforeClass
    public static void setUpTestCase() {
        TestCase.setUpTestCase();
        ConfigOptions options = new ConfigOptions();
        options.setConnectTimeout(3000);
        sdb = new Sequoiadb(
                TestConfig.getSingleHost(),
                Integer.valueOf(TestConfig.getSinglePort()),
                TestConfig.getSingleUsername(),
                TestConfig.getSinglePassword(),
                options);
    }

    @AfterClass
    public static void tearDownTestCase() {
        sdb.disconnect();
        sdb = null;
        TestCase.tearDownTestCase();
    }
}
