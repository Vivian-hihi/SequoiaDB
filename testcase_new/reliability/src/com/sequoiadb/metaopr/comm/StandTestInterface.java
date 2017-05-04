package com.sequoiadb.metaopr.comm;

import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;

/**
 * @FileName
 * @Author laojingtang
 * @Date 17-4-20
 * @Version 1.00
 */
public interface StandTestInterface {
    @BeforeClass
    void setup();

    @AfterClass
    void tearDown();
}
