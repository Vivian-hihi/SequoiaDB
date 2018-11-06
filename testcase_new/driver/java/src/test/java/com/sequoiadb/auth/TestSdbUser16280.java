package com.sequoiadb.auth;

import org.testng.Assert;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.testcommon.SdbTestBase;

/**
 * @FileName:TestSdbUser16280  Connecting sequoiadb with incorrect password
 * @author wangkexin
 * @Date 2018-10-22
 * @version 1.00
 */

public class TestSdbUser16280 extends SdbTestBase{
    private Sequoiadb sdb;
    private String coordAddr;
    
    @BeforeClass
    public void setUp() {
        this.coordAddr = SdbTestBase.coordUrl;
        sdb = new Sequoiadb(this.coordAddr, "", "");
    }
    
    @Test
    public void test() {
        testSdbUser();
    }
    
    @AfterClass
    public void tearDown() {
        try {
            try {
                sdb.removeUser("admin", "admin");
            }catch (BaseException e) {
                if (-300 !=e.getErrorCode()) {
                    Assert.assertTrue(false, "drop user, errMsg: " + e.getMessage());
                }
            }
            sdb.close();
        } catch (BaseException e) {
            Assert.fail(e.getMessage());
        }
    } 
    
    public void testSdbUser() {
        try {
            sdb.createUser("admin", "admin");
            Sequoiadb sdb = new Sequoiadb(coordAddr, "admin", "");
        }catch (BaseException e) {
            Assert.assertEquals(e.getErrorCode(), -179);
        }
    }
}
 