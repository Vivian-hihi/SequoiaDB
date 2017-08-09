package com.sequoiadb.lob;

import com.sequoiadb.commlib.GroupMgr;
import com.sequoiadb.commlib.NodeWrapper;
import com.sequoiadb.commlib.StandTestInterface;
import com.sequoiadb.exception.ReliabilityException;
import com.sequoiadb.fault.NodeRestart;
import com.sequoiadb.task.FaultMakeTask;
import com.sequoiadb.task.TaskMgr;
import org.bson.types.ObjectId;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import java.util.*;

import static com.sequoiadb.metaopr.commons.MyUtil.*;
import static org.testng.Assert.assertTrue;

/**
 * @FileName 测试用例 seqDB-3885 :: 版本: 2 :: 切分表写lob时备节点正常重启
 * @Author laojingtang
 * @Date 17-5-11
 * @Version 1.00
 */
public class Lob3885 implements StandTestInterface {
    private String csName = "lob3885cs";
    private String clName = "lob3885cl";

    private List<LobBean> lobs2Create = new Vector<>();
    private List<LobBean> lobs2Delete = new Vector<>();

    /**
     * 1.在集合上并发执行以下操作：
     * (1)不停写Lob，覆盖不同lob大小，例如：200k、500M ;
     * (2)不停地删除，并发操作时，备节点正常重启
     * 2.故障恢复后，再次进行写/删除lob
     *
     * @throws ReliabilityException
     */
    @Test
    public void test() throws ReliabilityException {
        //备节点重启
        GroupMgr groupMgr = new GroupMgr();
        NodeWrapper node = groupMgr.getGroupByName("group1").getSlave();
        FaultMakeTask faultMakeTask = NodeRestart.getFaultMakeTask(node, 0, 5);

        //并发读写lob
        List<ObjectId> createIds = new ArrayList<>(50);
        Map<ObjectId, String> deteledIdMap = new HashMap<>();

        LobTask createLobsTask = LobTask.getCreateLobsTask(lobs2Create)
                .setCsAndClName(csName, clName);
        createLobsTask.setName("create lobs task");
        LobTask deleteLobsTask = LobTask.getDeleteLobsTask(lobs2Delete)
                .setCsAndClName(csName, clName);
        deleteLobsTask.setName("delete lobs task ");

        TaskMgr mgr = new TaskMgr(faultMakeTask);
        mgr.addTask(createLobsTask);
        mgr.addTask(deleteLobsTask);
        mgr.execute();

        checkBusiness();
        assertTrue(isLobsAllCreated(csName, clName, lobs2Create));
        assertTrue(isLobsAllDelete(csName, clName, lobs2Delete));
        assertTrue(isLobNumInspectInGroup(csName, clName, "group1"));
        assertTrue(isLobNumInspectInGroup(csName, clName, "group2"));
        List<LobBean> lobs = new ArrayList<>();
        lobs.addAll(lobs2Create);
        lobs.addAll(lobs2Delete);
        assertTrue(isLobMd5InspectInGroup(csName, clName, "group1", lobs));
        assertTrue(isLobMd5InspectInGroup(csName, clName, "group2", lobs));
    }

    @BeforeClass
    @Override
    public void setup() {
        printBeginTime(this);
        LobUtil.createLobCsAndCl(csName, clName);
        deleteAllLobs(csName, clName);

        for (int i = 0; i < 100; i++) {
            lobs2Delete.add(new LobBean(createRandomBytes(1024 * 200)));
            lobs2Create.add(new LobBean(createRandomBytes(1024 * 200)));
        }
        createLobs(csName, clName, lobs2Delete);
    }


    @AfterClass
    @Override
    public void tearDown() {
        deleteAllLobs(csName, clName);
        dropCS(csName);
        printEndTime(this);
    }
}
