package com.sequoiadb.lob;

import com.sequoiadb.commlib.GroupMgr;
import com.sequoiadb.commlib.NodeWrapper;
import com.sequoiadb.commlib.StandTestInterface;
import com.sequoiadb.exception.ReliabilityException;
import com.sequoiadb.fault.NodeRestart;
import com.sequoiadb.metaopr.commons.MyUtil;
import com.sequoiadb.task.FaultMakeTask;
import com.sequoiadb.task.TaskMgr;
import org.bson.types.ObjectId;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Vector;

import static com.sequoiadb.metaopr.commons.MyUtil.*;
import static org.testng.AssertJUnit.assertTrue;

/**
 * @FileName
 * @Author laojingtang
 * @Date 17-5-11
 * @Version 1.00
 */
public class Lob3890 implements StandTestInterface {
    String csName = "lob3890cs";
    String clName = "lob3890cl";
    private List<LobBean> lobs2Create = new Vector<>();
    private List<LobBean> lobs2Delete = new Vector<>();
    TaskMgr taskMgr = new TaskMgr();

    @BeforeClass
    @Override
    public void setup() {
        printBeginTime(this);
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
        MyUtil.printEndTime(this);
        MyUtil.dropCS(csName);
    }

    @Test
    public void test() throws ReliabilityException {
        //所有节点重启
        allNodeRestart();

        List<ObjectId> createdId = new ArrayList<>();
        HashMap<ObjectId, String> deletedIdMap = new HashMap<>();

        LobTask createTask = LobTask.getCreateLobsTask(lobs2Create)
                .setCsAndClName(csName, clName);
        createTask.setName("create lob task");
        LobTask readTask = LobTask.getReadLobsTask(lobs2Delete)
                .setCsAndClName(csName, clName);
        readTask.setName("read lob task ");
        LobTask deleteTask = LobTask.getDeleteLobsTask(lobs2Delete)
                .setCsAndClName(csName, clName);
        deleteTask.setName("delete lob task");

        taskMgr.addTask(createTask);
        taskMgr.addTask(readTask);
        taskMgr.addTask(deleteTask);

        taskMgr.execute();

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

    private void allNodeRestart() throws ReliabilityException {
        GroupMgr groupMgr = new GroupMgr();
        for (NodeWrapper node : groupMgr.getGroupByName("group1").getNodes()) {
            FaultMakeTask faultMakeTask = NodeRestart.getFaultMakeTask(node, 0, 3);
            taskMgr.addTask(faultMakeTask);
        }
        for (NodeWrapper node : groupMgr.getGroupByName("group2").getNodes()) {
            FaultMakeTask faultMakeTask = NodeRestart.getFaultMakeTask(node, 0, 3);
            taskMgr.addTask(faultMakeTask);
        }
        groupMgr.close();
    }
}
