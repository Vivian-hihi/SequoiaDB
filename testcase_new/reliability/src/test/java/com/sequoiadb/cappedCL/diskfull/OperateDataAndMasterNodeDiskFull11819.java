package com.sequoiadb.cappedCL.diskfull;

import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.DBCursor;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.cappedCL.Utils;
import com.sequoiadb.commlib.GroupMgr;
import com.sequoiadb.commlib.GroupWrapper;
import com.sequoiadb.commlib.NodeWrapper;
import com.sequoiadb.commlib.SdbTestBase;
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.exception.ReliabilityException;
import com.sequoiadb.fault.DiskFull;
import com.sequoiadb.task.FaultMakeTask;
import com.sequoiadb.task.OperateTask;
import com.sequoiadb.task.TaskMgr;
import org.bson.BSONObject;
import org.bson.BasicBSONObject;
import org.bson.util.JSON;
import org.testng.Assert;
import org.testng.SkipException;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import java.util.Random;

/**
 * @FileName seqDB-11819 operating data when master node disk is full
 * @author xiejianhong
 * @Date 2017-08-16
 * @version 1.0
 */
public class OperateDataAndMasterNodeDiskFull11819 extends SdbTestBase {

    private GroupMgr groupMgr = null;
    private TaskMgr taskMgr = null;
    private GroupWrapper dataGroup = null;
    private Sequoiadb db = null;
    private boolean runSuccess = false;
    private static final int INSERT_COUNT = 10000;
    private static final int INSERT_ONCE = 3;
    private static final int SDB_NO_SPACE = -11;        //no disk space
    private final String CSNAME = "cs_11819_master";
    private final String CLNAME = "cl_11819_master";
    private static final int CHECK_TIME = 120;

    @BeforeClass
    public void setUp() {

        try {
            //print testcase begin time
            Utils.printBeginTime(this);
            groupMgr = new GroupMgr();
            //check whether environment is normal
            if (!groupMgr.checkBusiness(CHECK_TIME)) {
                throw new SkipException("checkBusiness failed");
            }

            db = new Sequoiadb(coordUrl, "", "");
            dataGroup = groupMgr.getAllDataGroup().get(0);
            createOperateDataCL(db);
            InsertDataForPop(db);
        } catch (ReliabilityException e) {
            if(db != null) {
                db.close();
            }
            Assert.fail(this.getClass().getName() + " setUp error, error description:" + e.getMessage() + "\r\n"
                    + Utils.getKeyStack(e, this));
        }
    }

    /**
     * 1. operating data and master node disk full
     * 2. restore master node disk
     * 3. check whether InsertTask and PopTask worked successfully
     * 4. check whether environment is normal and LSN is consistent
     * 5. check whether data group lsn is consistent
     */
    @Test
    public void test() {
        Sequoiadb db = null;
        try {
            NodeWrapper masterNode = dataGroup.getMaster();

            //set tasks
            FaultMakeTask faultTask = DiskFull.getFaultMakeTask(masterNode.hostName(), SdbTestBase.reservedDir, 0, 10);
            taskMgr = new TaskMgr(faultTask);
            taskMgr.addTask(new InsertTask());
            taskMgr.addTask(new PopTask());
            taskMgr.execute();

            Assert.assertEquals(taskMgr.isAllSuccess(), true, taskMgr.getErrorMsg());
            Assert.assertEquals(groupMgr.checkBusinessWithLSN(600),true);
            Assert.assertEquals(dataGroup.checkInspect(60),true);

            db = new Sequoiadb(coordUrl, "", "");
            OperateData(db);
            runSuccess = true;
        } catch (ReliabilityException e) {
            e.printStackTrace();
            Assert.fail(e.getMessage());
        } finally {
            if(db != null) {
                db.close();
            }
        }
    }


    @AfterClass()
    public void tearDown() {

        if (!runSuccess) {
            throw new SkipException("to save environment");
        }
        Sequoiadb db = null;
        try {
            db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
            db.dropCollectionSpace(CSNAME);
        } catch (BaseException e) {
            Assert.fail(e.getMessage() + "\r\n" + Utils.getKeyStack(e, this));
        } finally {
            if (db != null) {
                db.close();
            }
            Utils.printEndTime(this);
        }
    }

