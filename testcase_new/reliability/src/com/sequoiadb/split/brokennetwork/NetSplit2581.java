package com.sequoiadb.split.brokennetwork;

import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.List;

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
 * @FileName:SEQDB-2581 对range分区组进行百分比切分，切分时目标组主节点断网
 * @author huangqiaohui
 * @version 1.00
 *
 */

public class NetSplit2581 extends SdbTestBase {
    private String clName = "testcaseCL2581";
    private String srcGroupName;
    private String destGroupName;
    private GroupMgr groupMgr = null;
    private String connectUrl;
    private boolean clearFlag = false;

    @BeforeClass()
    public void setUp() {
        Sequoiadb commSdb = new Sequoiadb(coordUrl, "", "");
        try {
            System.out.println(
                    "the TestCase Name:" + this.getClass().getName() + ". the TestCase begin at:"
                            + new SimpleDateFormat("YYYY-MM-dd HH:mm:ss.SSS").format(new Date()));
            groupMgr = GroupMgr.getInstance();

            if (!groupMgr.checkBusiness(true)) {
                throw new SkipException("checkBusiness faile");
            }
            List<GroupWrapper> glist = groupMgr.getAllDataGroup();

            srcGroupName = glist.get(0).getGroupName();
            destGroupName = glist.get(1).getGroupName();

            CollectionSpace commCS = commSdb.getCollectionSpace(csName);
            DBCollection cl = commCS.createCollection(clName, (BSONObject) JSON.parse(
                    "{ShardingKey:{'sk':1},ShardingType:'range',Group:'" + srcGroupName + "'}"));
            insertData(cl, 0, 4000);// 写入待切分的记录（1000普通记录，1000lob）
        }
        catch (ReliabilityException e) {
            Assert.fail(this.getClass().getName() + " setUp error, error description:"
                    + e.getMessage() + "\r\n" + Utils.getStackString(e));
        }
        finally {
            commSdb.disconnect();
        }
    }

    public void insertData(DBCollection cl, int begin, int end) {
        for (int i = begin; i < end; i++) {
            BSONObject obj = (BSONObject) JSON.parse("{sk:" + i + "}");
            cl.insert(obj);
        }
    }

    @Test()
    public void test() {
        Sequoiadb db = null;
        try {
            // 调整断网主机上的主节点
            GroupWrapper srcGroup = groupMgr.getGroupByName(srcGroupName);
            GroupWrapper destGroup = groupMgr.getGroupByName(destGroupName);
            GroupWrapper cataGroup = groupMgr.getGroupByName("SYSCatalogGroup");
            String destPriHost = destGroup.getMaster().hostName();
            String cataPriHost = cataGroup.getMaster().hostName();
            String srcPriHost = srcGroup.getMaster().hostName();
            if (destPriHost.equals(cataPriHost) && !cataGroup.changePrimary(10)) {
                throw new SkipException("SYSCataLogGroup reelect faile");
            }
            if (destPriHost.equals(srcPriHost) && !srcGroup.changePrimary(10)) {
                throw new SkipException(srcGroupName + " reelect faile");
            }

            connectUrl = Utils.getDiffHostWithSvc(destPriHost, groupMgr.getAllHosts());

            // 建立并行任务
            FaultMakeTask faultTask = BrokenNetwork.getFaultMakeTask(destPriHost, 2, 10, 15);
            TaskMgr mgr = new TaskMgr(faultTask);
            mgr.addTask(new Split("Split"));
            mgr.addTask(new Insert("insert"));
            mgr.init();
            mgr.start();
            mgr.join();
            mgr.fini();

            Assert.assertEquals(mgr.isAllSuccess(), true, mgr.getErrorMsg());

            // 最长等待20分钟的环境恢复
            Assert.assertEquals(Utils.checkBusinessWithTimeout(groupMgr, 1200), true,
                    "wait restore business faile");

            db = new Sequoiadb(connectUrl, "", "");
            db.setSessionAttr((BSONObject) JSON.parse("{PreferedInstance:'M'}"));
            DBCollection cl = db.getCollectionSpace(csName).getCollection(clName);
            insertData(cl, 9000, 10000);

            // 结果校验
            if (!destGroup.checkInspect(60, 1)) {
                Assert.fail(destGroup.getInspectStdout());
            }
            if (!srcGroup.checkInspect(60, 1)) {
                Assert.fail(srcGroup.getInspectStdout());
            }
            checkGroupData(db, destGroupName, "{sk:{$gte:4000,$lt:9000}}", 5000);
            checkGroupData(db, srcGroupName, "{$or:[{sk:{$gte:9000}},{sk:{$lt:4000}}]}", 5000);
            clearFlag = true;
        }
        catch (ReliabilityException e) {
            e.printStackTrace();
            Assert.fail(e.getMessage());
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
                    destGroupName + " count:" + count + " macherCount:" + macherCount);
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
        Sequoiadb commSdb = new Sequoiadb(SdbTestBase.coordUrl, "", "");
        try {
            if (clearFlag) {
                CollectionSpace commCS = commSdb.getCollectionSpace(csName);
                commCS.dropCollection(clName);
            }
        }
        catch (BaseException e) {
            Assert.fail(e.getMessage() + "\r\n" + Utils.getStackString(e));
        }
        finally {
            commSdb.disconnect();
            System.out.println(
                    "the TestCase Name:" + this.getClass().getName() + ". the TestCase end at:"
                            + new SimpleDateFormat("YYYY-MM-dd HH:mm:ss.SSS").format(new Date()));
        }
    }

    class Insert extends OperateTask {

        public Insert(String name) {
            super(name);
            // TODO Auto-generated constructor stub
        }

        @Override
        public void exec() throws Exception {
            Sequoiadb db = new Sequoiadb(connectUrl, "", "");
            DBCollection cl = db.getCollectionSpace(csName).getCollection(clName);
            insertData(cl, 4000, 9000);
        }

        @Override
        public void faultNotify(BSONObject status) {
            // TODO Auto-generated method stub

        }
    }

    class Split extends OperateTask {

        public Split(String name) {
            super(name);
        }

        @Override
        public void exec() throws Exception {
            Sequoiadb sdb = null;
            try {
                sdb = new Sequoiadb(connectUrl, "", "");
                sdb.setSessionAttr((BSONObject) JSON.parse("{PreferedInstance:'M'}"));
                DBCollection cl = sdb.getCollectionSpace(csName).getCollection(clName);
                cl.split(srcGroupName, destGroupName, (BSONObject) JSON.parse("{sk:4000}"),
                        (BSONObject) JSON.parse("{sk:9000}"));

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
