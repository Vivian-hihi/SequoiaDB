package com.sequoiadb.transaction.brokennetwork;

import java.util.List;

import org.testng.Assert;
import org.testng.SkipException;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.DataProvider;
import org.testng.annotations.Test;

import com.sequoiadb.base.CollectionSpace;
import com.sequoiadb.base.DBCursor;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.commlib.CommLib;
import com.sequoiadb.commlib.GroupMgr;
import com.sequoiadb.commlib.GroupWrapper;
import com.sequoiadb.commlib.NodeWrapper;
import com.sequoiadb.commlib.SdbTestBase;
import com.sequoiadb.exception.ReliabilityException;
import com.sequoiadb.fault.BrokenNetwork;
import com.sequoiadb.task.FaultMakeTask;
import com.sequoiadb.task.TaskMgr;
import com.sequoiadb.transaction.common.TransUtil;
import com.sequoiadb.transaction.common.TransferTh;

/**
 * @Description seqDB-18519: hash分区表/主子表，转账的过程中部分数据节点主节点断网
 * @author yinzhen
 * @date 2019-6-19
 *
 */
@Test(groups = "rcauto")
public class Transaction18519 extends SdbTestBase {
    private Sequoiadb sdb;
    private String hashCLName = "cl18519_hash";
    private String mainCLName = "cl18519_main";
    private String subCLName1 = "subcl18519_1";
    private String subCLName2 = "subcl18519_2";
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

        // 创建hash分区表/主子表(主表下挂载多个子表，子表覆盖分区表)，replSize设置为1，且已切分到所有组上，切分键为账户字段
        // 并插入数据 10000 个账户，每个账户 10000 元
        TransUtil.createCLsAndInsertData(sdb, csName, hashCLName, mainCLName, subCLName1, subCLName2);
    }

    @AfterClass
    public void tearDown() {
        try {
            CollectionSpace cs = sdb.getCollectionSpace(csName);
            cs.dropCollection(hashCLName);
            cs.dropCollection(mainCLName);
        } finally {
            if (sdb != null) {
                sdb.close();
            }
        }
    }

    @DataProvider(name = "getCL")
    private Object[][] getCLName() {
        return new Object[][] { { hashCLName }, { mainCLName } };
    }

    @Test(dataProvider = "getCL")
    public void test(String clName) throws ReliabilityException {
        // 部分数据节点的主节点断网
        TaskMgr taskMgr = new TaskMgr();
        for (int i = 0; i < groupNames.size() - 1; i++) {
            String groupName = groupNames.get(i);
            GroupWrapper group = groupMgr.getGroupByName(groupName);
            NodeWrapper node = group.getMaster();
            FaultMakeTask task = BrokenNetwork.getFaultMakeTask(node.hostName(), 180, 10);
            taskMgr.addTask(task);
        }

        for (int i = 0; i < 200; i++) {
            taskMgr.addTask(new TransferTh(csName, clName));
        }
        taskMgr.execute();

        Assert.assertTrue(taskMgr.isAllSuccess(), taskMgr.getErrorMsg());
        Assert.assertTrue(groupMgr.checkBusinessWithLSN(120), "GROUP ERROR");

        // 待集群正常后，查询所有账户的金额总和
        DBCursor cursor = sdb.exec("select sum(balance) as balance from " + csName + "." + clName);
        double balance = (double) cursor.getNext().get("balance");
        Assert.assertEquals((int) balance, 100000000);
    }
}
