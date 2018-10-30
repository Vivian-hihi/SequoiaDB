package com.sequoiadb.auth;

import java.text.SimpleDateFormat;
import java.util.Date;

import org.bson.BSONObject;
import org.bson.BasicBSONObject;
import org.testng.Assert;
import org.testng.annotations.AfterClass;
import org.testng.annotations.AfterTest;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.BeforeTest;
import org.testng.annotations.Test;
import com.sequoiadb.base.CollectionSpace;
import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.DBCursor;
import com.sequoiadb.base.Node;
import com.sequoiadb.base.ReplicaGroup;
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
        try {
            this.coordAddr = SdbTestBase.coordUrl;
            System.out.println("the TestCase Name:" + this.getClass().getName() + 
                    ". the TestCase begin at:" + new SimpleDateFormat("YYYY-MM-dd HH:mm:ss.SSS").format(new Date()));
            sdb = new Sequoiadb(this.coordAddr, "", "");
            
        }catch (BaseException e) {
            Assert.fail("Sequoiadb driver TestSdbUser16820 setUp error, error description:" + e.getMessage());
        }
    }
    @Test
    public void test() {
        testSdbUser();
    }
    
    public void testSdbUser() {
        if (!Util.isCluster(this.sdb)) {
            return ;
        }
        try {
            sdb.createUser("admin", "admin");
            Sequoiadb sdb = new Sequoiadb(coordAddr, "admin", "");
        }catch (BaseException e) {
            Assert.assertEquals(e.getErrorCode(), -179);
        }
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
            System.out.println("the TestCase Name:" + this.getClass().getName() + 
                    ". the TestCase end at:" + new SimpleDateFormat("YYYY-MM-dd HH:mm:ss.SSS").format(new Date()));
        } catch (BaseException e) {
            Assert.fail(e.getMessage());
        }
    } 
}
 