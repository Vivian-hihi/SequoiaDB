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
import com.sequoiadb.commlib.CommLib;
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
 * @FileName: seqDB-2581:对range分区组进行范围切分，切分时目标组主节点断网
 * @author huangqiaohui
 * @version 1.00
 */

public class NetSplit2581 extends SdbTestBase {
    private String clName = "testcaseCL2581";
    private String srcGroupName;
    private String destGroupName;
    private GroupMgr groupMgr = null;
    private String connectUrl;
    private boolean clearFlag = false;
    private String brokenNetHost;
    private boolean isSplitSuccess = false;
    private int initRecsNum = 10000;
    private int destRecsStart = 5000;
    private int destRecsEnd = 35000;
    private int totalRecsNum = destRecsEnd + 1000;
    private int faultSuccRecsNum = 0;

    @BeforeClass()
    public void setUp() {
        Sequoiadb commSdb = null;
        try {
            System.out.println("the TestCase Name:" + this.getClass().getName() + ". the TestCase begin at:"
                    + new SimpleDateFormat("YYYY-MM-dd HH:mm:ss.SSS").format(new Date()));
            groupMgr = GroupMgr.getInstance();

            if (!groupMgr.checkBusiness(20)) {
                throw new SkipException("checkBusiness return false");
            }
            commSdb = new Sequoiadb(coordUrl, "", "");
            List<GroupWrapper> glist = groupMgr.getAllDataGroup();

            srcGroupName = glist.get(0).getGroupName();
            destGroupName = glist.get(1).getGroupName();
            System.out.println("split srcRG:" + srcGroupName + " destRG:" + destGroupName);

            CollectionSpace commCS = commSdb.getCollectionSpace(csName);
            DBCollection cl = commCS.createCollection(clName, (BSONObject) JSON
                    .parse("{ShardingKey:{'sk':1},ShardingType:'range',Group:'" + srcGroupName + "'}"));

            insertData(cl, 0, initRecsNum);// 写入待切分的记录

            // 调整主机
            brokenNetHost = groupMgr.getGroupByName(destGroupName).getMaster().hostName();
            Utils.reelect(brokenNetHost, Utils.CATA_RG_NAME, srcGroupName);
            connectUrl = CommLib.getSafeCoordUrl(brokenNetHost);
            groupMgr.refresh();
            System.out.println("brokenHost:" + brokenNetHost + " connectUrl:" + connectUrl);
        } catch (ReliabilityException e) {
            Assert.fail(this.getClass().getName() + " setUp error, error description:" + e.getMessage() + "\r\n"
                    + Utils.getStackString(e));
        } finally {
            if (commSdb != null) {
                commSdb.close();
            }
        }
    }

    public void insertData(DBCollection cl, int begin, int end) {
        for (int i = begin; i < end; i++) {
            BSONObject obj = (BSONObject) JSON.parse("{sk:" + i + "}");
            cl.insert(obj);
            if (i < destRecsEnd) {
                faultSuccRecsNum = i;
            }
        }
    }

    @Test()
    public void test() {
        Sequoiadb db = null;
        try {
            // 建立并行任务
            FaultMakeTask faultTask = BrokenNetwork.getFaultMakeTask(brokenNetHost, 2, 10, 15);
            TaskMgr mgr = new TaskMgr(faultTask);
            mgr.addTask(new Split());
            mgr.addTask(new Insert());
            mgr.execute();

            Assert.assertEquals(mgr.isAllSuccess(), true, mgr.getErrorMsg());

            // 最长等待2分钟的环境恢复
            Assert.assertEquals(groupMgr.checkBusiness(600), true, "failed to restore business");
            System.out.println("faultSuccRecsNum: " + faultSuccRecsNum + ", destRecsEnd: " + destRecsEnd);

            db = new Sequoiadb(connectUrl, "", "");
            db.setSessionAttr((BSONObject) JSON.parse("{PreferedInstance:'M'}"));
            DBCollection cl = db.getCollectionSpace(csName).getCollection(clName);
            insertData(cl, faultSuccRecsNum + 1, totalRecsNum);

            if (isSplitSuccess) {
                Utils.waitSplit(db, cl.getFullName());
                checkGroupData(db, destGroupName, "{sk:{$gte:" + destRecsStart + ",$lt:" + destRecsEnd + "}}",
                        destRecsEnd - destRecsStart);
                checkGroupData(db, srcGroupName,
                        "{$or:[{sk:{$lt:" + destRecsStart + "}},{sk:{$gte:" + destRecsEnd + "}}]}",
                        totalRecsNum - (destRecsEnd - destRecsStart));
            }
            Assert.assertEquals(cl.getCount(), totalRecsNum);
            clearFlag = true;
        } catch (ReliabilityException e) {
            e.printStackTrace();
            Assert.fail(e.getMessage());
        } finally {
            if (db != null) {
                db.close();
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
            Assert.assertEquals(macherCount, count);
            Assert.assertEquals(count, expectCount);
        } catch (BaseException e) {
            e.printStackTrace();
            Assert.fail(e.getMessage() + "\r\n" + Utils.getStackString(e));
        } finally {
            if (cursor != null) {
                cursor.close();
            }
            if (dataNode != null) {
                dataNode.close();
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
        } catch (BaseException e) {
            Assert.fail(e.getMessage() + "\r\n" + Utils.getStackString(e));
        } finally {
            commSdb.close();
            System.out.println("the TestCase Name:" + this.getClass().getName() + ". the TestCase end at:"
                    + new SimpleDateFormat("YYYY-MM-dd HH:mm:ss.SSS").format(new Date()));
        }
    }

    class Insert extends OperateTask {
        @Override
        public void exec() throws Exception {
            try {
                Sequoiadb db = new Sequoiadb(connectUrl, "", "");
                DBCollection cl = db.getCollectionSpace(csName).getCollection(clName);
                insertData(cl, initRecsNum, destRecsEnd);
                db.close();
            } catch (BaseException e) {
                System.out.println("insert have exception:" + e.getMessage());
            }

        }
    }

    class Split extends OperateTask {
        @Override
        public void exec() throws Exception {
            Sequoiadb sdb = null;
            try {
                sdb = new Sequoiadb(connectUrl, "", "");
                sdb.setSessionAttr((BSONObject) JSON.parse("{PreferedInstance:'M'}"));
                DBCollection cl = sdb.getCollectionSpace(csName).getCollection(clName);
                try {
                    cl.split(srcGroupName, destGroupName, (BSONObject) JSON.parse("{sk:" + destRecsStart + "}"),
                            (BSONObject) JSON.parse("{sk:" + destRecsEnd + "}"));
                    isSplitSuccess = true;
                } catch (BaseException e) {
                    System.out.println("split have exception:" + e.getMessage());
                }
            } catch (BaseException e) {
                throw e;
            } finally {
                if (sdb != null) {
                    sdb.close();
                }
            }
        }
    }

}
