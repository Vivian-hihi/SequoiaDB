package com.sequoiadb.lob;

import com.sequoiadb.commlib.GroupMgr;
import com.sequoiadb.commlib.NodeWrapper;
import com.sequoiadb.commlib.StandTestInterface;
import com.sequoiadb.exception.ReliabilityException;
import com.sequoiadb.fault.NodeRestart;
import com.sequoiadb.metaopr.commons.MyUtil;
import com.sequoiadb.task.FaultMakeTask;
import com.sequoiadb.task.TaskMgr;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import java.util.ArrayList;
import java.util.List;
import java.util.Vector;

import static com.sequoiadb.metaopr.commons.MyUtil.createLobs;
import static com.sequoiadb.metaopr.commons.MyUtil.createRandomBytes;
import static org.testng.AssertJUnit.assertTrue;

/**
 * @FileName
 * @Author laojingtang
 * @Date 17-5-11
 * @Version 1.00
 */
public class Lob11434 implements StandTestInterface {
    String csName = "lob11434cs";
    String clName = "lob11434cl";

    private List<LobBean> lobs2Create = new Vector<>();
    private List<LobBean> lobs2Delete = new Vector<>();

    @BeforeClass
    @Override
    public void setup() {
        MyUtil.printBeginTime(this);
        LobUtil.createLobCsAndCl(csName, clName);
        MyUtil.deleteAllLobs(csName, clName);

        for (int i = 0; i < 100; i++) {
            lobs2Delete.add(new LobBean(createRandomBytes(1024 * 200)));
            lobs2Create.add(new LobBean(createRandomBytes(1024 * 200)));
        }
        createLobs(csName, clName, lobs2Delete);
    }

    @AfterClass
    @Override
    public void tearDown() {
        MyUtil.dropCS(csName);
        MyUtil.printEndTime(this);
    }

    /**
     * 1.在集合上并发执行以下操作：
     * (1)循环增删改查数据、循环读写Lob，其中lob大小取不同值，如200K/500k
     * (2)循环读写删lob、循环增删改查数据操作
     * 2.并发执行步骤1时，数据组主节点正常重启
     * 3.故障恢复后，再次读写lob操作
     *
     * @throws ReliabilityException
     */
    @Test
    public void test() throws ReliabilityException {
        GroupMgr groupMgr = new GroupMgr();
        NodeWrapper master = groupMgr.getGroupByName("group1").getMaster();
        FaultMakeTask faultMakeTask = NodeRestart.getFaultMakeTask(master, 0, 3);

        LobTask createTask = LobTask.getCreateLobsTask(lobs2Create)
                .setCsAndClName(csName, clName);
        createTask.setName("create lob task");
        LobTask readTask = LobTask.getReadLobsTask(lobs2Delete)
                .setCsAndClName(csName, clName);
        readTask.setName("read lob task");
        LobTask deleteTask = LobTask.getDeleteLobsTask(lobs2Delete)
                .setCsAndClName(csName, clName);
        deleteTask.setName("delete lob task");

        TaskMgr taskMgr = new TaskMgr(faultMakeTask);
        taskMgr.addTask(createTask);
        taskMgr.addTask(readTask);
        taskMgr.addTask(deleteTask);
        taskMgr.execute();

        groupMgr.checkBusiness();
        assertTrue(MyUtil.isLobsAllCreated(csName, clName, lobs2Create));
        assertTrue(MyUtil.isLobsAllDelete(csName, clName, lobs2Delete));
        assertTrue(MyUtil.isLobNumInspectInGroup(csName, clName, "group1"));
        assertTrue(MyUtil.isLobNumInspectInGroup(csName, clName, "group2"));
        List<LobBean> lobs = new ArrayList<>();
        lobs.addAll(lobs2Create);
        lobs.addAll(lobs2Delete);
        assertTrue(MyUtil.isLobMd5InspectInGroup(csName, clName, "group1", lobs));
        assertTrue(MyUtil.isLobMd5InspectInGroup(csName, clName, "group2", lobs));
    }


}
