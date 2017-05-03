package com.sequoiadb.metaopr.comm;

import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.exception.ReliabilityException;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

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
