package com.sequoiadb.transaction.restartnode;

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
import com.sequoiadb.fault.NodeRestart;
import com.sequoiadb.task.FaultMakeTask;
import com.sequoiadb.task.OperateTask;
import com.sequoiadb.task.TaskMgr;

/**
 * @Description seqDB-18521: hash分区表/主子表，转账的过程中正常重启所有数据节点主节点及转账程序执行的coord节点
 * @author yinzhen
 * @date 2019-6-19
 *
 */
public class Transaction18521 extends SdbTestBase {
    private Sequoiadb sdb;
    private Sequoiadb gmrDB;
    private String clName = "cl18521";
    private String coordUrl;
    private GroupMgr groupMgr;
    private List<String> groupNames;

    @BeforeClass
    public void setUp() throws ReliabilityException {
        gmrDB = new Sequoiadb(SdbTestBase.coordUrl, "", "");
        getCoordConn();
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
        sdb.getCollectionSpace(csName).createCollection("sub118521");
        sdb.getCollectionSpace(csName).createCollection("sub218521", (BSONObject) JSON
                .parse("{'ShardingKey':{'account':1}, 'ShardingType':'hash', 'AutoSplit':true, 'ReplSize':-1}"));
        mainCL.attachCollection(csName + ".sub118521",
                (BSONObject) JSON.parse("{LowBound:{'account':{'$minKey':1}}, UpBound:{'account':3000}}"));
        mainCL.attachCollection(csName + ".sub218521",
                (BSONObject) JSON.parse("{LowBound:{'account':3000}, UpBound:{'account':{'$maxKey':1}}}"));
        insertData(hashCL);
        insertData(mainCL);
    }

    @AfterClass
    public void tearDown() {
        // TODO :这里应该要使用gmrDB去清理环境，因为coord节点被你重启，连接不在了，另外释放连接放到finally里面
        CollectionSpace cs = gmrDB.getCollectionSpace(csName);
        if (cs.isCollectionExist(clName + "hash")) {
            cs.dropCollection(clName + "hash");
        }
        if (cs.isCollectionExist(clName + "mainCL")) {
            cs.dropCollection(clName + "mainCL");
        }
        if (gmrDB != null) {
            gmrDB.close();
        }
        if (gmrDB != null) {
            gmrDB.close();
        }
    }

    @DataProvider(name = "getCL")
    private Object[][] getCLName() {
        return new Object[][] { { clName + "hash" }, { clName + "mainCL" } };
    }

    @Test(dataProvider = "getCL")
    public void test(String clName) throws ReliabilityException {
        // 正常重启所有数据节点的主节点及转账程序连接的coord节点
        TaskMgr taskMgr = new TaskMgr();
        for (String groupName : groupNames) {
            GroupWrapper group = groupMgr.getGroupByName(groupName);
            NodeWrapper node = group.getMaster();
            FaultMakeTask task = NodeRestart.getFaultMakeTask(node, 600, 10, 20);
            taskMgr.addTask(task);
            System.out.println("RESTART: " + node.hostName() + ":" + node.svcName());
        }
        NodeWrapper coordNode = getCoordNode();
        FaultMakeTask task = NodeRestart.getFaultMakeTask(coordNode, 600, 10, 20);
        taskMgr.addTask(task);
        System.out.println("RESTART: " + coordNode.hostName() + ":" + coordNode.svcName());

        for (int i = 0; i < 200; i++) {
            taskMgr.addTask(new Transfer(clName));
        }
        taskMgr.execute();

        Assert.assertTrue(taskMgr.isAllSuccess(), taskMgr.getErrorMsg());
        Assert.assertTrue(groupMgr.checkBusinessWithLSN(120), "GROUP ERROR");

        // 待集群正常后，查询所有账户的金额总和
        sdb = new Sequoiadb(coordUrl, "", "");
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
                db = new Sequoiadb(coordUrl, "", "");
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

    private void getCoordConn() {
        List<String> nodeAddress = CommLib.getNodeAddress(gmrDB, "SYSCoord");
        for (String nodeAddr : nodeAddress) {
            String hostName = nodeAddr.split(":")[0];
            String svcName = nodeAddr.split(":")[1];
            if (!hostName.equals(gmrDB.getHost())) {
                coordUrl = hostName + ":" + svcName;
                sdb = new Sequoiadb(coordUrl, "", "");
                break;
            }
        }
        System.out.println("getCoordConn:sdb:" + sdb);
    }

    private NodeWrapper getCoordNode() {
        GroupWrapper group = groupMgr.getGroupByName("SYSCoord");
        List<NodeWrapper> nodes = group.getNodes();
        String hostName = sdb.getHost();
        String svcName = String.valueOf(sdb.getPort());
        for (NodeWrapper node : nodes) {
            if (hostName.equals(node.hostName()) && svcName.equals(node.svcName())) {
                return node;
            }
        }
        return null;
    }
}
