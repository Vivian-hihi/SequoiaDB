package com.sequoiadb.rename;

import org.testng.Assert;
import org.testng.SkipException;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import com.sequoiadb.base.CollectionSpace;
import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.testcommon.CommLib;
import com.sequoiadb.testcommon.SdbTestBase;
import com.sequoiadb.testcommon.SdbThreadBase;

/**
 *  @FileName:TestRenameCL16082
 *  并发修改cl，其中一个cl修改为另一个cl旧名 
 *  @author chensiqin
 *  @Date 2018-10-23
 *  @version 1.00
 */
public class TestRenameCL16082 extends SdbTestBase{
    
    private String clName1 = "cl16082_1";
    private String clName2 = "cl16082_2";
    private String newclName = "newcl16082";
    private Sequoiadb sdb = null;
    private CollectionSpace cs = null;
    
    @BeforeClass
    public void setUp() {
        sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "");
        if (CommLib.isStandAlone(sdb)) {
            throw new SkipException("skip StandAlone");
        }
        cs = sdb.getCollectionSpace(SdbTestBase.csName);
        DBCollection cl1 = cs.createCollection(clName1);
        DBCollection cl2 = cs.createCollection(clName2);
        RenameUtil.insertData(cl1, 5);
        RenameUtil.insertData(cl2, 20);
    }

    @Test
    public void test16082() {
        RenameCLThread1 rename1 = new RenameCLThread1();
        RenameCLThread2 rename2 = new RenameCLThread2();
        rename1.start();
        rename2.start();
        if(rename1.isSuccess() && rename2.isSuccess()) {
            sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "");
            checkCLRecord(newclName, 5);
            checkCLRecord(clName1, 20);
            RenameUtil.checkRenameCLResult(sdb, SdbTestBase.csName, clName1, newclName);
            RenameUtil.checkRenameCLResult(sdb, SdbTestBase.csName, clName2, clName1);
        } else if (rename1.isSuccess() && !rename2.isSuccess()) {
            sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "");
            RenameUtil.checkRenameCLResult(sdb, SdbTestBase.csName, clName1, newclName);
            checkCLRecord(newclName, 5);
            checkCLRecord(clName2, 20);
            BaseException e = (BaseException)rename2.getExceptions().get(0);
            if ( e.getErrorCode() != -22 && e.getErrorCode() != -147) {
                Assert.fail("errcode not expected : " + e.getMessage());
            }
        } else {
            Assert.fail("test16082 failed: "+rename1.getErrorMsg()+rename2.getErrorMsg());
        }
    }
    
    public void checkCLRecord(String name, int expected) {
        DBCollection cl = cs.getCollection(name);
        Assert.assertEquals(cl.getCount(), expected);
    }
    
    @AfterClass
    public void tearDown() {
        if(cs.isCollectionExist(clName1)){
            cs.dropCollection(clName1);
        }
        if(cs.isCollectionExist(clName2)){
            cs.dropCollection(clName2);
        }
        if(cs.isCollectionExist(newclName)){
            cs.dropCollection(newclName);
        }
        if(this.sdb != null){
            this.sdb.close();
        }
    }
    
    
    private class RenameCLThread1 extends SdbThreadBase{

        @Override
        public void exec() throws BaseException {
            Sequoiadb db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
            try {
                CollectionSpace localcs = db.getCollectionSpace(csName);
                localcs.renameCollection(clName1, newclName);
                
            }finally {
                db.close();
            }
        }
    }
    
    private class RenameCLThread2 extends SdbThreadBase{

        @Override
        public void exec() throws BaseException {
            Sequoiadb db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
            try {
                CollectionSpace localcs = db.getCollectionSpace(csName);
                localcs.renameCollection(clName2, clName1);
            }finally {
                db.close();
            }
        }
    }
}
