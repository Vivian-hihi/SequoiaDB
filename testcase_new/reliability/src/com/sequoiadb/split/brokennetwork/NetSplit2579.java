package com.sequoiadb.split.brokennetwork;

import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.List;
import java.util.concurrent.atomic.AtomicBoolean;

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
import com.sequoiadb.exception.FaultException;
import com.sequoiadb.exception.ReliabilityException;
import com.sequoiadb.fault.BrokenNetwork;
import com.sequoiadb.task.FaultMakeTask;
import com.sequoiadb.task.OperateTask;
import com.sequoiadb.task.TaskMgr;

/**
 * @FileName:SEQDB-2579 对range分区组进行百分比切分，切分时源主节点断网
 * 
 * @author huangqiaohui
 * @version 1.00
 *
 */

public class NetSplit2579 extends SdbTestBase {
    private String clName = "testcaseCL2579";
    private String srcGroupName;
    private String destGroupName;
    private GroupMgr groupMgr = null;
    private String connectUrl;
    private boolean clearFlag = false;
    private int exceptionRecNum;

    @BeforeClass()
    public void setUp() {
        Sequoiadb sdb = new Sequoiadb(coordUrl, "", "");
        try {
            System.out.println(
                    "the TestCase Name:" + this.getClass().getName() + ". the TestCase begin at:"
                            + new SimpleDateFormat("YYYY-MM-dd HH:mm:ss.SSS").format(new Date()));
            groupMgr = GroupMgr.getInstance();

            // CheckBusiness(true),检测当前集群环境，若存在异常返回false，
            if (!groupMgr.checkBusiness(true)) {
                throw new SkipException("checkBusiness faile");
            }

            // 确定切分的源和目标组
            List<GroupWrapper> glist = groupMgr.getAllDataGroup();
            srcGroupName = glist.get(0).getGroupName();
            destGroupName = glist.get(1).getGroupName();

            CollectionSpace commCS = sdb.getCollectionSpace(csName);
            DBCollection cl = commCS.createCollection(clName, (BSONObject) JSON.parse(
                    "{ShardingKey:{'sk':1},ShardingType:'range',ReplSize:2,Group:'" + srcGroupName + "'}"));
            // 准备切分的数据
            insertData(cl, 0, 5000);
        }
        catch (ReliabilityException e) {
            Assert.fail(this.getClass().getName() + " setUp error, error description:"
                    + e.getMessage() + "\r\n" + Utils.getStackString(e));
        }
        finally {
            sdb.disconnect();
        }
    }

    public void insertData(DBCollection cl, int begin, int end) {
        for (int i = begin; i < end; i++) {
            BSONObject obj = (BSONObject) JSON.parse("{sk:" + i + "}");
            cl.insert(obj);
        }
    }

