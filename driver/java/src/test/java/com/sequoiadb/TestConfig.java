package com.sequoiadb;

import static org.junit.Assert.assertEquals;

import org.junit.Ignore;
import org.junit.Test;

public class TestConfig {
    @Ignore
    @Test
    public void testConfig() {
        assertEquals("localhost", Config.getSingleHost());
        assertEquals("11810", Config.getSinglePort());
        assertEquals("", Config.getSingleUsername());
        assertEquals("", Config.getSinglePassword());
    }
}
