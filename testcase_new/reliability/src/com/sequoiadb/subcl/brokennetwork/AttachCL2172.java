package com.sequoiadb.subcl.brokennetwork;

import java.text.SimpleDateFormat;
import java.util.Date;

import org.testng.Assert;
import org.testng.SkipException;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.commlib.GroupMgr;
import com.sequoiadb.commlib.GroupWrapper;
import com.sequoiadb.commlib.SdbTestBase;
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.exception.ReliabilityException;
import com.sequoiadb.fault.BrokenNetwork;
import com.sequoiadb.subcl.commlib.AttachCLTask;
import com.sequoiadb.subcl.commlib.Utils;
import com.sequoiadb.task.FaultMakeTask;
import com.sequoiadb.task.TaskMgr;

/**
 * @FileName seqDB-2172: attachCL过程中dataRG主节点断网
 * @Author linsuqiang
 * @Date 2017-03-14
 * @Version 1.00
 */

/*
 * 1、创建主表和子表
 * 2、批量执行db.collectionspace.collection.attachCL()挂载多个子表 
 * 3、子表挂载过程中将dataRG主节点网络断掉（如：使用cutnet.sh工具，命令格式为nohup ./cutnet.sh &），检查attachCL执行结果 
 * 4、将dataRG主节点网络恢复，并对挂载成功的子表对应的主表做基本操作（如insert)
 */

public class AttachCL2172 extends SdbTestBase {
    private GroupMgr groupMgr = null;
    private boolean runSuccess = false;
    private String mclName = "cl_2172";
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
            FaultMakeTask faultTask = BrokenNetwork.getFaultMakeTask(dataPriHost, 1, 10, 10);
            TaskMgr mgr = new TaskMgr(faultTask);
            String safeUrl = Utils.getDiffHostWithSvc(dataPriHost, groupMgr.getAllHosts());
            // -104 SDB_CLS_NOT_PRIMARY 非主节点
            int[] expErrCodes = {-104};
            AttachCLTask aTask = new AttachCLTask("attachCL", mclName, safeUrl, expErrCodes);
            mgr.addTask(aTask);
            mgr.init();
            mgr.start();
            mgr.join();
            mgr.fini();

            // 检验任务结果
            Assert.assertEquals(mgr.isAllSuccess(), true, mgr.getErrorMsg());

            // 等待集群恢复
            while (groupMgr.checkBusinessWithLSN(false) != true) {}
            
            // 检查用例结果
            Utils.checkConsistency(cataGroup);
            db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
            Utils.checkIntegrated(db, mclName);
            Utils.checkAttached(db, mclName, aTask.getAttachedSclCnt());
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
        Sequoiadb db = null;
        try {
            db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
            Utils.dropMclAndScl(db, mclName);
        } catch (BaseException e) {
            Assert.fail(e.getMessage() + "\r\n" + Utils.getKeyStack(e, this));
        } finally {
            if (db != null) {
                db.disconnect();
            }
            System.out.println("the TestCase Name:" + this.getClass().getName() + ". the TestCase end at:"
                    + new SimpleDateFormat("YYYY-MM-dd HH:mm:ss.SSS").format(new Date()));
        }
    }
}