package com.sequoiadb.index;

import com.sequoiadb.commlib.GroupMgr;
import com.sequoiadb.commlib.NodeWrapper;
import com.sequoiadb.exception.ReliabilityException;
import com.sequoiadb.fault.KillNode;
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
public class Index4278 extends IndexTestBase {

    /**
     * 测试用例
     * seqDB-4278
     * ::
     * 版本:
     * 2
     * ::
     * 在切分表上，创建/删除索引时，数据库所有节点主机异常重启
     *
     * @throws ReliabilityException
     */
    @Test
    public void test() throws ReliabilityException {
        TaskMgr taskMgr = new TaskMgr();
        IndexTask createTask = IndexTask.getCreateIndexTask(csName, clName, index2Create);
        IndexTask deleteTask = IndexTask.getRemoveIndexTask(csName, clName, indexAlreadlyCreated);
        for (FaultMakeTask faultMakeTask : getGroupKillNodeFaultTask()) {
            taskMgr.addTask(faultMakeTask);
        }

        taskMgr.execute();
        MyUtil.checkBusiness();

        Assert.assertTrue(taskMgr.isAllSuccess(), taskMgr.getErrorMsg());

        Assert.assertTrue(MyUtil.isIndexAllCreated(csName, clName, index2Create));
        Assert.assertTrue(MyUtil.isIndexAllDeleted(csName, clName, indexAlreadlyCreated));
    }

    private List<FaultMakeTask> getGroupKillNodeFaultTask() throws ReliabilityException {
        GroupMgr mgr = new GroupMgr();
        List<NodeWrapper> nodes = mgr.getGroupByName("group1").getNodes();
        nodes.addAll(mgr.getGroupByName("group2").getNodes());
        List<FaultMakeTask> tasks = new ArrayList<>(10);
        for (NodeWrapper node : nodes) {
            tasks.add(KillNode.getFaultMakeTask(node, 5));
        }
        return tasks;
    }
}
