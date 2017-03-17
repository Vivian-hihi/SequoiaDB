package com.sequoiadb.subcl.killnode;

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
import com.sequoiadb.base.DBCursor;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.commlib.GroupMgr;
import com.sequoiadb.commlib.GroupWrapper;
import com.sequoiadb.commlib.NodeWrapper;
import com.sequoiadb.commlib.SdbTestBase;
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.exception.FaultException;
import com.sequoiadb.exception.ReliabilityException;
import com.sequoiadb.fault.KillNode;
import com.sequoiadb.task.FaultMakeTask;
import com.sequoiadb.task.OperateTask;
import com.sequoiadb.task.TaskMgr;

/**
 * @FileName:SEQDB-2433 attachCL过程中catalog主节点异常重启
 * @author huangqiaohui
 * @version 1.00
 *
 */

public class KillNodeSubcl2433 extends SdbTestBase {
    private String mainClName = "testcaseCL2433";
    private List<String> subClName = new ArrayList<String>();
    private CollectionSpace commCS;
    private DBCollection mainCL;
    private GroupMgr groupMgr = null;
    private Sequoiadb commSdb;
    private boolean clearFlag = false;
    private int bound = 0;

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

            commSdb = new Sequoiadb(SdbTestBase.coordUrl, "", "");
            commCS = commSdb.getCollectionSpace(csName);
            mainCL = commCS.createCollection(mainClName, (BSONObject) JSON
                    .parse("{ShardingKey:{'sk':1},ShardingType:'range',IsMainCL:true}"));
            createSubCL(10);
        }
        catch (ReliabilityException e) {
            if (commSdb != null) {
                commSdb.disconnect();
            }
            Assert.fail(this.getClass().getName() + " setUp error, error description:"
                    + e.getMessage() + "\r\n" + Utils.getStackString(e));
        }
    }

    private void createSubCL(int subClCount) {
        for (int i = 0; i < subClCount; i++) {
            DBCollection cl = commCS.createCollection(mainClName + "_sub_" + i);
            subClName.add(cl.getFullName());
        }
    }

    @Test
    public void test() {
        try {
            GroupMgr groupMgr = new GroupMgr();
            GroupWrapper cataGroup = groupMgr.getGroupByName("SYSCatalogGroup");
            NodeWrapper cataMaster = cataGroup.getMaster();

            // 建立并行任务
            FaultMakeTask faultTask = KillNode.getFaultMakeTask(cataMaster.hostName(),
                    cataMaster.svcName(), 1, 30);
            TaskMgr mgr = new TaskMgr(faultTask);
            mgr.addTask(new Attach("Attach"));
            mgr.init();
            mgr.start();
            mgr.join();
            mgr.fini();
            Assert.assertEquals(mgr.isAllSuccess(), true, mgr.getErrorMsg());

            Utils.checkBusinessLSNWithTimeout(groupMgr, 60 * 20);

            CheckCatalog(cataGroup, mainClName);
            for (String subName : subClName) {
                CheckCatalog(cataGroup, subName.split("\\.")[1]);
            }

            // 插入数据
            for (int i = 0; i < bound; i++) {
                mainCL.insert("{sk:" + i + "}");
            }
            DBCursor cusor = mainCL.query(null, "{sk:1}", "{sk:1}", null);
            int count = 0;
            // 查询
            while (cusor.hasNext()) {
                Assert.assertEquals(cusor.getNext(), (BSONObject) JSON.parse("{sk:" + count + "}"));
                count++;
            }
            Assert.assertEquals(count, bound);
            clearFlag = true;
        }
        catch (ReliabilityException e) {
            e.printStackTrace();
            Assert.fail(e.getMessage());
        }

    }

    private void CheckCatalog(GroupWrapper cataGroup, String clName) throws ReliabilityException {
        cataGroup.refresh();
        List<String> urls = cataGroup.getAllUrls();
        ArrayList<BSONObject> res = new ArrayList<BSONObject>();
        for (String url : urls) {
            Sequoiadb db = new Sequoiadb(url, "", "");
            DBCollection cl = db.getCollectionSpace("SYSCAT").getCollection("SYSCOLLECTIONS");
            DBCursor cursor = cl.query(
                    (BSONObject) JSON.parse("{Name:'" + csName + "." + clName + "'}"), null, null,
                    null);
            if (cursor.hasNext()) {
                res.add(cursor.getNext());
                if (cursor.hasNext()) {
                    Assert.fail(clName + " " + cursor.getNext() + " " + res.toString());
                }
            }
            else {
                Assert.fail(clName + "query faile");
            }
        }
        for (BSONObject obj : res) {
            if (!res.get(0).equals(obj)) {
                Assert.fail(clName + ":" + obj + " not equal res.get(0),res:" + res);
            }
        }
    }

    @AfterClass
    public void tearDown() {
        try {
            if (clearFlag) {
                CollectionSpace commCS = commSdb.getCollectionSpace(csName);
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

    class Attach extends OperateTask {

        public Attach(String name) {
            super(name);
        }

        @Override
        public void exec() throws Exception {
            Sequoiadb sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "");
            bound = 0;
            try {
                for (String name : subClName) {
                    mainCL.attachCollection(name, (BSONObject) JSON.parse(
                            "{LowBound:{sk:" + bound + "},UpBound:{sk:" + (bound + 100) + "}}"));
                    bound += 100;
                    Thread.sleep(150);
                }
            }
            catch (BaseException e) {
                if (e.getErrorCode() != -134 && e.getErrorCode() != -104) {
                    throw e;
                }
            }
            catch (InterruptedException e) {
                // ignore
            }
            finally {
                System.out.println("bound:" + bound);
                if (sdb != null) {
                    sdb.disconnect();
                }
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
