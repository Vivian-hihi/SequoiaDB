package com.sequoiadb.netsplit;

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
import com.sequoiadb.base.DBLob;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.commlib.GroupMgr;
import com.sequoiadb.commlib.GroupWrapper;
import com.sequoiadb.commlib.SdbTestBase;
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.exception.ReliabilityException;
import com.sequoiadb.fault.BrokenNetwork;
import com.sequoiadb.task.FaultMakeTask;
import com.sequoiadb.task.OperateTask;
import com.sequoiadb.task.TaskMgr;

/**
 * @FileName:SEQDB-2569 对hash分区组进行范围切分，切分时目标组主节点断网,切分数据类型为lob对象
 * @author huangqiaohui
 * @version 1.00
 *
 */

public class NetSplit2569 extends SdbTestBase {
    private String clName = "testcaseCL2569";
    private String srcGroupName;
    private String destGroupName;
    private Sequoiadb commSdb = null;
    private GroupMgr groupMgr = null;
    private int totalCount;
    private String connectUrl;

    @BeforeClass()
    public void setUp() {

        try {
            System.out.println(
                    "the TestCase Name:" + this.getClass().getName() + ". the TestCase begin at:"
                            + new SimpleDateFormat("YYYY-MM-dd HH:mm:ss.SSS").format(new Date()));
            commSdb = new Sequoiadb(coordUrl, "", "");
            groupMgr = GroupMgr.getInstance();

            if (!groupMgr.checkBusiness(true)) {
                throw new SkipException("checkBusiness faile");
            }
            List<GroupWrapper> glist = groupMgr.getAllDataGroup();

            srcGroupName = glist.get(0).getGroupName();
            destGroupName = glist.get(1).getGroupName();

            CollectionSpace commCS = commSdb.getCollectionSpace(csName);
            DBCollection cl = commCS.createCollection(clName,
                    (BSONObject) JSON
                            .parse("{ShardingKey:{'sk':1},Partition:4096,ShardingType:'hash',Group:'"
                                    + srcGroupName + "'}"));
            insertData(cl, 0, 1000);// 写入待切分的记录（1000普通记录，1000lob）
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
            DBLob lob = cl.createLob();
            String id = lob.getID().toString();
            lob.write(id.getBytes());
            lob.close();
        }
        totalCount = totalCount + (end - begin);
    }

    @Test
    public void test() {
        Sequoiadb db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
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
            db = new Sequoiadb(connectUrl, "", "");
            // 建立并行任务
            FaultMakeTask faultTask = BrokenNetwork.getFaultMakeTask(destPriHost, 2, 10, 10);
            TaskMgr mgr = new TaskMgr(faultTask);
            mgr.addTask(new Split("Split"));
            mgr.addTask(new Insert("insert"));
            mgr.init();
            mgr.start();
            mgr.join();
            mgr.fini();

            // 最长等待20分钟的环境恢复
            Assert.assertEquals(Utils.checkBusinessWithTimeout(groupMgr, 1200), true,
                    "wait restore business faile");

            // 结果校验
            if (!destGroup.checkInspect(60, 1)) {
                Assert.fail(destGroup.getInspectStdout());
            }
            if (!srcGroup.checkInspect(60, 1)) {
                Assert.fail(srcGroup.getInspectStdout());
            }
            Assert.assertEquals(mgr.isAllSuccess(), true, mgr.getErrorMsg());
            checkGroupLob(db, destGroupName);
            checkGroupLob(db, srcGroupName);

        }
        catch (ReliabilityException e) {
            e.printStackTrace();
            Assert.fail(e.getMessage());
        }

    }

    private void checkGroupLob(Sequoiadb sdb, String destGroupName) {
        Sequoiadb destDataNode = null;
        DBCursor cursor = null;
        try {
            destDataNode = sdb.getReplicaGroup(destGroupName).getMaster().connect();// 获得源主节点链接
            DBCollection destCL = destDataNode.getCollectionSpace(csName).getCollection(clName);

            cursor = destCL.listLobs();
            int lobCount = 0;
            while (cursor.hasNext()) {
                cursor.getNext();
                lobCount++;
            }
            // 数据量应在totalCount / 2条左右（切分范围2048-4096）
            Assert.assertEquals(
                    lobCount > totalCount / 2 - (totalCount / 2 * 0.3)
                            && lobCount < totalCount / 2 + (totalCount / 2 * 0.3),
                    true, "srcGroup count:" + lobCount);
        }
        catch (BaseException e) {
            Assert.fail(e.getMessage() + "\r\n" + Utils.getStackString(e));
        }
        finally {
            if (cursor != null) {
                cursor.close();
            }
            if (destDataNode != null) {
                destDataNode.disconnect();
            }
        }
    }

    @AfterClass
    public void tearDown() {
        try {
            CollectionSpace commCS = commSdb.getCollectionSpace(csName);
            commCS.dropCollection(clName);
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

    class Insert extends OperateTask {

        public Insert(String name) {
            super(name);
            // TODO Auto-generated constructor stub
        }

        @Override
        public void exec() throws Exception {
            Sequoiadb db = new Sequoiadb(connectUrl, "", "");
            DBCollection cl = db.getCollectionSpace(csName).getCollection(clName);
            insertData(cl, 1000, 3000);
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
                // insertData(cl, 3000, 4000);
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
