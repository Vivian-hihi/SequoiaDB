package com.sequoiadb.rename;

import org.testng.Assert;
import org.testng.SkipException;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import com.sequoiadb.base.CollectionSpace;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.testcommon.CommLib;
import com.sequoiadb.testcommon.SdbTestBase;
import com.sequoiadb.testcommon.SdbThreadBase;

/**
 *  @FileName:TestRenameCS16133
 *  @content 修改cs名和创建/删除cl并发 
 *  @author chensiqin
 *  @Date 2018-10-19
 *  @version 1.00
 */
public class TestRenameCS16133 extends SdbTestBase{
    
    private String csName = "cs16133";
    private String newCSName = "newcs16133";
    private String clName = "cl16133";
    private Sequoiadb sdb = null;
    private CollectionSpace cs = null;
    private boolean clExist = false;
    
    @BeforeClass
    public void setUp() {
        sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "");
        if (CommLib.isStandAlone(sdb)) {//TODO:1.不需要屏蔽独立模式
            throw new SkipException("skip StandAlone");
        }
        if(sdb.isCollectionSpaceExist(csName)){
            sdb.dropCollectionSpace(csName);
        }
        if(sdb.isCollectionSpaceExist(newCSName)){
            sdb.dropCollectionSpace(newCSName);
        }
        cs = sdb.createCollectionSpace(csName);
    }
    
    @Test
    public void test16133() {
        RenameCSThread renameCSThread = new RenameCSThread();
        CreateCLThread createClThread = new CreateCLThread();
        renameCSThread.start();
        createClThread.start();
        
        if (renameCSThread.isSuccess() && !createClThread.isSuccess()){
            sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "");//TODO:2.这里的sdb为啥要重新new一个？
            RenameUtil.checkRenameCSResult(sdb, csName, newCSName, 0);
            BaseException e = (BaseException)createClThread.getExceptions().get(0);
            Assert.assertEquals(-34, e.getErrorCode(),"clThread failed : "+e.getMessage());
        } else if (!renameCSThread.isSuccess() && createClThread.isSuccess()){
            if (clExist) {
                Assert.assertTrue(cs.isCollectionExist(clName));
            } else {
                Assert.assertFalse(cs.isCollectionExist(clName));
            }
            BaseException e = (BaseException)renameCSThread.getExceptions().get(0);
            Assert.assertEquals(-147, e.getErrorCode(),"renameCS failed : "+e.getMessage());
        } else if (!renameCSThread.isSuccess() && !createClThread.isSuccess()){
            Assert.fail("renameCSThread and createClThread all failed: "+renameCSThread.getErrorMsg()+createClThread.getErrorMsg());
        } else{
            cs = sdb.getCollectionSpace(newCSName);
            if (clExist) {
                Assert.assertTrue(cs.isCollectionExist(clName));
            } else {
                Assert.assertFalse(cs.isCollectionExist(clName));
            }
        }
        
    }
    
    @AfterClass
    public void tearDown() {
        try {
            if(sdb.isCollectionSpaceExist(csName)){
                sdb.dropCollectionSpace(csName);
            }
            if(sdb.isCollectionSpaceExist(newCSName)){
                sdb.dropCollectionSpace(newCSName);
            }
        } catch (BaseException e) {
            Assert.fail(e.getMessage());
        } finally {
            if(this.sdb != null){
                this.sdb.close();
            }
        }
    }
    
    private class RenameCSThread extends SdbThreadBase{

        @Override
        public void exec() throws BaseException {
            Sequoiadb db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
            try {
                db.renameCollectionSpace(csName, newCSName);
            }finally {
                db.close();
            }
        }
    }
    
    private class CreateCLThread extends SdbThreadBase{

        @Override
        public void exec() throws BaseException {
            Sequoiadb db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
            try{//TODO:3.这里的并发有问题，如果这里的步骤执行成功，那么clExist = false，clExist的初始值也是false，用这个判断直接结果不准确，另外请确认下用例测试点
                CollectionSpace localcs = db.getCollectionSpace(csName);
                for(int i = 0; i <= 100; i++) {
                    localcs.createCollection(clName);
                    clExist = true;
                    localcs.dropCollection(clName);
                    clExist = false;
                }
            }finally{
                db.close();
            }
        }
    }
}
