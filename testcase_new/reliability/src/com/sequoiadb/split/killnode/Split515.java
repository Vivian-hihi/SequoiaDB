package com.sequoiadb.split.killnode;

import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.List;

import org.bson.BSONObject;
import org.bson.types.BasicBSONList;
import org.bson.util.JSON;
import org.testng.Assert;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import com.sequoiadb.base.CollectionSpace;
import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.DBCursor;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.commlib.GroupMgr;
import com.sequoiadb.commlib.GroupWrapper;
import com.sequoiadb.commlib.NodeWrapper;
import com.sequoiadb.commlib.SdbTestBase;
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.exception.ReliabilityException;
import com.sequoiadb.fault.KillNode;
import com.sequoiadb.task.FaultMakeTask;
import com.sequoiadb.task.OperateTask;
import com.sequoiadb.task.TaskMgr;

public class Split515 extends SdbTestBase {
    public static final int FLG_INSERT_CONTONDUP = 0x00000001;
    private String clName = "split515";
    private String srcGroupName;
    private String destGroupName;
    private Sequoiadb commSdb = null;
    private GroupMgr groupMgr = null;
    private String killedHost = "";
    private String killedPort = "";
    
    @BeforeClass
    public void setUp() {
        try {
            commSdb = new Sequoiadb(coordUrl, "", "");
            groupMgr = new GroupMgr();
            CollectionSpace commCS = commSdb.getCollectionSpace(csName);
            List<GroupWrapper> glist = groupMgr.getAllDataGroup();
            srcGroupName = glist.get(0).getGroupName();
            destGroupName = glist.get(1).getGroupName();
            commCS.createCollection(clName, (BSONObject) JSON.parse("{ShardingKey:{\"a\":1},Group:\""+srcGroupName+"\",ReplSize:1,ShardingType:\"range\"}"));
            prepareData(commSdb); // 准备切分的数据
            
        } catch (ReliabilityException e) {
            if (commSdb != null) {
                commSdb.disconnect();
            }
            Assert.fail(this.getClass().getName() + " setUp error, error description:"
                    + e.getMessage() + "\r\n" + Utils.getStackString(e));
        }
    }
    
    public void prepareData(Sequoiadb db) {
        try {
            DBCollection cl = db.getCollectionSpace(csName).getCollection(clName);
            ArrayList<BSONObject> arr = new ArrayList<BSONObject>();
            for (int i = 0; i < 1000000; i++) {
                arr.add((BSONObject) JSON.parse("{a:" + i + "}"));
            }
            cl.bulkInsert(arr,this.FLG_INSERT_CONTONDUP);
        } catch (BaseException e) {
            throw e;
        }
    }
    
    @Test
    public void test() {
        try {
            GroupWrapper destGroup = groupMgr.getGroupByName(destGroupName);
            NodeWrapper destSlave = destGroup.getMaster();
            killedHost = destSlave.hostName();
            killedPort = destSlave.svcName();
            
            
            System.out.println("KillNode:" + killedHost + ":" + killedPort);
            
            FaultMakeTask faultTask = KillNode.getFaultMakeTask(killedHost,killedPort, 0);
            TaskMgr mgr1 = new TaskMgr(faultTask);
            mgr1.addTask(new Split());
            mgr1.execute();
            Assert.assertEquals(mgr1.isAllSuccess(), true, mgr1.getErrorMsg());
            Assert.assertEquals(groupMgr.checkBusiness(600), true, "failed to restore business");
            Assert.assertEquals(100000, checkGroupData(srcGroupName));
            Assert.assertEquals(900000, checkGroupData(destGroupName));
        }catch (ReliabilityException e) {
            Assert.fail(this.getClass().getName() + " test error, error description:"
                    + e.getMessage() + "\r\n" + Utils.getStackString(e));
        } finally {

        }
    }
    
    private long checkGroupData(String groupName) {
        Sequoiadb dataNode = null;
        Sequoiadb sdb = null;
        DBCursor cursor = null;
        try {
            sdb = new Sequoiadb(coordUrl, "", "");
            dataNode = sdb.getReplicaGroup(groupName).getMaster().connect();// 获得目标组主节点链接
            DBCollection cl = dataNode.getCollectionSpace(csName).getCollection(clName);
            long count = cl.getCount();
            return count;
        }
        catch (BaseException e) {
            e.printStackTrace();
            Assert.fail(e.getMessage() + "\r\n" + Utils.getStackString(e));
        }
        finally {
            if (cursor != null) {
                cursor.close();
            }
            if (dataNode != null) {
                dataNode.close();
            }
            if (sdb != null) {
                sdb.close();
            }
        }
        return 0;
    }
    
    @AfterClass
    public void tearDown() {
        try {
            CollectionSpace commCS = commSdb.getCollectionSpace(csName);
            commCS.dropCollection(clName);
        } catch (BaseException e) {
            Assert.fail(this.getClass().getName() + " setUp error, error description:"
                    + e.getMessage() + "\r\n" + Utils.getStackString(e));
        } finally {
            if (commSdb != null) {
                commSdb.disconnect();
            }
        }
    }
      
    class Split extends OperateTask {

        @Override
        public void exec() throws Exception {
            Sequoiadb sdb = null;
            try {
                sdb = new Sequoiadb(coordUrl, "", "");
                DBCollection cl = sdb.getCollectionSpace(csName).getCollection(clName);
                cl.split(srcGroupName, destGroupName, (BSONObject) JSON.parse("{a:0}"),
                        (BSONObject) JSON.parse("{a:900000}"));
            } catch (BaseException e) {
                throw e;
            } finally {
                if (sdb != null) {
                    sdb.disconnect();
                }
            }
        }
    }
}
