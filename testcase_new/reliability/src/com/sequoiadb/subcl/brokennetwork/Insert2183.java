package com.sequoiadb.subcl.brokennetwork;

import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.Random;

import org.bson.BSONObject;
import org.testng.Assert;
import org.testng.SkipException;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import com.sequoiadb.base.CollectionSpace;
import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.DBCursor;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.commlib.GroupMgr;
import com.sequoiadb.commlib.GroupWrapper;
import com.sequoiadb.commlib.SdbTestBase;
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.exception.ReliabilityException;
import com.sequoiadb.fault.BrokenNetwork;
import com.sequoiadb.subcl.commlib.Utils;

/**
 * @FileName seqDB-2183: 在主表insert大量数据时dataRG主节点连续降备
 * @Author linsuqiang
 * @Date 2017-03-15
 * @Version 1.00
 */

/*
 * 1、创建主表和子表（分区方式：主表range，子表hash，AutoSplit：true,多个分区键） 
 * 2、在主表插入大量数据（如每个子表插入10万条数据），
 * 3、插入数据过程中将dataRG主节点网络断掉（如：使用cutnet.sh工具，命令格式为nohup ./cutnet.sh &），检查insert执行结果 
 * 4、dataRG重新选主后立即将新主的网络断掉
 * 5、重复步骤4两道三遍 
 * 6、将dataRG备节点网络恢复，查询原操作对应主表数据是否完整一致，并重新对主表做基本操作（如insert） 
 */

public class Insert2183 extends SdbTestBase {
    private GroupMgr groupMgr = null;
    private boolean runSuccess = false;
    private String mclName = "cl_2183";
    private String clGroup = null;
    private static final int SCLNUM = Utils.SCLNUM;
    private static final int RANGE_WIDTH = Utils.RANGE_WIDTH;

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
            // 调整断网主机上的主节点
            GroupWrapper cataGroup = groupMgr.getGroupByName("SYSCatalogGroup");
            String cataPriHost = cataGroup.getMaster().hostName();
            GroupWrapper dataGroup = groupMgr.getGroupByName(clGroup);
            String dataPriHost = dataGroup.getMaster().hostName();
            if (cataPriHost.equals(dataPriHost) && !cataGroup.changePrimary(10)) {
                throw new SkipException(cataGroup.getGroupName() + " reelect fail");
            }
            
            // 建立并行任务
            String safeUrl = Utils.getDiffHostWithSvc(dataPriHost, groupMgr.getAllHosts());
            InsertThread iThread = new InsertThread(safeUrl);
            ContinuousCutNetThread cThread = new ContinuousCutNetThread();
            iThread.start();
            cThread.start();
            iThread.join();
            cThread.join();

            // 检验任务结果
            if (!iThread.isSuccess()) {
                Assert.fail("insert failed: " + iThread.getErrMsg());
            }
            if (!cThread.isSuccess()) {
                Assert.fail("brokenNetwork failed: " + cThread.getErrMsg());
            }

            // 等待集群恢复
            while (groupMgr.checkBusinessWithLSN(false) != true) {}
            
            // 检查用例结果
            dataGroup.checkInspect(10, 1);
            db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
            checkInserted(db, iThread.getInsertedCnt());
            checkUsable(db);
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

    class InsertThread extends Thread {
        private Exception err = null;
        private int insertedCnt = 0;
        private String safeUrl = null;
        private static final int RECORD_TOTAL = 100000;
        
        public InsertThread(String safeUrl) {
            this.safeUrl = safeUrl;
        }

        @Override
        public void run() {
            System.out.println("insert start");
            Sequoiadb db = null;
            try {
                db = new Sequoiadb(safeUrl, "", "");
                CollectionSpace cs = db.getCollectionSpace(csName);
                DBCollection mcl = cs.getCollection(mclName);
                int mclRange = SCLNUM * RANGE_WIDTH;
                for (int i = 0; i < RECORD_TOTAL; i++) {
                    int valueInRange = i % mclRange;
                    mcl.insert("{ a: " + valueInRange + " }");
                    insertedCnt++;
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
            System.out.println("insert end");
        }
        
        public int getInsertedCnt() {
            return insertedCnt;
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
                    // 设置断网节点情况
                    GroupWrapper cataGroup = groupMgr.getGroupByName("SYSCatalogGroup");
                    String cataPriHost = cataGroup.getMaster().hostName();
                    GroupWrapper dataGroup = groupMgr.getGroupByName(clGroup);
                    String dataPriHost = dataGroup.getMaster().hostName();
                    if (cataPriHost.equals(dataPriHost) && !cataGroup.changePrimary(10)) {
                        throw new SkipException(cataGroup.getGroupName() + " reelect fail");
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
    
    private void checkInserted(Sequoiadb db, int insertedCnt) {
        DBCollection mcl = db.getCollectionSpace(csName).getCollection(mclName);
        if (mcl.getCount() < insertedCnt) {
            Assert.fail("records count is less then the expected.");
        }
        DBCursor cursor = mcl.query(null, null, "{ _id: 1 }", null);
        int mclRange = SCLNUM * RANGE_WIDTH;
        for (int i = 0; i < insertedCnt; i++) {
            BSONObject res = cursor.getNext();
            int expValue = i % mclRange;
            int actValue = (int)res.get("a");
            if (actValue != expValue) {
                Assert.fail("fail to checkInserted. expected: " + expValue + " but found: " + actValue);
            }
        }
        cursor.close();
    }

    private void checkUsable(Sequoiadb db) throws ReliabilityException {
        try {
            DBCollection mcl = db.getCollectionSpace(csName).getCollection(mclName);
            for (int i = 0; i < SCLNUM; i++) {
                int lowBound = i * RANGE_WIDTH;
                int upBound = (i + 1) * RANGE_WIDTH - 1;
                mcl.insert("{ a: " + lowBound + ", b: " + i + " }");
                mcl.insert("{ a: " + upBound + ", b: " + i + " }");
                if (mcl.getCount("{ b: " + i + " }") != 2) {
                    Assert.fail("scl " + i + " is not usable");
                }
            }
        } catch (BaseException e) {
            Assert.fail(e.getMessage());
        }
    }
}