package com.sequoiadb.clsmgr.brokennetwork;

import java.text.SimpleDateFormat;
import java.util.Date;

import org.bson.BSONObject;
import org.bson.BasicBSONObject;
import org.testng.Assert;
import org.testng.SkipException;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import com.sequoiadb.base.Node;
import com.sequoiadb.base.ReplicaGroup;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.commlib.GroupMgr;
import com.sequoiadb.commlib.GroupWrapper;
import com.sequoiadb.commlib.SdbTestBase;
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.exception.FaultException;
import com.sequoiadb.exception.ReliabilityException;
import com.sequoiadb.fault.BrokenNetwork;
import com.sequoiadb.task.FaultMakeTask;
import com.sequoiadb.task.OperateTask;
import com.sequoiadb.task.TaskMgr;

/**
 * @FileName:SEQDB-6201 删除coord节点过程中catalog主节点断网
 * @author huangqiaohui
 * @version 1.00
 *
 */

public class NetDeleteNode6201 extends SdbTestBase {
    private GroupMgr groupMgr = null;
    private int coordPort = 26666;
    private String coordDbPath = SdbTestBase.reservedDir;
    private String connectUrl;
    private boolean deleteFlag = false;

    @BeforeClass()
    public void setUp() {
        Sequoiadb sdb = new Sequoiadb(coordUrl, "", "");
        try {
            System.out.println(
                    "the TestCase Name:" + this.getClass().getName() + ". the TestCase begin at:"
                            + new SimpleDateFormat("YYYY-MM-dd HH:mm:ss.SSS").format(new Date()));
            groupMgr = GroupMgr.getInstance();

            // CheckBusiness(true),检测当前集群环境，若存在异常返回false，
            if (!groupMgr.checkBusiness()) {
                throw new SkipException("checkBusiness faile");
            }

        }
        catch (ReliabilityException e) {
            Assert.fail(this.getClass().getName() + " setUp error, error description:"
                    + e.getMessage() + "\r\n" + Utils.getStackString(e));
        }
        finally {
            sdb.disconnect();
        }
    }

    @Test
    public void test() {
        Sequoiadb db = null;
        try {
            GroupWrapper cataGroup = groupMgr.getGroupByName("SYSCatalogGroup");
            String cataPriHost = cataGroup.getMaster().hostName();

            // 得到一个非断网主机的coordurl
            connectUrl = Utils.getDiffHostWithSvc(cataPriHost, groupMgr.getAllHosts());

            db = new Sequoiadb(connectUrl, "", "");
            ReplicaGroup coordGroup = db.getReplicaGroup("SYSCoord");
            Node coordNode = coordGroup.createNode(connectUrl.split(":")[0], coordPort,
                    coordDbPath + "/" + coordPort, new BasicBSONObject());
            coordNode.start();

            // 建立并行任务
            System.out.println("cata:" + cataPriHost);
            FaultMakeTask faultTask = BrokenNetwork.getFaultMakeTask(cataPriHost, 1, 10, 15);
            TaskMgr mgr = new TaskMgr(faultTask);
            mgr.addTask(new RemoveCoord("removeCoord"));
            mgr.init();
            mgr.start();
            mgr.join();
            mgr.fini();
            // TaskMgr检查线程异常
            Assert.assertEquals(mgr.isAllSuccess(), true, mgr.getErrorMsg());

            // 最长等待20分钟的集群环境恢复
            Assert.assertEquals(Utils.checkBusinessWithTimeout(groupMgr, 1200), true,
                    "wait restore business faile");

            if (deleteFlag) {
                Assert.assertEquals(groupMgr.checkResidu(), true);
            }
            else {
                coordNode = coordGroup.createNode(connectUrl.split(":")[0], coordPort,
                        coordDbPath + "/" + coordPort, new BasicBSONObject());
                coordNode.start();
                coordNode.connect().disconnect();
                coordGroup.removeNode(connectUrl.split(":")[0], coordPort, null);
            }
        }
        catch (ReliabilityException e) {
            Assert.fail(e.getMessage() + "\r\n" + Utils.getStackString(e));
        }
        finally {
            if (db != null) {
                db.disconnect();
            }
        }

    }

    @AfterClass
    public void tearDown() {
        Sequoiadb sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "");
        try {

        }
        catch (BaseException e) {
            Assert.fail(e.getMessage() + "\r\n" + Utils.getStackString(e));
        }
        finally {
            sdb.disconnect();
            System.out.println(
                    "the TestCase Name:" + this.getClass().getName() + ". the TestCase end at:"
                            + new SimpleDateFormat("YYYY-MM-dd HH:mm:ss.SSS").format(new Date()));
        }
    }

    class RemoveCoord extends OperateTask {
        public RemoveCoord(String name) {
            super(name);
            // TODO Auto-generated constructor stub
        }

        @Override
        public void exec() throws Exception {
            System.out.println("thread:" + connectUrl);
            try {
                Sequoiadb db = new Sequoiadb(connectUrl, "", "");
                ReplicaGroup coordGroup = db.getReplicaGroup("SYSCoord");
                coordGroup.removeNode(connectUrl.split(":")[0], coordPort, null);
                deleteFlag = true;
            }
            catch (BaseException e) {
                System.out.println(e.getMessage());
                if (e.getErrorCode() != -134) {
                    throw e;
                }
            }
        }

        @Override
        public void faultNotify(BSONObject status) throws FaultException {
            OperateTask.faultStatus mk = (faultStatus) status.get(FaultMakeTask.MAKE_RESULT);
            OperateTask.faultStatus rt = (faultStatus) status.get(FaultMakeTask.RESTORE_RESULT);
            if (mk == OperateTask.faultStatus.MAKEFAILURE) {
                throw new FaultException(mk.toString());
            }
            if (rt == OperateTask.faultStatus.RESTOREFAILURE) {
                throw new FaultException(rt.toString());
            }
        }
    }

}
