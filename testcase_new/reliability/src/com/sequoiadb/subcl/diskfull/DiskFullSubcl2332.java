package com.sequoiadb.subcl.diskfull;

import java.text.SimpleDateFormat;
import java.util.Date;

import org.bson.BSONObject;
import org.bson.util.JSON;
import org.testng.Assert;
import org.testng.SkipException;
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
import com.sequoiadb.exception.FaultException;
import com.sequoiadb.exception.ReliabilityException;
import com.sequoiadb.fault.DiskFull;
import com.sequoiadb.task.FaultMakeTask;
import com.sequoiadb.task.OperateTask;
import com.sequoiadb.task.TaskMgr;

/**
 * @FileName:SEQDB-2332 在主表做基本操作时dataRG备节点所在服务器磁盘满
 * @author huangqiaohui
 * @version 1.00
 *
 */

public class DiskFullSubcl2332 extends SdbTestBase {
    private String mainClName = "testcaseCL2332_main";
    private String subClName = "testcaseCL2332_sub";
    private String subClGroupName;
    private CollectionSpace commCS;
    private DBCollection mainCL;
    private DBCollection subCL;
    private GroupMgr groupMgr = null;
    private Sequoiadb commSdb;
    private boolean clearFlag = false;
    public int insertCount = 0;

    @BeforeClass()
    public void setUp() {
        try {
            System.out.println(
                    "the TestCase Name:" + this.getClass().getName() + ". the TestCase begin at:"
                            + new SimpleDateFormat("YYYY-MM-dd HH:mm:ss.SSS").format(new Date()));
            groupMgr = GroupMgr.getInstance();

            // CheckBusiness(true),检测当前集群环境，若存在异常返回false，
            if (!groupMgr.checkBusiness(true)) {
                throw new SkipException("checkBusiness faile");
            }
            subClGroupName = groupMgr.getAllDataGroup().get(0).getGroupName();

            commSdb = new Sequoiadb(SdbTestBase.coordUrl, "", "");
            commSdb.setSessionAttr((BSONObject) JSON.parse("{PreferedInstance:'M'}"));
            commCS = commSdb.getCollectionSpace(csName);
            mainCL = commCS.createCollection(mainClName, (BSONObject) JSON
                    .parse("{ShardingKey:{'sk':1},ShardingType:'range',IsMainCL:true}"));
            subCL = commCS.createCollection(subClName, (BSONObject) JSON.parse(
                    "{ShardingKey:{sk:1},ShardingType:'range',Group:'" + subClGroupName + "'}"));
            mainCL.attachCollection(subCL.getFullName(),
                    (BSONObject) JSON.parse("{LowBound:{sk:0},UpBound:{sk:10000}}"));

        }
        catch (ReliabilityException e) {
            if (commSdb != null) {
                commSdb.disconnect();
            }
            Assert.fail(this.getClass().getName() + " setUp error, error description:"
                    + e.getMessage() + "\r\n" + Utils.getStackString(e));
        }
    }

    @Test
    public void test() {
        try {
            GroupWrapper subGroup = groupMgr.getGroupByName(subClGroupName);
            NodeWrapper subCLGroupSlave = subGroup.getSlave();
            FaultMakeTask faultMakeTask = DiskFull.getFaultMakeTask(subCLGroupSlave.hostName(),
                    SdbTestBase.workDir, 1, 10, 96);

            TaskMgr taskMgr = new TaskMgr(faultMakeTask);
            taskMgr.addTask(new Insert("insert"));
            taskMgr.init();
            taskMgr.start();
            taskMgr.join();
            taskMgr.fini();
            Assert.assertEquals(taskMgr.isAllSuccess(), true, taskMgr.getErrorMsg());

            checkAndInsert();

            if (!subGroup.checkInspect(120, 2)) {
                Assert.fail(subGroup.getInspectStdout());
            }

            clearFlag = true;
        }
        catch (ReliabilityException e) {
            e.printStackTrace();
            Assert.fail(e.getMessage());
        }

    }

    private void checkAndInsert() {
        String padStr = Utils.getString(1024 * 1024);
        for (int i = insertCount; i < insertCount + 2; i++) {
            mainCL.insert("{sk:" + i + ",pad:'" + padStr + "'}");
        }
        insertCount += 2;

        DBCursor cursor = null;
        cursor = mainCL.query(null, "{sk:1,pad:1}", "{sk:1}", null);
        int count = 0;
        while (cursor.hasNext()) {
            BSONObject obj = cursor.getNext();
            Assert.assertEquals(obj,
                    (BSONObject) JSON.parse("{sk:" + count + ",pad:'" + padStr + "'}"));
            count++;
        }

        Assert.assertEquals(count, insertCount);
    }

    @AfterClass
    public void tearDown() {
        try {
            if (clearFlag) {
                CollectionSpace commCS = commSdb.getCollectionSpace(csName);
                commCS.dropCollection(subClName);
                commCS.dropCollection(mainClName);
            }
        }
        catch (BaseException e) {
            Assert.fail(e.getMessage() + "\r\n" + Utils.getStackString(e));
        }
        finally {
            if (commSdb != null) {
                commSdb.disconnect();
            }
            System.out.println(
                    "the TestCase Name:" + this.getClass().getName() + ". the TestCase end at:"
                            + new SimpleDateFormat("YYYY-MM-dd HH:mm:ss.SSS").format(new Date()));
        }
    }

    class Insert extends OperateTask {

        public Insert(String name) {
            super(name);
        }

        @Override
        public void exec() throws Exception {
            String padStr = Utils.getString(1024 * 1024);
            for (int i = 0; i < 128; i++) {
                mainCL.insert("{sk:" + i + ",pad:'" + padStr + "'}");
                insertCount++;
            }
        }

        @Override
        public void faultNotify(BSONObject status) throws FaultException {
            OperateTask.faultStatus mk = (faultStatus) status.get(FaultMakeTask.MAKE_RESULT);
            OperateTask.faultStatus rt = (faultStatus) status.get(FaultMakeTask.RESTORE_RESULT);
            if (mk == OperateTask.faultStatus.MAKEFAILURE) {
                throw new FaultException(mk.toString());
            }
            if (rt == OperateTask.faultStatus.RESTOREFAILURE) {
                throw new FaultException(rt.toString());
            }
        }
    }
}
