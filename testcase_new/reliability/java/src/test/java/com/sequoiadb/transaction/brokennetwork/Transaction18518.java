package com.sequoiadb.transaction.brokennetwork;

import java.util.ArrayList;
import java.util.List;

import org.bson.BSONObject;
import org.bson.util.JSON;
import org.testng.Assert;
import org.testng.SkipException;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.DataProvider;
import org.testng.annotations.Test;

import com.sequoiadb.base.CollectionSpace;
import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.DBCursor;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.commlib.CommLib;
import com.sequoiadb.commlib.GroupMgr;
import com.sequoiadb.commlib.GroupWrapper;
import com.sequoiadb.commlib.NodeWrapper;
import com.sequoiadb.commlib.SdbTestBase;
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.exception.ReliabilityException;
import com.sequoiadb.fault.BrokenNetwork;
import com.sequoiadb.task.FaultMakeTask;
import com.sequoiadb.task.OperateTask;
import com.sequoiadb.task.TaskMgr;

/**
 * @Description seqDB-18518: hash分区表/主子表，转账的过程中所有数据节点主节点断网
 * @author yinzhen
 * @date 2019-6-19
 *
 */
@Test(groups = "rcauto")
public class Transaction18518 extends SdbTestBase {
    private Sequoiadb sdb;
    private String clName = "cl18518";
    private GroupMgr groupMgr;
    private List<String> groupNames;

    @BeforeClass
    public void setUp() throws ReliabilityException {
        sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "");
        if (CommLib.isStandAlone(sdb)) {
            throw new SkipException("STANDALONE MODE");
        }
        groupNames = CommLib.getDataGroupNames(sdb);
        if (groupNames.size() < 2) {
            throw new SkipException("ONE GROUP MODE");
        }
        groupMgr = GroupMgr.getInstance();
        if (!groupMgr.checkBusiness(120)) {
            throw new SkipException("GROUP ERROR");
        }

        // 创建hash分区表/主子表(主表下挂载多个子表，子表覆盖分区表)，replSize设置为-1，且已切分到所有组上，切分键为账户字段
        DBCollection hashCL = sdb.getCollectionSpace(csName).createCollection(clName + "hash", (BSONObject) JSON
                .parse("{'ShardingKey':{'account':1}, 'ShardingType':'hash', 'AutoSplit':true, 'ReplSize':-1}"));

        DBCollection mainCL = sdb.getCollectionSpace(csName).createCollection(clName + "mainCL", (BSONObject) JSON
                .parse("{'ShardingKey':{'account':1}, 'ShardingType':'range', 'IsMainCL':true, 'ReplSize':-1}"));
        sdb.getCollectionSpace(csName).createCollection("sub118518");
        sdb.getCollectionSpace(csName).createCollection("sub218518", (BSONObject) JSON
                .parse("{'ShardingKey':{'account':1}, 'ShardingType':'hash', 'AutoSplit':true, 'ReplSize':-1}"));
        mainCL.attachCollection(csName + ".sub118518",
                (BSONObject) JSON.parse("{LowBound:{'account':{'$minKey':1}}, UpBound:{'account':3000}}"));
        mainCL.attachCollection(csName + ".sub218518",
                (BSONObject) JSON.parse("{LowBound:{'account':3000}, UpBound:{'account':{'$maxKey':1}}}"));
        insertData(hashCL);
        insertData(mainCL);
    }

    @AfterClass
    public void tearDown() {
        CollectionSpace cs = sdb.getCollectionSpace(csName);
        if (cs.isCollectionExist(clName + "hash")) {
            cs.dropCollection(clName + "hash");
        }
        if (cs.isCollectionExist(clName + "mainCL")) {
            cs.dropCollection(clName + "mainCL");
        }
        if (sdb != null) {
            sdb.close();
        }
    }

    @DataProvider(name = "getCL")
    private Object[][] getCLName() {
        return new Object[][] { { clName + "hash" }, { clName + "mainCL" } };
    }

    @Test(dataProvider = "getCL")
    public void test(String clName) throws ReliabilityException {
        // 所有数据节点的主节点断网
        TaskMgr taskMgr = new TaskMgr();
        for (String groupName : groupNames) {
            GroupWrapper group = groupMgr.getGroupByName(groupName);
            NodeWrapper node = group.getMaster();
            FaultMakeTask task = BrokenNetwork.getFaultMakeTask(node.hostName(), 600, 10);
            taskMgr.addTask(task);
        }

        for (int i = 0; i < 200; i++) {
            taskMgr.addTask(new Transfer(clName));
        }
        taskMgr.execute();

        Assert.assertTrue(taskMgr.isAllSuccess(), taskMgr.getErrorMsg());
        Assert.assertTrue(groupMgr.checkBusinessWithLSN(120), "GROUP ERROR");

        // 待集群正常后，查询所有账户的金额总和
        DBCursor cursor = sdb.exec("select sum(balance) as balance from " + csName + "." + clName);
        double balance = (double) cursor.getNext().get("balance");
        Assert.assertEquals((int) balance, 100000000);
    }

    private class Transfer extends OperateTask {
        private String clName;

        private Transfer(String clName) {
            this.clName = clName;
        }

        @Override
        public void exec() throws Exception {
            Sequoiadb db = null;
            try {
                db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
                DBCollection cl = db.getCollectionSpace(csName).getCollection(clName);
                int count = 0;

                // 模拟转账操作：开启事务，随机取一个账户转出value；随机取另一个账户转入value
                while (count++ < 6000) {
                    int accountA = (int) (Math.random() * 10000);
                    int accountB = (int) (Math.random() * 10000);
                    int transAmount = (int) (Math.random() * 200);

                    db.beginTransaction();
                    cl.update("{'account':" + accountA + "}", "{$inc:{'balance':" + (-transAmount) + "}}",
                            "{'':'$shard'}");
                    cl.update("{'account':" + accountB + "}", "{$inc:{'balance':" + transAmount + "}}",
                            "{'':'$shard'}");
                    db.commit();
                    Thread.sleep(200);
                }
            } catch (BaseException e) {
                db.rollback();
            } finally {
                if (db != null) {
                    db.commit();
                    db.close();
                }
            }
        }
    }

    private void insertData(DBCollection cl) {
        List<BSONObject> reocrds = new ArrayList<>();
        for (int i = 0; i < 10000; i++) {
            reocrds.add((BSONObject) JSON.parse("{'balance':10000, 'account':" + i + "}"));
        }
        cl.insert(reocrds);
    }
}
