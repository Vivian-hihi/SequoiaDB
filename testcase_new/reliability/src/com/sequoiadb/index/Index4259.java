package com.sequoiadb.index;

import com.sequoiadb.commlib.GroupMgr;
import com.sequoiadb.commlib.NodeWrapper;
import com.sequoiadb.exception.ReliabilityException;
import com.sequoiadb.fault.NodeRestart;
import com.sequoiadb.metaopr.commons.MyUtil;
import com.sequoiadb.task.FaultMakeTask;
import com.sequoiadb.task.TaskMgr;
import org.testng.Assert;
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
        IndexTask createTask = IndexTask.getCreateIndexTask(csName, clName, index2Create);
        IndexTask deleteTask = IndexTask.getRemoveIndexTask(csName, clName, indexAlreadlyCreated);
        for (FaultMakeTask faultMakeTask : getGroupNodeRestartFaultTask()) {
            taskMgr.addTask(faultMakeTask);
        }

        taskMgr.execute();
        MyUtil.checkBusiness();

        Assert.assertTrue(taskMgr.isAllSuccess(), taskMgr.getErrorMsg());

        Assert.assertTrue(MyUtil.isIndexAllCreated(csName, clName, index2Create));
        Assert.assertTrue(MyUtil.isIndexAllDeleted(csName, clName, indexAlreadlyCreated));
    }

    private List<FaultMakeTask> getGroupNodeRestartFaultTask() throws ReliabilityException {
        GroupMgr mgr=new GroupMgr();
        List<NodeWrapper> nodes=mgr.getGroupByName("group1").getNodes();
        nodes.addAll(mgr.getGroupByName("group2").getNodes());
        List<FaultMakeTask> tasks=new ArrayList<>(10);
        for (NodeWrapper node : nodes) {
            tasks.add(NodeRestart.getFaultMakeTask(node,0,5));
        }
        return tasks;
    }
}
