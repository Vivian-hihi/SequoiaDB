package com.sequoiadb.split.killnode;

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
import com.sequoiadb.exception.ReliabilityException;
import com.sequoiadb.fault.KillNode;
import com.sequoiadb.task.FaultMakeTask;
import com.sequoiadb.task.OperateTask;
import com.sequoiadb.task.TaskMgr;

/**
 * @FileName:SEQDB-2757 对hash分区组进行范围切分，切分时源主节点异常重启
 * @author huangqiaohui
 * @version 1.00
 *
 */

public class KillNodeSplit2757 extends SdbTestBase {
    private String clName = "testcaseCL2757";
    private String srcGroupName;
    private String destGroupName;
    private GroupMgr groupMgr = null;
    private int totalCount;
    private Sequoiadb commSdb;
    private boolean clearFlag = false;

    @BeforeClass()
    public void setUp() {
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

            commSdb = new Sequoiadb(SdbTestBase.coordUrl, "", "");
            CollectionSpace commCS = commSdb.getCollectionSpace(csName);
            DBCollection cl = commCS.createCollection(clName,
                    (BSONObject) JSON
                            .parse("{ShardingKey:{'sk':1},Partition:4096,ShardingType:'hash',Group:'"
                                    + srcGroupName + "'}"));
            // 准备切分的数据
            insertData(cl, 0, 10000);
        }
        catch (ReliabilityException e) {
            if (commSdb != null) {
                commSdb.disconnect();
            }
            Assert.fail(this.getClass().getName() + " setUp error, error description:"
                    + e.getMessage() + "\r\n" + Utils.getStackString(e));
        }
    }

    public void insertData(DBCollection cl, int begin, int end) {
        for (int i = begin; i < end; i++) {
            BSONObject obj = (BSONObject) JSON.parse("{sk:" + i + "}");
            cl.insert(obj);
        }
        totalCount = totalCount + end - begin;
    }

    @Test
    public void test() {
        try {
            // 获取源组主节点的主机名端口号
            GroupWrapper srcGroup = groupMgr.getGroupByName(srcGroupName);
            GroupWrapper destGroup = groupMgr.getGroupByName(destGroupName);
            String srcPriHost = srcGroup.getMaster().hostName();
            String srcSvcName = srcGroup.getMaster().svcName();
            System.out.println("KillNode:" + srcPriHost + ":" + srcSvcName);

            // 建立并行任务
            FaultMakeTask faultTask = KillNode.getFaultMakeTask(srcPriHost, srcSvcName, 1, 30);
            TaskMgr mgr = new TaskMgr(faultTask);
            mgr.addTask(new Split("Split"));
            mgr.init();
            mgr.start();
            mgr.join();
            mgr.fini();

            // TaskMgr检查线程异常
            Assert.assertEquals(mgr.isAllSuccess(), true, mgr.getErrorMsg());

            // 最长等待20分钟的集群环境恢复
            Assert.assertEquals(Utils.checkBusinessLSNWithTimeout(groupMgr, 1200), true,
                    "wait restore business faile");

            // 再次插入数据
            commSdb.setSessionAttr((BSONObject) JSON.parse("{PreferedInstance:'M'}"));
            DBCollection cl = commSdb.getCollectionSpace(csName).getCollection(clName);
            insertData(cl, 10000, 11000);

            // 源和目标数据量比对
            long destCount = checkGroupData(commSdb, destGroupName);
            long srcCount = checkGroupData(commSdb, srcGroupName);
            Assert.assertEquals(srcCount + destCount, totalCount);
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
    }

    private long checkGroupData(Sequoiadb sdb, String groupName) {
        Sequoiadb dataNode = null;
        DBCursor cursor = null;
        try {
            dataNode = sdb.getReplicaGroup(groupName).getMaster().connect();// 获得目标组主节点链接
            DBCollection cl = dataNode.getCollectionSpace(csName).getCollection(clName);
            long count = cl.getCount();
            // 组的数据量应该在totalCount / 2条左右（切分范围2048-4096）
            Assert.assertEquals(
                    count > totalCount / 2 - (totalCount / 2 * 0.3)
                            && count < totalCount / 2 + (totalCount / 2 * 0.3),
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
            if (commSdb != null) {
                commSdb.disconnect();
            }
            System.out.println(
                    "the TestCase Name:" + this.getClass().getName() + ". the TestCase end at:"
                            + new SimpleDateFormat("YYYY-MM-dd HH:mm:ss.SSS").format(new Date()));
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
                sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "");
                sdb.setSessionAttr((BSONObject) JSON.parse("{PreferedInstance:'M'}"));
                DBCollection cl = sdb.getCollectionSpace(csName).getCollection(clName);
                cl.split(srcGroupName, destGroupName, (BSONObject) JSON.parse("{Partition:2048}"), // 切分
                        (BSONObject) JSON.parse("{Partition:4096}"));
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
