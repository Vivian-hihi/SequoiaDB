package com.sequoiadb.cappedCL.restartnode;

import org.bson.BSONObject;
import org.bson.util.JSON;
import org.testng.Assert;
import org.testng.SkipException;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import com.sequoiadb.base.CollectionSpace;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.cappedCL.Utils;
import com.sequoiadb.commlib.GroupMgr;
import com.sequoiadb.commlib.GroupWrapper;
import com.sequoiadb.commlib.NodeWrapper;
import com.sequoiadb.commlib.SdbTestBase;
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.exception.ReliabilityException;
import com.sequoiadb.fault.NodeRestart;
import com.sequoiadb.task.FaultMakeTask;
import com.sequoiadb.task.OperateTask;
import com.sequoiadb.task.TaskMgr;

/**
 * @FileName seqDB-11813 dropping collection when slave node disk restart
 * @author xiejianhong
 * @Date 2017-08-16
 * @version 1.0
 */
public class DropCappedCLAndSlaveNodeRestart11815 extends SdbTestBase {

    private GroupMgr groupMgr = null;
    private TaskMgr taskMgr = null;
    private GroupWrapper dataGroup = null;
    private Sequoiadb db = null;
    private boolean runSuccess = false;
    private String clGroupName = null;
    private final String CSNAMEBASE = "cs_11815_restart";
    private final String CLNAMEBASE = "cl_11815_restart";
    private static final int CL_NUM = 2000;
    private static final int CS_NUM = 1;
    private static final int CHECK_TIME = 120;

    @BeforeClass
    public void setUp() {

        try {
            groupMgr = GroupMgr.getInstance();
            // check whether environment is normal
            if (!groupMgr.checkBusiness(CHECK_TIME)) {
                throw new SkipException("checkBusiness failed");
            }

            db = new Sequoiadb(coordUrl, "", "");
            clGroupName = groupMgr.getAllDataGroupName().get(0);
            // create cs for dropping
            createCSForDropping(db);
        } catch (ReliabilityException e) {
            if (db != null) {
                db.close();
            }
            e.printStackTrace();
            Assert.fail(this.getClass().getName() + " setUp error, error description:" + e.getMessage() + "\r\n"
                    + Utils.getKeyStack(e, this));
        }
    }

    /**
     * 1. dropping capped collection when slave node restart 2. check whether
     * DropCLTask worked successfully 3. check whether environment is normal and
     * LSN is consistent 4. check whether catalog group lsn is consistent
     */
    @Test
    public void test() {

        try {
            dataGroup = groupMgr.getGroupByName(clGroupName);
            NodeWrapper slaveNode = dataGroup.getSlave();

            // set tasks
            FaultMakeTask fault = NodeRestart.getFaultMakeTask(slaveNode, 0, 10);
            taskMgr = new TaskMgr(fault);
            taskMgr.addTask(new DropCLTask());
            taskMgr.execute();

            // check status
            Assert.assertEquals(taskMgr.isAllSuccess(), true, taskMgr.getErrorMsg());
            Assert.assertEquals(groupMgr.checkBusinessWithLSN(600), true);
            GroupWrapper catalogGroup = groupMgr.getGroupByName("SYSCatalogGroup");
            Assert.assertEquals(catalogGroup.checkInspect(60), true);

            runSuccess = true;
        } catch (ReliabilityException e) {
            Assert.fail(e.getMessage() + "\r\n" + Utils.getKeyStack(e, this));
        } finally {
            if (db != null) {
                db.close();
            }
        }
    }

    @AfterClass
    public void tearDown() {

        if (!runSuccess) {
            throw new SkipException("to save environment");
        }
        Sequoiadb db = null;
        try {
            db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
            dropCSs(db);
        } catch (BaseException e) {
            Assert.fail(e.getMessage() + "\r\n" + Utils.getKeyStack(e, this));
        } finally {
            if (db != null) {
                db.close();
            }
        }
    }

    private void createCSForDropping(Sequoiadb db) {
        BSONObject csOption = (BSONObject) JSON.parse("{Capped:true}");
        BSONObject clOption = (BSONObject) JSON
                .parse("{Capped:true,Size:100,Max:0,AutoIndexId:false,Group:'" + clGroupName + "'}");
        for (int i = 0; i < CS_NUM; i++) {
            String csName = CSNAMEBASE + "_" + i;
            CollectionSpace cs = db.createCollectionSpace(csName, csOption);
            for (int j = 0; j < CL_NUM; j++) {
                String clName = CLNAMEBASE + "_" + j;
                cs.createCollection(clName, clOption);
            }
        }
    }

    private void dropCSs(Sequoiadb db) {
        for (int i = 0; i < CS_NUM; i++) {
            String csName = CSNAMEBASE + "_" + i;
            if (db.isCollectionSpaceExist(csName)) {
                db.dropCollectionSpace(csName);
            } else {
                break;
            }
        }
    }

    private class DropCLTask extends OperateTask {

        private Sequoiadb db = null;

        @Override
        public void exec() throws Exception {
            try {
                db = new Sequoiadb(coordUrl, "", "");
                for (int i = 0; i < CS_NUM; i++) {
                    String csName = CSNAMEBASE + "_" + i;
                    if (db.isCollectionSpaceExist(csName)) {
                        CollectionSpace cs = db.getCollectionSpace(csName);
                        for (int j = 0; j < CL_NUM; j++) {
                            String clName = CLNAMEBASE + "_" + j;
                            if (cs.isCollectionExist(clName)) {
                                cs.dropCollection(clName);
                            }
                        }
                    }
                }
            } catch (BaseException e) {
                throw e;
            } finally {
                if (db != null) {
                    db.close();
                }
            }
        }
    }
}
