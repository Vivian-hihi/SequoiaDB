package com.sequoiadb.transaction;

import java.util.ArrayList;
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
import com.sequoiadb.commlib.NodeWrapper;
import com.sequoiadb.commlib.SdbTestBase;
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.exception.ReliabilityException;
import com.sequoiadb.fault.NodeRestart;
import com.sequoiadb.task.FaultMakeTask;
import com.sequoiadb.task.OperateTask;
import com.sequoiadb.task.TaskMgr;

/**
 * @Description seqDB-18527: 主子表，转账的过程中正常重启所有数据节点主节点
 * @author yinzhen
 * @date 2019-6-19
 *
 */
@Test(groups = "rcauto")
public class Transaction18527 extends SdbTestBase {
    private Sequoiadb sdb;
    private DBCollection cl;
    private String clName = "cl18527";
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

        // 创建主子表，主表下挂载多个子表，切分键为账户字段，replSize设置为-1
        cl = sdb.getCollectionSpace(csName).createCollection(clName, (BSONObject) JSON
                .parse("{'ShardingKey':{'account':1}, 'ShardingType':'range', 'IsMainCL':true, 'ReplSize':-1}"));
        sdb.getCollectionSpace(csName).createCollection("sub118527");
        sdb.getCollectionSpace(csName).createCollection("sub218527", (BSONObject) JSON
                .parse("{'ShardingKey':{'account':1}, 'ShardingType':'hash', 'AutoSplit':true, 'ReplSize':-1}"));
        cl.attachCollection(csName + ".sub118527",
                (BSONObject) JSON.parse("{LowBound:{'account':{'$minKey':1}}, UpBound:{'account':3000}}"));
        cl.attachCollection(csName + ".sub218527",
                (BSONObject) JSON.parse("{LowBound:{'account':3000}, UpBound:{'account':{'$maxKey':1}}}"));
        insertData();
    }

    @AfterClass
    public void tearDown() {
        CollectionSpace cs = sdb.getCollectionSpace(csName);
        if (cs.isCollectionExist(clName)) {
            cs.dropCollection(clName);
        }
        if (sdb != null) {
            sdb.close();
        }
    }

    @Test
    public void test() throws ReliabilityException {
        // 正常重启所有数据节点的主节点
        TaskMgr taskMgr = new TaskMgr();
        for (String groupName : groupNames) {
            GroupWrapper group = groupMgr.getGroupByName(groupName);
            NodeWrapper node = group.getMaster();
            FaultMakeTask task = NodeRestart.getFaultMakeTask(node, 600, 10);
            taskMgr.addTask(task);
        }

        for (int i = 0; i < 200; i++) {
            taskMgr.addTask(new Transfer());
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
        @Override
        public void exec() throws Exception {
            Sequoiadb db = null;
            try {
                db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
                DBCollection cl = db.getCollectionSpace(csName).getCollection(clName);
                int count = 0;

                // 模拟转账操作：开启事务，随机取一个账户转出value；随机取另一个账户转入value
                while (count++ < 600) {
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
            } finally {
                if (db != null) {
                    db.commit();
                    db.close();
                }
            }
        }
    }

    private void insertData() {
        List<BSONObject> reocrds = new ArrayList<>();
        for (int i = 0; i < 10000; i++) {
            reocrds.add((BSONObject) JSON.parse("{'balance':10000, 'account':" + i + "}"));
        }
        cl.insert(reocrds);
    }
}
