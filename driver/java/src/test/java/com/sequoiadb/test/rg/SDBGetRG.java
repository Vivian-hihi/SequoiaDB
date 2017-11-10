package com.sequoiadb.test.rg;

import com.sequoiadb.base.Node;
import com.sequoiadb.base.ReplicaGroup;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.exception.SDBError;
import com.sequoiadb.test.common.Constants;
import org.bson.BSONObject;
import org.bson.types.BasicBSONList;
import org.junit.*;

import java.util.Random;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotEquals;
import static org.junit.Assert.assertTrue;

public class SDBGetRG {
    private static Sequoiadb sdb;
    private static ReplicaGroup rg;
    private static Node node;
    private static int groupID;
    private static String groupName;
    private static String Name;
    private static boolean isCluster = true;

    @BeforeClass
    public static void setConnBeforeClass() throws Exception {
        // sdb
        isCluster = Constants.isCluster();
        if (!isCluster)
            return;
        sdb = new Sequoiadb(Constants.COOR_NODE_CONN, "", "");
    }

    @AfterClass
    public static void DropConnAfterClass() throws Exception {
        if (!isCluster)
            return;
        sdb.disconnect();
    }

    @Before
    public void setUp() throws Exception {
        if (!isCluster)
            return;
        rg = sdb.getReplicaGroup(1000);
        Name = rg.getGroupName();
        groupID = Constants.GROUPID;
        groupName = Name;
    }

    @After
    public void tearDown() throws Exception {
        if (!isCluster)
            return;
    }

    @Test
    public void getReplicaGroupById() {
        if (!isCluster)
            return;
        rg = sdb.getReplicaGroup(groupID);
        int id = rg.getId();
        assertEquals(groupID, id);
    }

    @Test
    public void getReplicaGroupByName() {
        if (!isCluster)
            return;
        groupName = Constants.CATALOGRGNAME;
        rg = sdb.getReplicaGroup(groupName);
        String name = rg.getGroupName();
        assertEquals(groupName, name);
    }

    @Test
    public void getCataReplicaGroupById() {
        if (!isCluster)
            return;
        groupID = 1;
        rg = sdb.getReplicaGroup(groupID);
        int id = rg.getId();
        assertEquals(groupID, id);
        boolean f = rg.isCatalog();
        assertTrue(f);
    }

    @Test
    public void getCataReplicaGroupByName() {
        if (!isCluster)
            return;
        groupName = Constants.CATALOGRGNAME;
        rg = sdb.getReplicaGroup(groupName);
        String name = rg.getGroupName();
        assertEquals(groupName, name);
        boolean f = rg.isCatalog();
        assertTrue(f);
    }

    @Test
    public void getCataReplicaGroupByName1() {
        if (!isCluster)
            return;
        try {
            // no replica group can name start wtih "SYS"
            groupName = "SYSCatalogGroupForTest";
            rg = sdb.createReplicaGroup(groupName);
        } catch (BaseException e) {
            assertEquals(SDBError.SDB_INVALIDARG.getErrorCode(),
                e.getErrorCode());
            return;
        }
        assertTrue(false);
    }

    @Test
    public void getMasterAndSlaveNodeTest() {
        if (!isCluster)
            return;
        groupName = "SYSCatalogGroup";
        rg = sdb.getReplicaGroup(groupName);
        BSONObject detail = rg.getDetail();
        BasicBSONList nodeList = (BasicBSONList)detail.get("Group");
        int nodeCount = nodeList.size();
        assertTrue(nodeCount != 0);

        Node master = null;
        Node slave = null;

        // case 1
        master = rg.getMaster();
        slave = rg.getSlave();
        System.out.println(String.format("case1: group is: %s, master is: %s, slave is: %s", groupName,
                master == null ? null : master.getNodeName(),
                slave == null ? null : slave.getNodeName()));
        if (nodeCount == 1) {
            assertEquals(master.getNodeName(), slave.getNodeName());
        } else {
            assertNotEquals(master.getNodeName(), slave.getNodeName());
        }

        // case 2
        slave = rg.getSlave(1,2,3,4,5,6,7);
        System.out.println(String.format("case2: group is: %s, master is: %s, slave is: %s", groupName,
                master == null ? null : master.getNodeName(),
                slave == null ? null : slave.getNodeName()));
        if (nodeCount == 1) {
            assertEquals(master.getNodeName(), slave.getNodeName());
        } else {
            assertNotEquals(master.getNodeName(), slave.getNodeName());
        }

        // case 3
        Random random = new Random();
        int pos1 = random.nextInt(7) + 1;
        int pos2 = 0;
        while(true) {
            pos2 = random.nextInt(7) + 1;
            if (pos2 != pos1) {
                break;
            }
        }
        slave = rg.getSlave(pos1, pos2);
        System.out.println(String.format("case3: group is: %s, master is: %s, slave is: %s", groupName,
                master == null ? null : master.getNodeName(),
                slave == null ? null : slave.getNodeName()));
        if (nodeCount == 1) {
            assertEquals(master.getNodeName(), slave.getNodeName());
        } else {
            assertNotEquals(master.getNodeName(), slave.getNodeName());
        }

        // case 4
        slave= rg.getSlave(null);
        System.out.println(String.format("case4: group is: %s, master is: %s, slave is: %s", groupName,
                master == null ? null : master.getNodeName(),
                slave == null ? null : slave.getNodeName()));
        if (nodeCount == 1) {
            assertEquals(master.getNodeName(), slave.getNodeName());
        } else {
            assertNotEquals(master.getNodeName(), slave.getNodeName());
        }
    }

}
