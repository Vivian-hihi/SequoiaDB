package com.sequoiadb.fulltext.killnode;

import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Date;
import java.util.List;

import org.bson.BSONObject;
import org.bson.util.JSON;
import org.testng.Assert;
import org.testng.SkipException;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import com.sequoiadb.base.CollectionSpace;
import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.commlib.CommLib;
import com.sequoiadb.commlib.GroupMgr;
import com.sequoiadb.commlib.GroupWrapper;
import com.sequoiadb.commlib.NodeWrapper;
import com.sequoiadb.commlib.SdbTestBase;
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.exception.ReliabilityException;
import com.sequoiadb.fault.KillNode;
import com.sequoiadb.fulltext.FullTextUtils;
import com.sequoiadb.task.FaultMakeTask;
import com.sequoiadb.task.TaskMgr;

/**
 * @Description seqDB-14406:异常启动DB备节点不影响全文索引功能
 * @author yinzhen
 * @date 2018/11/23
 */

public class Fulltext14408 extends SdbTestBase {
    private String clName = "killSlaveNode14408";
    private CollectionSpace cs;
    private DBCollection cl;
    private GroupMgr groupMgr = null;
    private Sequoiadb sdb;
    private boolean clearFlag = false;
    private String groupName;
    private String fullIndexName = "fullIndex14408";

    @BeforeClass()
    public void setUp() {
        try {
            sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "");
            if (CommLib.isStandAlone(sdb)) {
                throw new SkipException("StandAlone environment!");
            }

            System.out.println("the TestCase Name:" + this.getClass().getName() + ". the TestCase begin at:"
                    + new SimpleDateFormat("YYYY-MM-dd HH:mm:ss.SSS").format(new Date()));
            groupMgr = GroupMgr.getInstance();

            // CheckBusiness(true),检测当前集群环境，若存在异常返回false，
            if (!groupMgr.checkBusiness(20)) {
                throw new SkipException("checkBusiness return false");
            }
            groupName = groupMgr.getAllDataGroupName().get(0);

            cs = sdb.getCollectionSpace(csName);
            cl = cs.createCollection(clName, (BSONObject) JSON.parse("{Group:'" + this.groupName + "'}"));

        } catch (ReliabilityException e) {
            if (sdb != null) {
                sdb.close();
            }
            Assert.fail(this.getClass().getName() + " setUp error, error description:" + e.getMessage());
        }
    }

    public void insertData() {
        List<BSONObject> records = new ArrayList<BSONObject>();
        for (int i = 0; i < 100; i++) {
            for (int j = 0; j < 5000; j++) {
                BSONObject record = (BSONObject) JSON.parse("{a:'a" + i + "" + j + "',g:'g" + i + "" + j + "'}");
                records.add(record);
            }
            this.cl.insert(records);
            records.clear();
        }
    }

    @Test
    public void test() throws Exception {
        try {
            this.cl.createIndex(fullIndexName, "{\"a\":\"text\"}", false, false);
            this.insertData();
            Assert.assertTrue(FullTextUtils.isIndexCreated(cl, fullIndexName, 500000));
            GroupWrapper subCLGroup = groupMgr.getGroupByName(groupName);
            NodeWrapper subCLGroupMaster = subCLGroup.getSlave();
            System.out.println("Kill node:" + subCLGroupMaster.hostName() + ":" + subCLGroupMaster.svcName());

            // 建立并行任务
            FaultMakeTask faultTask = KillNode.getFaultMakeTask(subCLGroupMaster.hostName(), subCLGroupMaster.svcName(),
                    0);
            TaskMgr mgr = new TaskMgr(faultTask);
            mgr.execute();
            Assert.assertTrue(mgr.isAllSuccess(), mgr.getErrorMsg());
            Assert.assertTrue(groupMgr.checkBusinessWithLSN(120));

            Assert.assertTrue(FullTextUtils.isIndexCreated(cl, fullIndexName, 500000));

            // insert
            this.insertData();
            Assert.assertTrue(FullTextUtils.isIndexCreated(cl, fullIndexName, 1000000));

            // update
            this.cl.update("{a:'a01'}", "{'$set':{a:'helloworld'}}", "{'':null}");
            Assert.assertTrue(FullTextUtils.isIndexCreated(cl, fullIndexName, 1000000));

            // delete
            this.cl.delete("{a:'a11'}");
            Assert.assertTrue(FullTextUtils.isIndexCreated(cl, fullIndexName, 999998));

            // query
            this.cl.query();
            Assert.assertTrue(FullTextUtils.isIndexCreated(cl, fullIndexName, 999998));

            clearFlag = true;
        } catch (ReliabilityException e) {
            e.printStackTrace();
            Assert.fail(e.getMessage());
        } finally {
            sdb.closeAllCursors();
        }

    }

    @AfterClass
    public void tearDown() {
        try {
            if (clearFlag) {
                CollectionSpace commCS = sdb.getCollectionSpace(csName);
                commCS.dropCollection(clName);
            }
        } catch (BaseException e) {
            Assert.fail(e.getMessage());
        } finally {
            if (sdb != null) {
                sdb.close();
            }
            System.out.println("the TestCase Name:" + this.getClass().getName() + ". the TestCase end at:"
                    + new SimpleDateFormat("YYYY-MM-dd HH:mm:ss.SSS").format(new Date()));
        }
    }
}
