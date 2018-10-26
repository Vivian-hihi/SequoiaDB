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
import com.sequoiadb.testcommon.SdbTestBase;
import com.sequoiadb.testcommon.SdbThreadBase;
import com.sequoiadb.testcommon.CommLib;

/**
 *  @FileName:TestRenameCL16088
 *  并发修改主表名和子表名  
 *  @author chensiqin
 *  @Date 2018-10-23
 *  @version 1.00
 */
public class TestRenameCL16088 extends SdbTestBase {
    
    private String subCLName = "subCL16088";
    private String newSubCLName = "newSubCL16088";
    private String mainCLName = "mainCL16088";
    private String newMainCLName = "newMainCL16088";
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
        DBCollection maincl = cs.getCollection(mainCLName);
        BSONObject options = new BasicBSONObject();
        options.put("LowBound", (BSONObject) JSON.parse("{\"a\":1}"));
        options.put("UpBound", (BSONObject) JSON.parse("{\"a\":100}"));
        maincl.attachCollection(SdbTestBase.csName+"."+subCLName, options);
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
    
    @Test
    public void test16088(){
        RenameMainCLThread mainCLThread = new RenameMainCLThread();
        RenameSubCLThread subCLThread = new RenameSubCLThread();
        mainCLThread.start();
        subCLThread.start();
        if (subCLThread.isSuccess() && !mainCLThread.isSuccess()) {
            sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "");
            RenameUtil.checkRenameCLResult(sdb, SdbTestBase.csName, subCLName, newSubCLName);
            Assert.assertEquals(cs.isCollectionExist(mainCLName), true);
            BaseException e = (BaseException)mainCLThread.getExceptions().get(0);
            if ( e.getErrorCode() != -147 ) {
                Assert.fail("errcode not expected : " + e.getMessage());
            }
        } else if ( !subCLThread.isSuccess() && mainCLThread.isSuccess() ) {
            sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "");
            RenameUtil.checkRenameCLResult(sdb, SdbTestBase.csName, mainCLName, newMainCLName);
            Assert.assertEquals(cs.isCollectionExist(subCLName), true);
            BaseException e = (BaseException)subCLThread.getExceptions().get(0);
            if ( e.getErrorCode() != -147 ) {
                Assert.fail("errcode not expected : " + e.getMessage());
            }
        } else if (subCLThread.isSuccess() && mainCLThread.isSuccess()) {
            Assert.assertEquals(cs.isCollectionExist(subCLName), false);
            Assert.assertEquals(cs.isCollectionExist(mainCLName), false);
        } else {
            Assert.fail("subCLThread : "+subCLThread.getErrorMsg()+"\nmainCLThread :"+mainCLThread.getErrorMsg());
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
        if(cs.isCollectionExist(newMainCLName)){
            cs.dropCollection(newMainCLName);
        }
        if(this.sdb != null){
            this.sdb.close();
        }
    }
    
    private class RenameMainCLThread extends SdbThreadBase{

        @Override
        public void exec() throws BaseException {
            Sequoiadb db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
            try {
                CollectionSpace localcs = db.getCollectionSpace(SdbTestBase.csName);
                localcs.renameCollection(mainCLName, newMainCLName);
                
            }finally {
                db.close();
            }
        }
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
}
