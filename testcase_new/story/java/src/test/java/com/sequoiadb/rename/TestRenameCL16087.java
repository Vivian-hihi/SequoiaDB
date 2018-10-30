package com.sequoiadb.rename;

import org.bson.BSONObject;
import org.bson.BasicBSONObject;
import org.bson.util.JSON;
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
 *  @FileName:TestRenameCL16087
 *  并发修改主表名和子表名  
 *  @author chensiqin
 *  @Date 2018-10-24
 *  @version 1.00
 */
public class TestRenameCL16087 extends SdbTestBase {
    /*
     * 1、并发修改子表名和attach子表，多线程执行如下操作：
     *  a、修改子表名，如修改subcl1为subcl2 
     *  b、主表上attach子表subcl1 
     *  2、检查操作结果 
     * */
    private String subCLName = "subCL16087";
    private String newSubCLName = "newSubCL16087";
    private String mainCLName = "mainCL16087";
    private Sequoiadb sdb = null;
    private CollectionSpace cs = null;
    
    @BeforeClass
    public void setUp() {
        sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "");
        if (CommLib.isStandAlone(sdb)) {
            throw new SkipException("skip StandAlone");
        }
        cs = sdb.getCollectionSpace(SdbTestBase.csName);
        createMainCL();
        cs.createCollection(subCLName,(BSONObject) JSON.parse("{ShardingKey:{\"a\":1},ShardingType:\"hash\"}"));
        
    }
    
    @Test
    public void test16087(){
        AttachCLThread attachCLThread = new AttachCLThread();
        RenameSubCLThread subCLThread = new RenameSubCLThread();
        attachCLThread.start();
        subCLThread.start();
        if (attachCLThread.isSuccess() && subCLThread.isSuccess()) {
            RenameUtil.checkRenameCLResult(sdb, SdbTestBase.csName, subCLName, newSubCLName);
            DBCollection maincl = cs.getCollection(mainCLName);
            Assert.assertEquals(cs.isCollectionExist(subCLName), false);
            //maincl.detachCollection(SdbTestBase.csName+"."+newSubCLName);
        } else if ( !attachCLThread.isSuccess() && subCLThread.isSuccess() ) {
            RenameUtil.checkRenameCLResult(sdb, SdbTestBase.csName, subCLName, newSubCLName);
            Assert.assertEquals(cs.isCollectionExist(subCLName), false);
            try {
                DBCollection maincl = cs.getCollection(mainCLName);
                maincl.detachCollection(SdbTestBase.csName+"."+newSubCLName);
                Assert.fail("cl attachCollection ok, expected attachCollection fail!");
            }catch (BaseException e) {
                e.printStackTrace();
            }
            BaseException e = (BaseException)attachCLThread.getExceptions().get(0);
            if ( e.getErrorCode() != -23 ) {
                Assert.fail("errcode not expected : " + e.getMessage());
            }
        } else if ( attachCLThread.isSuccess() && !subCLThread.isSuccess() ) {
            BaseException e = (BaseException)attachCLThread.getExceptions().get(0);
            if ( e.getErrorCode() != -147 ) {
                Assert.fail("errcode not expected : " + e.getMessage());
            }
        } else {
            Assert.fail("attachCLThread : "+attachCLThread.getErrorMsg()+"\n subCLThread : "+subCLThread.getErrorMsg());
        }
    }
    
    @AfterClass
    public void tearDown() {
        if(cs.isCollectionExist(subCLName)){
            cs.dropCollection(subCLName);
        }
        if(cs.isCollectionExist(newSubCLName)){
            cs.dropCollection(newSubCLName);
        }
        if(cs.isCollectionExist(mainCLName)){
            cs.dropCollection(mainCLName);
        }
        if(this.sdb != null){
            this.sdb.close();
        }
    }
    
    
    public void createMainCL() {
        BSONObject options = new BasicBSONObject();
        options.put("IsMainCL", true);
        BSONObject opt = new BasicBSONObject();
        opt.put("a", 1);
        options.put("ShardingKey", opt);
        options.put("ShardingType", "range");
        cs.createCollection(mainCLName, options);
    }

    private class RenameSubCLThread extends SdbThreadBase{

        @Override
        public void exec() throws BaseException {
            Sequoiadb db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
            try {
                CollectionSpace localcs = db.getCollectionSpace(SdbTestBase.csName);
                localcs.renameCollection(subCLName, newSubCLName);
            }finally {
                db.close();
            }
        }
    }
    
    private class AttachCLThread extends SdbThreadBase{

        @Override
        public void exec() throws Exception {
            Sequoiadb db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
            try {
                DBCollection maincl = cs.getCollection(mainCLName);
                BSONObject options = new BasicBSONObject();
                options.put("LowBound", (BSONObject) JSON.parse("{\"a\":1}"));
                options.put("UpBound", (BSONObject) JSON.parse("{\"a\":100}"));
                maincl.attachCollection(SdbTestBase.csName+"."+subCLName, options);
            }finally {
                db.close();
            }
        }
    }

}
