package com.sequoiadb.transaction;

import java.util.Arrays;
import java.util.Random;

import org.testng.Assert;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import com.sequoiadb.base.CollectionSpace;
import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.rename.RenameUtil;
import com.sequoiadb.testcommon.CommLib;
import com.sequoiadb.testcommon.SdbConfTestBase;
import com.sequoiadb.testcommon.SdbTestBase;
import com.sequoiadb.testcommon.SdbThreadBase;

/**
 * @Description RenameCL_16104.java  并发事务操作和修改CS名 
 * @author luweikang
 * @date 2018年10月17日
 */
public class RenameCS_16104 extends SdbConfTestBase{
    
    private String csName = "renameCS_16104";
    private String newCSName= "renameCS_16104_new";
    private String clName = "rename_CL_16104";
    private Sequoiadb sdb = null;
    private CollectionSpace cs = null;
    private int recordNum = 1000;
    
    @Override
    protected void setNodeConf(){
        dataConf.put("transactionon", true);
        stdalnConf.put("transactionon", true);
    }
    
    @BeforeClass
    public void setUp(){
        sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "");
        cs = sdb.createCollectionSpace(csName);
        cs.createCollection(clName);
    }
    
    @Test
    public void test(){ 
        RenameCSThread renameCSThread = new RenameCSThread();
        TransactionThread transThread = new TransactionThread();
        
        renameCSThread.start();
        transThread.start();
        
        boolean rename = renameCSThread.isSuccess();
        boolean trans = transThread.isSuccess();
        
        if(!rename){
            Integer[] errnosA = { -190 };
            BaseException errorA = (BaseException)renameCSThread.getExceptions().get(0);
            if( !Arrays.asList(errnosA).contains(errorA.getErrorCode()) ){
                Assert.fail(renameCSThread.getErrorMsg());
            }
        }
        
        if(!trans){
            Integer[] errnosB = { -23, -34, -190 };
            BaseException errorB = (BaseException)transThread.getExceptions().get(0);
            if( !Arrays.asList(errnosB).contains(errorB.getErrorCode()) ){
                Assert.fail(transThread.getErrorMsg());
            }
        }
        
        Sequoiadb db = null; 
        try{
            db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
            if(rename && !trans){
                RenameUtil.checkRenameCSResult(db, csName, newCSName, 1);
                checkRecordNum(db, newCSName, clName, 0);
            }else if(!rename && trans){
                RenameUtil.checkRenameCSResult(db, newCSName, csName, 1);
                checkRecordNum(db, csName, clName, recordNum);
            }else if(rename && trans){
                RenameUtil.checkRenameCSResult(db, csName, newCSName, 1);
                checkRecordNum(db, newCSName, clName, recordNum);
            }else{
                Assert.fail("rename cl and split cl all failed");
            }
        } finally{
            db.close();//TODO：这个db应该判断是否为null再关闭？？
        }
    }
    
    @AfterClass
    public void tearDown(){
        CommLib.clearCS(sdb, csName);
        if(sdb!=null){//TODO：建议放在finally
            sdb.close();
        }
    }
    
    private class RenameCSThread extends SdbThreadBase{

        @Override
        public void exec() throws Exception {//TODO： 为啥时抛出Exception，建议抛出BaseException
            Sequoiadb db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
            try {
                Thread.sleep(new Random().nextInt(50) + 50);
                db.renameCollectionSpace(csName, newCSName);
            }finally {
                db.close();
            }
        }
    }
    
    private class TransactionThread extends SdbThreadBase{
        @Override
        public void exec() throws Exception {//TODO： 为啥时抛出Exception，建议抛出BaseException
            Sequoiadb db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
            try {
                DBCollection cl = db.getCollectionSpace(csName).getCollection(clName);
                db.beginTransaction();
                RenameUtil.insertData(cl, recordNum);
                db.commit();
            }finally {
                db.close();
            }
        }
    }
    
    private void checkRecordNum(Sequoiadb db, String csName, String clName, int recordNum){
        DBCollection cl = db.getCollectionSpace(csName).getCollection(clName);
        long actNum = cl.getCount();
        Assert.assertEquals(actNum, recordNum, "check record count");
    }
    
}