    private class InsertTask extends OperateTask {

        private Sequoiadb db = null;

        @Override
        public void exec() throws Exception {
            try {
                db = new Sequoiadb(coordUrl,"","");
                DBCollection cl = db.getCollectionSpace(CSNAME).getCollection(CLNAME);
                Random random = new Random();
                byte[] bytes = new byte[ 1024 * 1024 ];
                random.nextBytes(bytes);
                for(int i = 0;i<INSERT_COUNT;i++) {
                    for(int j =0;j<INSERT_ONCE;j++) {
                        BSONObject object = new BasicBSONObject();
                        object.put("id","insert");
                        object.put("value",bytes);
                        cl.insert(object);
                    }
                }
            } catch (BaseException e) {
                if(e.getErrorCode() == SDB_NO_SPACE ) {
                    System.out.println(e.getMessage());
                } else {
                    throw e;
                }
            } finally {
                if(db != null) {
                    db.close();
                }
            }

        }
    }

    private class PopTask extends OperateTask {

        private Sequoiadb db = null;

        @Override
        public void exec() throws Exception {
            try {
                db = new Sequoiadb(coordUrl,"","");
                DBCollection cl = db.getCollectionSpace(CSNAME).getCollection(CLNAME);
                for(int i = 0;i<INSERT_COUNT;i++) {
                    for(int j =0;j<INSERT_ONCE;j++) {
                        BSONObject object = new BasicBSONObject();
                        object.put("id",i*INSERT_ONCE+j);
                        DBCursor cursor= cl.query(object,null,null,null);
                        long logicalID = 0;
                        if(cursor.hasNext()) {
                            logicalID= (long)cursor.getNext().get("_id");
                        }
                        BSONObject popObject = new BasicBSONObject();
                        popObject.put("LogicalID",logicalID);
                        popObject.put("Direction",1);
                        cl.pop(popObject);
                    }
                }
            } catch (BaseException e) {
                if(e.getErrorCode() == SDB_NO_SPACE) {
                    System.out.println(e.getMessage());
                } else {
                    throw e;
                }
            } finally {
                if(db != null) {
                    db.close();
                }
            }

        }
    }
    public void OperateData(Sequoiadb db) {
        DBCollection cl = db.getCollectionSpace(CSNAME).getCollection(CLNAME);
        for(int i =0;i<INSERT_COUNT;i++) {
            BSONObject object = new BasicBSONObject();
            object.put("id","checkCorrect");
            object.put("value","correct");
            cl.insert(object);
            BSONObject findObject = new BasicBSONObject();
            findObject.put("id","checkCorrect");
            DBCursor cursor= cl.query(findObject,null,null,null);
            long logicalID = 0;
            if(cursor.hasNext()) {
                logicalID= (long)cursor.getNext().get("_id");
            }
            BSONObject popObject = new BasicBSONObject();
            popObject.put("LogicalID",logicalID);
            popObject.put("Direction",1);
            cl.pop(popObject);
        }
    }

    public void createOperateDataCL(Sequoiadb db) {
        BSONObject csOption = (BSONObject)JSON.parse("{Capped:true}");
        BSONObject clOption = (BSONObject)JSON.parse("{Capped:true,Size:2000,Max:0,AutoIndexId:false,Group:'"+dataGroup.getGroupName()+"'}");
        db.createCollectionSpace(CSNAME,csOption).createCollection(CLNAME,clOption);
    }

    public void InsertDataForPop(Sequoiadb db) {
        DBCollection cl = db.getCollectionSpace(CSNAME).getCollection(CLNAME);
        for(int i = 0;i<INSERT_COUNT;i++) {
            for(int j =0;j<INSERT_ONCE;j++) {
                BSONObject object = new BasicBSONObject();
                object.put("id",i*INSERT_ONCE+j);
                object.put("value","aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");
                cl.insert(object);
            }
        }
    }
}
