package com.sequoiadb.test.db;

import com.sequoiadb.base.ClientCharset;
import com.sequoiadb.base.ClientCharsetEnum;
import com.sequoiadb.base.ConfigOptions;
import org.junit.Assert;
import org.junit.Test;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotEquals;

public class ConfigOptionsTest {

    @Test
    public void test() {
        ConfigOptions netConf = new ConfigOptions();
        ClientCharset charset = new ClientCharset();
        charset.setResultsCharset(ClientCharsetEnum.GB18030);

        // case 1: default charset is null
        Assert.assertNull(netConf.getCharset());

        // case 2: set charset
        netConf.setCharset(charset);
        Assert.assertEquals(charset, netConf.getCharset());
    }

    @Test
    public void hashCodeAndEqualsTest() {
        ConfigOptions netConf1 = new ConfigOptions();
        ConfigOptions netConf2 = new ConfigOptions();
        ConfigOptions netConf3 = new ConfigOptions();
        netConf3.setCharset(new ClientCharset());

        assertEquals(netConf1.hashCode(), netConf2.hashCode());
        assertEquals(netConf1, netConf2);

        assertNotEquals(netConf1.hashCode(), netConf3.hashCode());
        assertNotEquals(netConf1, netConf3);
    }
}
