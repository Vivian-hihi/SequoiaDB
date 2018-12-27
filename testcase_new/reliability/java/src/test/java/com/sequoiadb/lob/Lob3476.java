package com.sequoiadb.lob;

import com.sequoiadb.commlib.CommLib;
import com.sequoiadb.commlib.GroupMgr;
import com.sequoiadb.commlib.StandTestInterface;
import com.sequoiadb.exception.ReliabilityException;
import com.sequoiadb.fault.BrokenNetwork;
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
public class Lob3476 implements StandTestInterface {
    String csName = "lob3476cs";
    String clName = "lob3476cl";
    private List<LobBean> lobs2Create = new Vector<>();
    private List<LobBean> lobs2Delete = new Vector<>();


    @BeforeClass
    @Override
    public void setup() {
        MyUtil.printBeginTime(this);
        LobUtil.createLobCsAndCl(csName, clName);
        MyUtil.deleteAllLobs(csName, clName);

        byte[] data = createRandomBytes(1024 * 20);
        for (int i = 0; i < 100; i++) {
            lobs2Delete.add(new LobBean(data));
            lobs2Create.add(new LobBean(data));
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
     * 1.在集合上并发执行以下操作： (1)不停地写lob，覆盖不同lob大小，例如：200k、500M (2)不停地读lob (3)不停地删除lob，并发的同时主节点断网
     * 2.成功选出新主后，继续读写删lob
     * 3.故障恢复后，检查lob数据
     *
     * @throws ReliabilityException
     */
    @Test
    public void test() throws ReliabilityException {

        GroupMgr groupMgr = GroupMgr.getInstance();
        String hostName = groupMgr.getGroupByName("group1").getMaster().hostName();
        FaultMakeTask faultMakeTask = BrokenNetwork.getFaultMakeTask(hostName, 0, 5);
        String safeHosName = CommLib.getSafeCoordUrl(hostName);

        LobTask createTask = LobTask.getCreateLobsTask(lobs2Create)
                .setCsAndClName(csName, clName)
                .setHostName(safeHosName);
        LobTask readTask = LobTask.getReadLobsTask(lobs2Delete)
                .setCsAndClName(csName, clName)
                .setHostName(safeHosName);
        LobTask deleteTask = LobTask.getDeleteLobsTask(lobs2Delete)
                .setCsAndClName(csName, clName)
                .setHostName(safeHosName);

        TaskMgr taskMgr = new TaskMgr(faultMakeTask);
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
}
