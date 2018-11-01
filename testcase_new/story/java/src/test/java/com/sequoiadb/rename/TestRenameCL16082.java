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
 *  并发修改cl，其中一个cl修改为另一个cl旧名 //TODO:1、建议加一个字段吧，如@content 这样看起来格式统一
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
        RenameUtil.insertData(cl1, 5);//TODO:2、建议不要使用常量数字，可以定义变量名或者加一些描述
        RenameUtil.insertData(cl2, 20);
    }

    @Test
    public void test16082() {
        RenameCLThread1 rename1 = new RenameCLThread1();
        RenameCLThread2 rename2 = new RenameCLThread2();
        rename1.start();
        rename2.start();
        if(rename1.isSuccess() && rename2.isSuccess()) {
            sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "");//TODO:3、setUp中已new sdb，可以直接使用，建议不用再另外new sdb
            checkCLRecord(newclName, 5);//TODO:4、同2，建议定义有意义的变量名或者描述
            checkCLRecord(clName1, 20);
            RenameUtil.checkRenameCLResult(sdb, SdbTestBase.csName, clName1, newclName);
            RenameUtil.checkRenameCLResult(sdb, SdbTestBase.csName, clName2, clName1);//TODO:5、这个不会失败吗？实现里面看clName1不应该存在
        } else if (rename1.isSuccess() && !rename2.isSuccess()) {
            sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "");//TODO:6、同3
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
    
    //TODO:7、建议把方法都放在@afterClass后面，代码前面时beforClass、test、afterClass结构
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
        if(this.sdb != null){//TODO:8、sdb关闭要放在finally，如果上面步骤失败，无法执行close（）
            this.sdb.close();
        }
    }
    
    //TODO:9、RenameCLThread1和RenameCLThread2代码一致，建议传参，用一份代码
    private class RenameCLThread1 extends SdbThreadBase{

        @Override
        public void exec() throws BaseException {
            Sequoiadb db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
            try {//TODO:10、建议使用自动释放资源方法，new db放在try条件中，不需要finally
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
