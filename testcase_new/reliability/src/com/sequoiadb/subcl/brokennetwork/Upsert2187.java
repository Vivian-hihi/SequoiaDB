package com.sequoiadb.subcl.brokennetwork;

import java.text.SimpleDateFormat;
import java.util.Date;

import org.bson.BSONObject;
import org.bson.util.JSON;
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
import com.sequoiadb.task.FaultMakeTask;
import com.sequoiadb.task.OperateTask;
import com.sequoiadb.task.TaskMgr;

/**
 * @FileName seqDB-2187: 在主表upsert大量数据时dataRG备节点断网
 * @Author linsuqiang
 * @Date 2017-03-16
 * @Version 1.00
 */

/* 
 * 1、创建主表和子表（分区方式：主表range，子表hash，AutoSplit：false） 
 * 2、在主表更新大量数据，更新数据过程中将dataRG备节点网络断掉（如：使用cutnet.sh工具，命令格式为nohup ./cutnet.sh &），检查upser/update执行结果 
 * 3、将dataRG备节点网络恢复，查询dataRG备节点数据是否完整一致 
 */

public class Upsert2187 extends SdbTestBase {
    private GroupMgr groupMgr = null;
    private boolean runSuccess = false;
    private String mclName = "cl_2187";
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
            String dataSlvHost = dataGroup.getSlave().hostName();
            if (cataPriHost.equals(dataSlvHost) && !cataGroup.changePrimary(10)) {
                throw new SkipException(cataGroup.getGroupName() + " reelect fail");
            }
            
            // 建立并行任务
            FaultMakeTask faultTask = BrokenNetwork.getFaultMakeTask(dataSlvHost, 0, 10, 10);
            TaskMgr mgr = new TaskMgr(faultTask);
            String safeUrl = Utils.getDiffHostWithSvc(dataSlvHost, groupMgr.getAllHosts());
            UpsertTask iTask = new UpsertTask("upsert", safeUrl);
            mgr.addTask(iTask);
            mgr.init();
            mgr.start();
            mgr.join();
            mgr.fini();

            // 检验任务结果
            Assert.assertEquals(mgr.isAllSuccess(), true, mgr.getErrorMsg());

            // 等待集群恢复
            while (groupMgr.checkBusinessWithLSN(false) != true) {}
            
            // 检查用例结果
            dataGroup.checkInspect(10, 1);
            db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
            checkUpserted(db, iTask.getInsertedCnt());
            checkUsable(db);
            runSuccess = true;
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

    class UpsertTask extends OperateTask {
        private int insertedCnt = 0;
        private String safeUrl = null;
        private static final int RECORD_TOTAL = 10000;
        
        public UpsertTask(String name, String safeUrl) {
            super(name);
            this.safeUrl = safeUrl;
        }

        @Override
        public void exec() throws Exception {
            Sequoiadb db = null;
            try {
                db = new Sequoiadb(safeUrl, "", "");
                CollectionSpace cs = db.getCollectionSpace(csName);
                DBCollection mcl = cs.getCollection(mclName);
                int mclRange = SCLNUM * RANGE_WIDTH;
                for (int i = 0; i < RECORD_TOTAL; i++) {
                    int valueInRange = i % mclRange;
                    BSONObject matcher = (BSONObject)JSON.parse("{ i: " + i + " }");
                    BSONObject modifier = (BSONObject)JSON.parse("{ $set: { a: " + valueInRange + " } }");
                    BSONObject setOnIns = (BSONObject)JSON.parse("{ b: " + valueInRange + " }");
                    mcl.upsert(matcher, modifier, null, setOnIns);
                    insertedCnt++;
                }
            } catch (BaseException e) {
                // -104 SDB_CLS_NOT_PRIMARY 非主节点
                if (e.getErrorCode() != -104) {
                    throw e;
                }
            } finally {
                if (db != null) {
                    db.disconnect();
                }
            }
        }
        
        public int getInsertedCnt() {
            return insertedCnt;
        }

        // 这是几乎通用的，建议抽出。
        @Override
        public void faultNotify(BSONObject status) {
            if (status.get(FaultMakeTask.MAKE_RESULT) == OperateTask.faultStatus.MAKEFAILURE) {
                Assert.fail("fail to make fault");
            }
            if (status.get(FaultMakeTask.RESTORE_RESULT) == OperateTask.faultStatus.RESTOREFAILURE) {
                Assert.fail("fail to restore fault");
            }
        }
    }
    
    private void checkUpserted(Sequoiadb db, int insertedCnt) {
        DBCollection mcl = db.getCollectionSpace(csName).getCollection(mclName);
        if (mcl.getCount() < insertedCnt) {
            Assert.fail("records count is less then the expected.");
        }
        DBCursor cursor = mcl.query(null, null, "{ _id: 1 }", null);
        int mclRange = SCLNUM * RANGE_WIDTH;
        for (int i = 0; i < insertedCnt; i++) {
            BSONObject res = cursor.getNext();
            int expValue = i % mclRange;
            int iField = (int)res.get("i");
            int aField = (int)res.get("a");
            int bField = (int)res.get("b");
            if (iField != i) {
                Assert.fail("fail to checkInserted. i field expected: " + i + " but found: " + iField);
            }
            if (aField != expValue) {
                Assert.fail("fail to checkInserted. a field expected: " + expValue + " but found: " + aField);
            }
            if (bField != expValue) {
                Assert.fail("fail to checkInserted. b field expected: " + expValue + " but found: " + bField);
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
                mcl.insert("{ a: " + lowBound + ", d: " + i + " }");
                mcl.insert("{ a: " + upBound + ", d: " + i + " }");
                if (mcl.getCount("{ d: " + i + " }") != 2) {
                    Assert.fail("scl " + i + " is not usable");
                }
            }
        } catch (BaseException e) {
            Assert.fail(e.getMessage());
        }
    }
}