package com.sequoiadb.subcl.brokennetwork;

import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.Random;

import org.testng.Assert;
import org.testng.SkipException;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import com.sequoiadb.base.CollectionSpace;
import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.commlib.GroupMgr;
import com.sequoiadb.commlib.GroupWrapper;
import com.sequoiadb.commlib.SdbTestBase;
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.exception.ReliabilityException;
import com.sequoiadb.fault.BrokenNetwork;
import com.sequoiadb.subcl.commlib.Utils;

/**
 * @FileName seqDB-2179: detachCL过程中dataRG主节点连续降备
 * @Author linsuqiang
 * @Date 2017-03-14
 * @Version 1.00
 */

/*
 * 1、创建主表和子表 
 * 2、批量执行db.collectionspace.collection.detachCL()分离多个子表 
 * 3、子表分离过程中将cl所在dataRG主节点网络断掉（如：使用cutnet.sh工具，命令格式为nohup ./cutnet.sh &），检查detachCL执行结果 
 * 4、dataRG重新选主后立即将新主的网络断掉 
 * 5、重复步骤4两道三遍 
 * 6、将dataRG主节点网络恢复，并对分离成功的子表（普通表）做基本操作（如insert)
 */

public class DetachCL2179 extends SdbTestBase {
    private GroupMgr groupMgr = null;
    private boolean runSuccess = false;
    private String mclName = "cl_2179";
    private String clGroup = null;

    @BeforeClass
    public void setUp() {
        Sequoiadb sdb = null;
        try {
            System.out.println("the TestCase Name:" + this.getClass().getName() + ". the TestCase begin at:"
                    + new SimpleDateFormat("YYYY-MM-dd HH:mm:ss.SSS").format(new Date()));
            sdb = new Sequoiadb(coordUrl, "", "");
            groupMgr = GroupMgr.getInstance();

            if (!groupMgr.checkBusiness()) {
                throw new SkipException("checkBusiness failed");
            }

            // 创建主表和子表
            clGroup = groupMgr.getAllDataGroupName().get(0);
            Utils.createMclAndScl(sdb, mclName, clGroup);
            // 挂载所有子表
            Utils.attachAllScl(sdb, mclName);
        } catch (ReliabilityException e) {
            Assert.fail(this.getClass().getName() + " setUp error, error description:" + e.getMessage() + "\r\n"
                    + Utils.getKeyStack(e, this));
        } finally {
            if (sdb != null) {
                sdb.disconnect();
            }
        }
    }

    @Test
    public void test() {
        Sequoiadb db = null;
        try {
            // 建立并行任务
            DetachCLThread dThread = new DetachCLThread();
            ContinuousCutNetThread cThread = new ContinuousCutNetThread();
            dThread.start();
            cThread.start();
            dThread.join();
            cThread.join();

            // 检验任务结果
            if (!dThread.isSuccess()) {
                Assert.fail("attachCL failed: " + dThread.getErrMsg());
            }
            if (!cThread.isSuccess()) {
                Assert.fail("brokenNetwork failed: " + cThread.getErrMsg());
            }

            // 等待集群恢复
            while (groupMgr.checkBusinessWithLSN(false) != true) {}
            
            // 检查用例结果
            GroupWrapper cataGroup = groupMgr.getGroupByName("SYSCatalogGroup");
            Utils.checkConsistency(cataGroup);
            db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
            Utils.checkIntegrated(db, mclName);
            Utils.checkDetached(db, mclName, dThread.getDetachedSclCnt());
            runSuccess = true;
        } catch (InterruptedException e) {
        } catch (ReliabilityException e) {
            e.printStackTrace();
            Assert.fail(e.getMessage());
        } finally {
            if (db != null) {
                db.disconnect();
            }
        }
    }

    @AfterClass
    public void tearDown() {
        if (!runSuccess) { throw new SkipException("to save environment"); }
        Sequoiadb sdb = null;
        try {
            sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "");
            Utils.dropMclAndScl(sdb, mclName);
        } catch (BaseException e) {
            Assert.fail(e.getMessage() + "\r\n" + Utils.getKeyStack(e, this));
        } finally {
            if (sdb != null) {
                sdb.disconnect();
            }
            System.out.println("the TestCase Name:" + this.getClass().getName() + ". the TestCase end at:"
                    + new SimpleDateFormat("YYYY-MM-dd HH:mm:ss.SSS").format(new Date()));
        }
    }

    class DetachCLThread extends Thread {
        private int detachedSclCnt = 0;
        private Exception err = null;

        @Override
        public void run() {
            System.out.println("detachCL start");
            Sequoiadb db = null;
            try {
                db = new Sequoiadb(coordUrl, "", "");
                CollectionSpace cs = db.getCollectionSpace(csName);
                DBCollection mcl = cs.getCollection(mclName);
                for (int i = 0; i < Utils.SCLNUM; i++) {
                    String sclFullName = csName + "." + mclName + "_" + i;
                    mcl.detachCollection(sclFullName);
                    detachedSclCnt++;
                }
            } catch (BaseException e) {
                // -104 SDB_CLS_NOT_PRIMARY 非主节点
                if (e.getErrorCode() != -104) {
                    err = e;
                }
            } finally {
                if (db != null) {
                    db.disconnect();
                }
            }
            System.out.println("detachCL end");
        }
        
        public boolean isSuccess() {
            if (err == null) {
                return true;
            } else {
                return false;
            }
        }
        
        public String getErrMsg() {
            return err.getMessage() + Utils.getKeyStack(err, this);
        }
        
        public int getDetachedSclCnt() {
            return detachedSclCnt;
        }
    }
    
    class ContinuousCutNetThread extends Thread {
        private Exception err = null;
        private static final int maxDelay = 1;
        private static final int cutNetTimes = 3;

        @Override
        public void run() {
            System.out.println("brokenNetwork start");
            try {
                delay(maxDelay);
                GroupMgr mgr = new GroupMgr();
                for (int i = 0; i < cutNetTimes; i++) {
                    GroupWrapper cataGroup = groupMgr.getGroupByName("SYSCatalogGroup");
                    String cataPriHost = cataGroup.getMaster().hostName();
                    GroupWrapper dataGroup = groupMgr.getGroupByName(clGroup);
                    String dataPriHost = dataGroup.getMaster().hostName();
                    if (cataPriHost.equals(dataPriHost) && !cataGroup.changePrimary(10)) {
                        throw new ReliabilityException(cataGroup.getGroupName() + " reelect fail");
                    }
                    BrokenNetwork broNet = new BrokenNetwork(dataPriHost, 10);
                    broNet.init();
                    broNet.make();
                    if (broNet.checkMakeResult() == false) {
                        throw new ReliabilityException("fail to make brokenNetwork " + i);
                    }
                    broNet.restore();
                    if (broNet.checkRestoreResult() == false) {
                        throw new ReliabilityException("fail to restore brokenNetwork " + i);
                    }
                    broNet.fini();
                    while (mgr.checkBusiness(false) != true) {}
                }
            } catch (Exception e) {
                err = e;
            }
            System.out.println("brokenNetwork end");
        }
        
        private void delay(int delaySecond) {
            Random random = new Random();
            try {
                Thread.sleep(random.nextInt(delaySecond * 1000));
            } catch (Exception e) {
            }
        }
        
        public boolean isSuccess() {
            if (err == null) {
                return true;
            } else {
                return false;
            }
        }
        
        public String getErrMsg() {
            return err.getMessage() + Utils.getKeyStack(err, this);
        }
    }
}