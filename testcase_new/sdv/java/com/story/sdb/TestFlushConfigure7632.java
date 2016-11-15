package com.story.sdb;

import org.bson.BSONObject;
import org.bson.util.JSON;
import org.testng.Assert;
import org.testng.annotations.Test;

import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.testcommon.SdbTestBase;

public class TestFlushConfigure7632 extends SdbTestBase{
    
    @Test
    public void testFlushConfigure() {
        String coordAddr = SdbTestBase.coordUrl;
        try {
            Sequoiadb sdb = new Sequoiadb(coordAddr, "", "");
            sdb.flushConfigure((BSONObject)JSON.parse("{Global:true}"));
        } catch (BaseException e) {
            Assert.fail("Sequoiadb driver TestFlushConfigure7632 testFlushConfigure error, error description:" + e.getMessage());
        }
    }
}
