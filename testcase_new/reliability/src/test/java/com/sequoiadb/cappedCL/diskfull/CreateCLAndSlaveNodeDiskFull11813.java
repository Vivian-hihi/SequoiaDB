package com.sequoiadb.cappedCL.diskfull;

import com.sequoiadb.base.CollectionSpace;
import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.cappedCL.Utils;
import com.sequoiadb.commlib.GroupMgr;
import com.sequoiadb.commlib.GroupWrapper;
import com.sequoiadb.commlib.NodeWrapper;
import com.sequoiadb.commlib.SdbTestBase;
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.exception.ReliabilityException;
import com.sequoiadb.fault.DiskFull;
import com.sequoiadb.task.OperateTask;
import com.sequoiadb.task.TaskMgr;
import org.bson.BSONObject;
import org.bson.util.JSON;
import org.testng.Assert;
import org.testng.SkipException;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

/**
 * @FileName seqDB-11813 creating collection when slave node disk is full
 * @author xiejianhong
 * @Date 2017-08-16
 * @version 1.0
 */
public class CreateCLAndSlaveNodeDiskFull11813 extends SdbTestBase {

    private GroupMgr groupMgr = null;
    private TaskMgr taskMgr = null;
    private GroupWrapper dataGroup = null;
    private Sequoiadb db = null;
    private boolean runSuccess = false;
    private String clGroupName = null;
    private final String CSNAMEBASE = "cs_11813_slave";
    private final String CLNAMEBASE = "cs_11813_slave";
    private static final int CL_NUM = 50;
    private static final int CS_NUM = 1;
    private static final int CHECK_TIME = 120;
    private static final int DISKFULL_PRESET_PERCENT = 95;

    @BeforeClass
    public void setUp() {

        try {
            //print the testcase begin time
            Utils.printBeginTime(this);

            groupMgr = new GroupMgr();
            //check whether the environment is normal
            if (!groupMgr.checkBusiness(CHECK_TIME)) {
                throw new SkipException("checkBusiness failed");
            }

            db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
            clGroupName = groupMgr.getAllDataGroupName().get(0);
        } catch (ReliabilityException e) {
            if(db != null) {
                db.close();
            }
            Assert.fail(this.getClass().getName() + " setUp error, error description:" + e.getMessage() + "\r\n"
                    + Utils.getKeyStack(e, this));
        }
    }

    /**
     * 1. make slave node disk full
     * 2. loop create capped collection
     * 3. restore slave node disk
     * 4. check whether createCLTask worked successfully
     * 5. check whether environment is normal and LSN is consistent
     * 6. check whether catalog group lsn is consistent
     */
    @Test
    public void test() {
        Sequoiadb db = null;
        try {
            dataGroup = groupMgr.getGroupByName(clGroupName);
            NodeWrapper slaveNode = dataGroup.getSlave();
            DiskFull diskFull = new DiskFull(slaveNode.hostName(), SdbTestBase.reservedDir,DISKFULL_PRESET_PERCENT);
            diskFull.init();
            diskFull.make();

            //set tasks
            taskMgr = new TaskMgr();
            taskMgr.addTask(new CreateCappedCLTask());
            taskMgr.execute();

            //restore disk
            diskFull.restore();

            //check status
            Assert.assertEquals(taskMgr.isAllSuccess(), true, taskMgr.getErrorMsg());
            Assert.assertEquals(groupMgr.checkBusinessWithLSN(600),true);
            GroupWrapper catalogGroup = groupMgr.getGroupByName("SYSCatalogGroup");
            Assert.assertEquals(catalogGroup.checkInspect(60),true);

            db = new Sequoiadb(SdbTestBase.coordUrl,"","");
            checkCLUseable(db);
            runSuccess = true;
        } catch (ReliabilityException e) {
            Assert.fail(e.getMessage() + "\r\n" + Utils.getKeyStack(e,this));
        } finally {
            if(db != null) {
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
            Utils.printEndTime(this);
        }
    }

    private void dropCSs(Sequoiadb db) {
        for(int i = 0;i<CS_NUM;i++) {
            String csName = CSNAMEBASE+"_"+i;
            if(db.isCollectionSpaceExist(csName)) {
                db.dropCollectionSpace(csName);
            } else {
                break;
            }
        }
    }

    private class CreateCappedCLTask extends OperateTask {

        private Sequoiadb db = null;

        @Override
        public void exec() throws Exception {
            try {
                db = new Sequoiadb(coordUrl, "", "");
                BSONObject csOption = (BSONObject)JSON.parse("{Capped:true}");
                BSONObject clOption = (BSONObject)JSON.parse("{Capped:true,Size:100,Max:0,AutoIndexId:false,Group:'"+clGroupName+"'}");
                for(int i = 0; i< CS_NUM; i++) {
                    String csName = CSNAMEBASE + "_" + i;
                    CollectionSpace cs = db.createCollectionSpace(csName,csOption);
                    for(int j = 0;j<CL_NUM;j++) {
                        String clName = CLNAMEBASE + "_" + j;
                        cs.createCollection(clName,clOption);
                    }
                }
            } catch (BaseException e) {
                throw e;
            } finally {
                if(db != null) {
                    db.close();
                }
            }
        }
    }

    private void checkCLUseable(Sequoiadb db) {
        for (int i = 0; i < CS_NUM; i++) {
            String csName = CSNAMEBASE + "_" + i;
            if(db.isCollectionSpaceExist(csName)) {
                CollectionSpace cs = db.getCollectionSpace(csName);
                for (int j = 0; j < CL_NUM; j++) {
                    String clName = CLNAMEBASE + "_" + j;
                    if(cs.isCollectionExist(clName)) {
                        DBCollection cl = cs.getCollection(clName);
                        cl.insert("{a:1}");
                    } else {
                        break;
                    }
                }
            } else {
                break;
            }
        }
    }
}