    @Test
    public void test() {
        Sequoiadb db = null;
        try {
            // 调整断网主机上的主节点
            GroupWrapper srcGroup = groupMgr.getGroupByName(srcGroupName);
            GroupWrapper destGroup = groupMgr.getGroupByName(destGroupName);
            GroupWrapper cataGroup = groupMgr.getGroupByName("SYSCatalogGroup");
            String srcPriHost = srcGroup.getMaster().hostName();
            String destPriHost = destGroup.getMaster().hostName();
            String cataPriHost = cataGroup.getMaster().hostName();
            // srcSlaHost为断网主机，若目标组主节点，catalog主节点在这台机上，changePrimary将尝试至多10次的切主操作
            if (srcPriHost.equals(destPriHost) && !destGroup.changePrimary(10)) {
                throw new SkipException(destGroup.getGroupName() + " reelect faile");
            }
            if (srcPriHost.equals(cataPriHost) && !cataGroup.changePrimary(10)) {
                throw new SkipException("SYSCataLogGroup reelect faile");
            }

            // 得到一个非断网主机的coordurl
            connectUrl = Utils.getDiffHostWithSvc(srcPriHost, groupMgr.getAllHosts());

            // 建立并行任务
            FaultMakeTask faultTask = BrokenNetwork.getFaultMakeTask(srcPriHost, 5, 15, 25);
            TaskMgr mgr = new TaskMgr(faultTask);
            mgr.addTask(new Split("Split"));
            mgr.addTask(new Insert("insert"));
            mgr.init();
            mgr.start();
            mgr.join();
            mgr.fini();

            // TaskMgr检查线程异常
            Assert.assertEquals(mgr.isAllSuccess(), true, mgr.getErrorMsg());

            // 最长等待20分钟的集群环境恢复
            Assert.assertEquals(Utils.checkBusinessWithTimeout(groupMgr, 20 * 60), true,
                    "wait restore business faile");

            // 再次插入数据
            db = new Sequoiadb(connectUrl, "", "");
            db.setSessionAttr((BSONObject) JSON.parse("{PreferedInstance:'M'}"));
            DBCollection cl = db.getCollectionSpace(csName).getCollection(clName);
            insertData(cl, 50000, 51000);

            // 源和目标数据量比对
            checkGroupData(db, destGroupName, "{sk:{$gte:5000,$lt:50000}}", exceptionRecNum);
            checkGroupData(db, srcGroupName, "{$or:[{sk:{$gte:50000}},{sk:{$lt:5000}}]}", 6000);

            // 组间一致性校验，尝试至多60次，每次间隔1秒
            if (!destGroup.checkInspect(60, 1)) {
                Assert.fail(destGroup.getInspectStdout());
            }
            if (!srcGroup.checkInspect(60, 1)) {
                Assert.fail(srcGroup.getInspectStdout());
            }

            clearFlag = true;
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

    private void checkGroupData(Sequoiadb sdb, String groupName, String macher, int expectCount) {
        Sequoiadb dataNode = null;
        DBCursor cursor = null;
        try {
            dataNode = sdb.getReplicaGroup(groupName).getMaster().connect();
            DBCollection cl = dataNode.getCollectionSpace(csName).getCollection(clName);
            long macherCount = cl.getCount(macher);
            long count = cl.getCount();
            Assert.assertEquals(macherCount == count && count == expectCount, true,
                    destGroupName + " count:" + count + " macherCount:" + macherCount
                            + " expectCount:" + expectCount);
            ;
        }
        catch (BaseException e) {
            e.printStackTrace();
            Assert.fail(e.getMessage() + "\r\n" + Utils.getStackString(e));
        }
        finally {
            if (cursor != null) {
                cursor.close();
            }
            if (dataNode != null) {
                dataNode.disconnect();
            }
        }
    }

    @AfterClass
    public void tearDown() {
        Sequoiadb sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "");
        try {
            if (clearFlag) {
                CollectionSpace commCS = sdb.getCollectionSpace(csName);
                commCS.dropCollection(clName);
            }
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

    class Insert extends OperateTask {
        private AtomicBoolean isCutnet = new AtomicBoolean(false);

        public Insert(String name) {
            super(name);
            // TODO Auto-generated constructor stub
        }

        @Override
        public void exec() throws Exception {
            Sequoiadb db = new Sequoiadb(connectUrl, "", "");
            DBCollection cl = db.getCollectionSpace(csName).getCollection(clName);
            insertDataForThread(cl);
        }

        private void insertDataForThread(DBCollection cl) {
            for (int i = 5000; i < 50000; i++) {
                BSONObject obj = (BSONObject) JSON.parse("{sk:" + i + "}");
                try {
                    cl.insert(obj);
                }
                catch (BaseException e) {
                    if (isCutnet.get()) {
                        System.out.println("insertThread insert record:{sk:" + i + "} :"
                                + e.getMessage() + Utils.getStackString(e));
                        exceptionRecNum = i - 5000;
                        return;
                    }
                    throw e;
                }
            }
            exceptionRecNum = 45000;
        }

        @Override
        public void faultNotify(BSONObject status) throws FaultException {
            OperateTask.faultStatus mk = (faultStatus) status.get(FaultMakeTask.MAKE_RESULT);
            OperateTask.faultStatus rt = (faultStatus) status.get(FaultMakeTask.RESTORE_RESULT);
            if (mk == OperateTask.faultStatus.MAKEFAILURE) {
                throw new FaultException(mk.toString());
            }
            if (mk == OperateTask.faultStatus.MAKESUCCESS) {
                isCutnet.set(true);
                return;
            }
            if (rt == OperateTask.faultStatus.RESTOREFAILURE) {
                throw new FaultException(rt.toString());
            }
        }
    }

    class Split extends OperateTask {
        public Split(String name)

        {
            super(name);
        }

        @Override
        public void exec() throws Exception {
            Sequoiadb sdb = null;
            try {
                sdb = new Sequoiadb(connectUrl, "", "");
                sdb.setSessionAttr((BSONObject) JSON.parse("{PreferedInstance:'M'}"));
                DBCollection cl = sdb.getCollectionSpace(csName).getCollection(clName);
                cl.split(srcGroupName, destGroupName, (BSONObject) JSON.parse("{sk:5000}"), // 切分
                        (BSONObject) JSON.parse("{sk:50000}"));
            }
            catch (BaseException e) {
                throw e;
            }
            finally {
                if (sdb != null) {
                    sdb.disconnect();
                }
            }
        }

        @Override
        public void faultNotify(BSONObject status) {

        }
    }

}
