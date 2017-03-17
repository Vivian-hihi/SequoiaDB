package com.sequoiadb.split.brokennetwork;

import java.text.SimpleDateFormat;
import java.util.ArrayList;
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
 * @FileName:SEQDB-2577 对hash分区组进行百分比切分，切分时catalog主节点断网
 * @author huangqiaohui
 * @version 1.00
 *
 */

public class NetSplit2577 extends SdbTestBase {
    private String clName = "testcaseCL2577";
    private String srcGroupName;
    private String destGroupName;
    private GroupMgr groupMgr = null;
    private int clTotalCount;
    private String connectUrl;
    private boolean clearFlag = false;

    @BeforeClass()
    public void setUp() {
        Sequoiadb sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "");
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

            CollectionSpace commCS = sdb.getCollectionSpace(csName);
            DBCollection cl = commCS.createCollection(clName,
                    (BSONObject) JSON
                            .parse("{ShardingKey:{'sk':1},Partition:4096,ShardingType:'hash',Group:'"
                                    + srcGroupName + "'}"));
            insertData(cl, 0, 2000);
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
        clTotalCount = clTotalCount + (end - begin);
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
            if (cataPriHost.equals(srcPriHost) && !srcGroup.changePrimary(10)) {
                throw new SkipException(srcGroupName + " reelect faile");
            }
            if (cataPriHost.equals(destPriHost) && !destGroup.changePrimary(10)) {
                throw new SkipException(destGroupName + " reelect faile");
            }

            connectUrl = Utils.getDiffHostWithSvc(cataPriHost, groupMgr.getAllHosts());

            // 建立并行任务
            FaultMakeTask faultTask = BrokenNetwork.getFaultMakeTask(cataPriHost, 2, 10, 15);
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

            // 再次插入数据
            db = new Sequoiadb(connectUrl, "", "");
            db.setSessionAttr((BSONObject) JSON.parse("{PreferedInstance:'M'}"));
            DBCollection cl = db.getCollectionSpace(csName).getCollection(clName);
            insertData(cl, 8000, 10000);

            // 结果校验
            if (!destGroup.checkInspect(60, 1)) {
                Assert.fail(destGroup.getInspectStdout());
            }
            if (!srcGroup.checkInspect(60, 1)) {
                Assert.fail(srcGroup.getInspectStdout());
            }

            long destCount = checkGroupData(db, destGroupName);
            long srcCount = checkGroupData(db, srcGroupName);
            Assert.assertEquals(destCount + srcCount, clTotalCount);
            for (int i = 0; i < 10; i++) {
                if (CheckCatalog(cataGroup)) {
                    clearFlag = true;
                    return;
                }
                Thread.sleep(5000);
            }
            Assert.fail("wait catalog synchronize timeout");
        }
        catch (ReliabilityException | InterruptedException e) {
            e.printStackTrace();
            Assert.fail(e.getMessage());
        }
        finally {
            if (db != null) {
                db.disconnect();
            }
        }

    }

    private boolean CheckCatalog(GroupWrapper cataGroup) throws ReliabilityException {
        cataGroup.refresh();
        List<String> urls = cataGroup.getAllUrls();
        ArrayList<BSONObject> res = new ArrayList<BSONObject>();
        for (String url : urls) {
            Sequoiadb db = new Sequoiadb(url, "", "");
            DBCollection cl = db.getCollectionSpace("SYSCAT").getCollection("SYSCOLLECTIONS");
            DBCursor cursor = cl.query(
                    (BSONObject) JSON.parse("{Name:'" + csName + "." + clName + "'}"), null, null,
                    null);
            if (cursor.hasNext()) {
                res.add(cursor.getNext());
                if (cursor.hasNext()) {
                    Assert.fail(cursor.getNext() + " " + res.toString());
                }
            }
            else {
                return false;
            }
        }
        for (BSONObject obj : res) {
            if (!res.get(0).equals(obj)) {
                return false;
            }
        }
        return true;
    }

    private long checkGroupData(Sequoiadb sdb, String groupName) {
        Sequoiadb dataNode = null;
        DBCursor cursor = null;
        try {
            dataNode = sdb.getReplicaGroup(groupName).getMaster().connect();// 获得目标组主节点链接
            DBCollection cl = dataNode.getCollectionSpace(csName).getCollection(clName);
            long count = cl.getCount();
            // 组的数据量应该在clTotalCount/2条左右（切分范围2048-4096）
            Assert.assertEquals(
                    count > clTotalCount / 2 - (clTotalCount / 2 * 0.3)
                            && count < clTotalCount / 2 + (clTotalCount / 2 * 0.3),
                    true, "destGroup data count:" + count);
            return count;
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
        return 0;
    }

    @AfterClass
    public void tearDown() {
        Sequoiadb db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
        try {
            if (clearFlag) {
                CollectionSpace commCS = db.getCollectionSpace(csName);
                commCS.dropCollection(clName);
            }
        }
        catch (BaseException e) {
            Assert.fail(e.getMessage() + "\r\n" + Utils.getStackString(e));
        }
        finally {
            db.disconnect();
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
            insertData(cl, 2000, 10000);
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
                cl.split(srcGroupName, destGroupName, 50);
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
