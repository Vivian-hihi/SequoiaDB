package com.sequoiadb.net;

import org.testng.annotations.AfterTest;
import org.testng.annotations.Test;

import com.sequoiadb.base.CollectionSpace;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.net.ConfigOptions;
import com.sequoiadb.testcommon.SdbTestBase;

public class TestSSL9644 extends SdbTestBase{
    private Sequoiadb sdb;
    private String csName;
    private String clName = "cl9644";
    
    @Test
    public void testConfigOptionSSL() {
        String coordAddr = SdbTestBase.coordUrl;
        this.csName = SdbTestBase.csName;
        try {
            ConfigOptions options = new ConfigOptions();
            options.setUseSSL(true);
            sdb = new Sequoiadb(coordAddr, "", "", options);
            if (this.sdb.isCollectionSpaceExist(csName)) {
                this.sdb.dropCollectionSpace(csName); 
            }
            CollectionSpace cs = sdb.createCollectionSpace(csName);
            cs.createCollection(clName);
            System.out.println("op finish..."); 
        } catch (BaseException e) {
            System.out.println(e.getErrorCode());
            e.printStackTrace();
        }
    }
    
    @AfterTest
    public void tearDown(){
        if (this.sdb.isCollectionSpaceExist(csName)) {
            this.sdb.dropCollectionSpace(csName); 
        }
        sdb.disconnect();
    }
}
