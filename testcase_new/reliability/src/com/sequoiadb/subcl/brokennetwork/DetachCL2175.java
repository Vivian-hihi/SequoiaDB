package com.sequoiadb.subcl.brokennetwork;

import java.text.SimpleDateFormat;
import java.util.Date;

import org.testng.Assert;
import org.testng.SkipException;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.commlib.CommLib;
import com.sequoiadb.commlib.GroupMgr;
import com.sequoiadb.commlib.GroupWrapper;
import com.sequoiadb.commlib.SdbTestBase;
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.exception.ReliabilityException;
import com.sequoiadb.fault.BrokenNetwork;
import com.sequoiadb.subcl.brokennetwork.commlib.DetachCLTask;
import com.sequoiadb.subcl.brokennetwork.commlib.Utils;
import com.sequoiadb.task.FaultMakeTask;
import com.sequoiadb.task.TaskMgr;

/**
 * @FileName seqDB-2175: detachCL过程中catalog主节点断网
 * @Author linsuqiang
 * @Date 2017-03-15
 * @Version 1.00
 */

/*
 * 1、创建主表和子表 
 * 2、批量执行db.collectionspace.collection.detachCL()分离多个子表 
 * 3、子表分离过程中将catalog主节点网络断掉（如：使用cutnet.sh工具，命令格式为nohup ./cutnet.sh &），检查detachCL执行结果
 * 4、将catalog主节点网络恢复，检查catalog主节点CL编目信息跟备节点编目信息是否完整一致 
 * 5、在已经分离子表（普通表）做基本操作（如insert)，检查CL功能正确性 
 */

public class DetachCL2175 extends SdbTestBase {
    private GroupMgr groupMgr = null;
    private boolean runSuccess = false;
    private String mclName = "cl_2175";

    @BeforeClass
    public void setUp() {
        Sequoiadb db = null;
        try {
            System.out.println("the TestCase Name:" + this.getClass().getName() + ". the TestCase begin at:"
                    + new SimpleDateFormat("YYYY-MM-dd HH:mm:ss.SSS").format(new Date()));

            groupMgr = GroupMgr.getInstance();
            if (!groupMgr.checkBusiness()) {
                throw new SkipException("checkBusiness failed");
            }

            db = new Sequoiadb(coordUrl, "", "");
            Utils.createMclAndScl(db, mclName);
            Utils.attachAllScl(db, mclName);
        } catch (ReliabilityException e) {
            Assert.fail(this.getClass().getName() + " setUp error, error description:" + e.getMessage() + "\r\n"
                    + Utils.getKeyStack(e, this));
        } finally {
            if (db != null) {
                db.disconnect();
            }
        }
    }

    @Test
    public void test() {
        Sequoiadb db = null;
        try {
            GroupWrapper cataGroup = groupMgr.getGroupByName("SYSCatalogGroup");
            String cataPriHost = cataGroup.getMaster().hostName();

            FaultMakeTask faultTask = BrokenNetwork.getFaultMakeTask(cataPriHost, 1, 10);
            TaskMgr mgr = new TaskMgr(faultTask);
            String safeUrl = CommLib.getSafeCoordUrl(cataPriHost);
            DetachCLTask aTask = new DetachCLTask(mclName, safeUrl);
            mgr.addTask(aTask);
            mgr.execute();
            Assert.assertEquals(mgr.isAllSuccess(), true, mgr.getErrorMsg());

            if (!groupMgr.checkBusinessWithLSN(300)) { Assert.fail("checkBusinessWithLSN() occurs timeout"); }
            
            Utils.checkConsistency(cataGroup);
            db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
            Utils.checkIntegrated(db, mclName);
            Utils.checkDetached(db, mclName, aTask.getDetachedSclCnt());
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