package com.sequoiadb.net;

import java.text.SimpleDateFormat;
import java.util.Date;

import org.testng.annotations.AfterTest;
import org.testng.annotations.Test;

import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.net.ConfigOptions;
import com.sequoiadb.testcommon.SdbTestBase;

public class TestMaxAutoConnectRetryTime extends SdbTestBase{
    private Sequoiadb sdb;
    private String csName;
    @Test
    public void testConfigOptionSSL() throws InterruptedException {
        String coordAddr = SdbTestBase.coordUrl;
        this.csName = SdbTestBase.csName;
        try {
            ConfigOptions options = new ConfigOptions();
            options.setMaxAutoConnectRetryTime(105000);
            //cutnet or normal
            System.out.println("conn sdb..." + new SimpleDateFormat("YYYY-MM-dd HH:mm:ss.SSS").format(new Date())); 
            sdb = new Sequoiadb(coordAddr, "", "", options);
            if (this.sdb.isCollectionSpaceExist(csName)) {
                this.sdb.dropCollectionSpace(csName); 
            }
            sdb.createCollectionSpace(csName);
            System.out.println("op finish..." + new SimpleDateFormat("YYYY-MM-dd HH:mm:ss.SSS").format(new Date())); 
        } catch (BaseException e) {
            System.out.println("BaseException finish..." + new SimpleDateFormat("YYYY-MM-dd HH:mm:ss.SSS").format(new Date())); 
            System.out.println(e.getErrorCode());
            e.printStackTrace();
        }
    }
    

    
//    @AfterTest
//    public void tearDown(){
//        if (this.sdb.isCollectionSpaceExist(csName)) {
//            this.sdb.dropCollectionSpace(csName); 
//        }
//        sdb.disconnect();
//    }
}
