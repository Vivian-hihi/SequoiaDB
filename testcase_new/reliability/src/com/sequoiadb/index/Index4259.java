package com.sequoiadb.index;

import com.sequoiadb.commlib.GroupMgr;
import com.sequoiadb.commlib.NodeWrapper;
import com.sequoiadb.exception.ReliabilityException;
import com.sequoiadb.fault.NodeRestart;
import com.sequoiadb.metaopr.commons.MyUtil;
import com.sequoiadb.task.FaultMakeTask;
import com.sequoiadb.task.TaskMgr;
import org.bson.BSONObject;
import org.bson.util.JSON;
import org.testng.Assert;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import java.util.ArrayList;
import java.util.List;

/**
 * @FileName
 * @Author laojingtang
 * @Date 17-8-8
 * @Version 1.00
 */
public class Index4259 extends IndexTestBase {
    @BeforeClass
    @Override
    public void setup() {
        super.setup();
        BSONObject option = (BSONObject) JSON.parse("{ ShardingKey: { \"age\": 1 }," +
                " ShardingType: \"hash\", " +
                "Partition: 1024, ReplSize: 1," +
                " Compressed: true ," +
                "Group:\"group1\"}");
//        createIndexCl(option);
//        insertData();
//        split50();
        createIndexes(indexAlreadlyCreated);
    }

    /**
     * 测试用例
     * seqDB-4259
     * ::
     * 版本:
     * 1
     * ::
     * 在切分表上，创建/删除索引时，数据组所有节点主机正常重启_rlb.hostNormalRestart.basicOpe.012
     */
    @Test
    public void test() throws ReliabilityException {
        TaskMgr taskMgr = new TaskMgr();
        IndexTask createTask = super.getCreateTask();
        IndexTask deleteTask = super.getDeleteTask();

//        for (FaultMakeTask faultMakeTask : getGroupNodeRestartFaultTask()) {
//            taskMgr.addTask(faultMakeTask);
//        }

        taskMgr.addTask(createTask);
        taskMgr.addTask(deleteTask);
        taskMgr.execute();
        MyUtil.checkBusiness();

        Assert.assertTrue(taskMgr.isAllSuccess(), taskMgr.getErrorMsg());

        Assert.assertTrue(isIndexesAllCreatedInNodes());
        Assert.assertTrue(isIndexesAllDeletedInNodes());
    }

    private List<FaultMakeTask> getGroupNodeRestartFaultTask() throws ReliabilityException {
        GroupMgr mgr = new GroupMgr();
        List<NodeWrapper> nodes = mgr.getGroupByName("group1").getNodes();
        nodes.addAll(mgr.getGroupByName("group2").getNodes());
        List<FaultMakeTask> tasks = new ArrayList<>(10);
        for (NodeWrapper node : nodes) {
            tasks.add(NodeRestart.getFaultMakeTask(node, 0, 5));
        }
        return tasks;
    }
}
