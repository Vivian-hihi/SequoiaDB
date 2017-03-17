package com.sequoiadb.subcl.brokennetwork;

import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.Random;

import org.bson.BSONObject;
import org.bson.util.JSON;
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
 * @FileName seqDB-2171: attachCL过程中catalog主节点连续降备
 * @Author linsuqiang
 * @Date 2017-03-14
 * @Version 1.00
 */

/*
 * 1、创建主表和子表 
 * 2、批量执行db.collectionspace.collection.attachCL()挂载多个子表 
 * 3、子表挂载过程中将catalog主节点网络断掉（如：使用cutnet.sh工具，命令格式为nohup ./cutnet.sh &），检查attachCL执行结果
 * 4、catalog新选主后立即将新主的网络断掉 
 * 5、重复步骤4两到三遍 
 * 6、将catalog主节点网络恢复，并对挂载成功的子表对应的主表做基本操作（如insert)
 */

public class AttachCL2171 extends SdbTestBase {
    private GroupMgr groupMgr = null;
    private boolean runSuccess = false;
    private String mclName = "cl_2171";

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
            Utils.createMclAndScl(sdb, mclName);
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
            GroupWrapper cataGroup = groupMgr.getGroupByName("SYSCatalogGroup");
            String cataPriHost = cataGroup.getMaster().hostName();
            
            // 建立并行任务
            String safeUrl = Utils.getDiffHostWithSvc(cataPriHost, groupMgr.getAllHosts());
            AttachCLThread aThread = new AttachCLThread(safeUrl);
            ContinuousCutNetThread cThread = new ContinuousCutNetThread();
            aThread.start();
            cThread.start();
            aThread.join();
            cThread.join();

            // 检验任务结果
            if (!aThread.isSuccess()) {
                Assert.fail("attachCL failed: " + aThread.getErrMsg());
            }
            if (!cThread.isSuccess()) {
                Assert.fail("brokenNetwork failed: " + cThread.getErrMsg());
            }

            // 等待集群恢复
            while (groupMgr.checkBusinessWithLSN(false) != true) {}
            
            // 检查用例结果
            Utils.checkConsistency(cataGroup);
            db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
            Utils.checkIntegrated(db, mclName);
            Utils.checkAttached(db, mclName, aThread.getAttachedSclCnt());
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

    class AttachCLThread extends Thread {
        private int attachedSclCnt = 0;
        private Exception err = null;
        private String safeUrl = null;
        
        public AttachCLThread(String safeUrl) {
            this.safeUrl = safeUrl;
        }

        @Override
        public void run() {
            System.out.println("attachCL start");
            Sequoiadb db = null;
            try {
                db = new Sequoiadb(safeUrl, "", "");
                CollectionSpace cs = db.getCollectionSpace(csName);
                DBCollection mcl = cs.getCollection(mclName);
                int rangeStart = 0;
                for (int i = 0; i < Utils.SCLNUM; i++) {
                    int rangeEnd = rangeStart + Utils.RANGE_WIDTH;
                    String sclFullName = csName + "." + mclName + "_" + i;
                    mcl.attachCollection(sclFullName, (BSONObject) JSON.parse("{ LowBound: { a: " + rangeStart
                            + " }, " + "UpBound: { a: " + rangeEnd + " } }"));
                    rangeStart += Utils.RANGE_WIDTH;
                    attachedSclCnt++;
                }
            } catch (BaseException e) {
                // -134 SDB_COORD_REMOTE_DISC 远程节点断开连接
                // -104 SDB_CLS_NODE_NOT_PRIMARY 非主节点
                // -252 SDB_CLS_WAIT_SYNC_FAILED 等待备节点同步该操作失败
                if (e.getErrorCode() != -134 && e.getErrorCode() != -104 && e.getErrorCode() != -252) {
                    err = e;
                }
            } catch (Exception e) {
                err = e;
            } finally {
                if (db != null) {
                    db.disconnect();
                }
                System.out.println("attachCL end");
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
        
        public int getAttachedSclCnt() {
            return attachedSclCnt;
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
                    BrokenNetwork broNet = new BrokenNetwork(cataPriHost, 10);
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
            } catch (ReliabilityException e) {
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