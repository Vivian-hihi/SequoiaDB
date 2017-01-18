package com.sequoiadb.net;

import java.text.SimpleDateFormat;
import java.util.Date;

import org.testng.Assert;
import org.testng.annotations.AfterTest;
import org.testng.annotations.Test;

import com.sequoiadb.base.ReplicaGroup;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.net.ConfigOptions;
import com.sequoiadb.testcommon.SdbTestBase;

public class TestSocketKeepAlive extends SdbTestBase{
    private Sequoiadb sdb;
    private String csName;
    
    @Test(enabled=true)
    public void testConfigOptionSSL() throws InterruptedException { 
        String coordAddr = SdbTestBase.coordUrl;
        this.csName = SdbTestBase.csName;
        try {
            ConfigOptions options = new ConfigOptions();
            options.setSocketKeepAlive(true);
            System.out.println("conn sdb..." + new SimpleDateFormat("YYYY-MM-dd HH:mm:ss.SSS").format(new Date())); 
            sdb = new Sequoiadb(coordAddr, "", "");
            sdb.changeConnectionOptions(options);
            ReplicaGroup rg = sdb.getReplicaGroup("datagroup3");
            System.out.println("1111===" + sdb.isClosed());
            rg.stop();
            System.out.println("2222===" + sdb.isClosed());
            rg.start();
            System.out.println("3333===" + sdb.isClosed());
            System.out.println("op finish..." + new SimpleDateFormat("YYYY-MM-dd HH:mm:ss.SSS").format(new Date())); 
            
        } catch (BaseException e) {
            System.out.println("BaseException finish..." + new SimpleDateFormat("YYYY-MM-dd HH:mm:ss.SSS").format(new Date())); 
            e.printStackTrace();
            Assert.fail();
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
