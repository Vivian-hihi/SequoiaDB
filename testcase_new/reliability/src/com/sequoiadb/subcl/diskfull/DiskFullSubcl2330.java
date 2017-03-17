package com.sequoiadb.subcl.diskfull;

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
import com.sequoiadb.fault.DiskFull;
import com.sequoiadb.task.OperateTask;

/**
 * @FileName:SEQDB-2330 detachCL过程中catalog备节点所在服务器磁盘满
 * @author huangqiaohui
 * @version 1.00
 *
 */

public class DiskFullSubcl2330 extends SdbTestBase {
    private String mainClName = "testcaseCL2330";
    private List<String> subClNames = new ArrayList<String>();
    private CollectionSpace commCS;
    private DBCollection mainCL;
    private GroupMgr groupMgr = null;
    private Sequoiadb commSdb;
    private boolean clearFlag = false;
    private int bound = 0;
    private NodeWrapper cataSlave = null;
    private String pad_M;
    private String pad_K;
    private String pad_HK;
    private String pad_HM;

    @BeforeClass()
    public void setUp() {
        try {
            System.out.println(
                    "the TestCase Name:" + this.getClass().getName() + ". the TestCase begin at:"
                            + new SimpleDateFormat("YYYY-MM-dd HH:mm:ss.SSS").format(new Date()));
            groupMgr = GroupMgr.getInstance();
            cataSlave = groupMgr.getGroupByName("SYSCatalogGroup").getSlave();
            // CheckBusiness(true),检测当前集群环境，若存在异常返回false，
            if (!groupMgr.checkBusiness(true)) {
                throw new SkipException("checkBusiness faile");
            }

            commSdb = new Sequoiadb(SdbTestBase.coordUrl, "", "");
            commCS = commSdb.getCollectionSpace(csName);
            mainCL = commCS.createCollection(mainClName, (BSONObject) JSON
                    .parse("{ShardingKey:{'sk':1},ShardingType:'range',IsMainCL:true}"));

            createSubCLAndAttach(500);

            pad_M = Utils.getString(1024 * 1024);
            pad_HM = Utils.getString(512 * 1024);
            pad_K = Utils.getString(1024);
            pad_HK = Utils.getString(512);

        }
        catch (ReliabilityException e) {
            if (commSdb != null) {
                commSdb.disconnect();
            }
            Assert.fail(this.getClass().getName() + " setUp error, error description:"
                    + e.getMessage() + "\r\n" + Utils.getStackString(e));
        }
    }

    private void fillUpCatalogSYSCL(String name, String padStr) {
        Sequoiadb db = null;
        try {
            System.out.println("strlen:" + padStr.length());
            db = new Sequoiadb(cataSlave.hostName() + ":" + cataSlave.svcName(), "", "");
            DBCollection cl = db.getCollectionSpace("SYSCAT").getCollection("SYSCOLLECTIONS");
            int i = 0;
            try {
                while (true) {
                    cl.insert("{Name:'" + name + i + "',pad:'" + padStr + i + "',deleteFlag:1}");
                    i++;
                }
            }
            catch (BaseException e) {
                System.out.println("fillUpCataSYSCL:" + e.getErrorCode());
                if (e.getErrorCode() != -11) {
                    throw e;
                }
            }
        }
        finally {
            if (db != null) {
                db.disconnect();
            }
        }
    }

    private void createSubCLAndAttach(int subClCount) {
        for (int i = 0; i < subClCount; i++) {
            DBCollection cl = commCS.createCollection(mainClName + "_sub_" + i);
            mainCL.attachCollection(cl.getFullName(), (BSONObject) JSON
                    .parse("{LowBound:{sk:" + bound + "},UpBound:{sk:" + (bound + 100) + "}}"));
            bound += 100;
            subClNames.add(cl.getName());
        }
    }

    @Test
    public void test() throws Exception {
        // 磁盘满
        GroupWrapper cataGroup = groupMgr.getGroupByName("SYSCatalogGroup");
        System.out.println(cataSlave.hostName());
        DiskFull df = new DiskFull(cataSlave.hostName(), SdbTestBase.workDir);
        df.init();
        df.make();

        // 分别以每条记录1m，512K,1k的大小填充SYSCAT.SYSCOLLECTIONS至-11错误
        fillUpCatalogSYSCL("pad_M", pad_M);
        fillUpCatalogSYSCL("pad_HM", pad_HM);
        fillUpCatalogSYSCL("pad_K", pad_K);

        // 启动attach的线程，及填充SYSCAT.SYSCOLLECTIONS的线程（每条记录512字节）
        Detach attach = new Detach("detach");
        InsertToCataCL insert = new InsertToCataCL("insert");
        insert.start();
        attach.start();
        insert.join();
        attach.join();

        // 磁盘恢复
        df.restore();
        df.fini();

        Assert.assertEquals(attach.isSuccess(), true, attach.getErrorMsg());
        Assert.assertEquals(insert.isSuccess(), true, insert.getErrorMsg());

        // 检测CATALOG组数据一致
        Utils.checkBusinessLSNWithTimeout(groupMgr, 60 * 20);
        CheckCatalog(cataGroup, mainClName);
        for (String subName : subClNames) {
            CheckCatalog(cataGroup, subName);
        }

        // 向子表插入数据，查询
        insertSubCLAndQuery();

        clearFlag = true;
    }

    private void insertSubCLAndQuery() {
        for (String subClName : subClNames) {
            // insert
            DBCollection cl = commCS.getCollection(subClName);
            for (int i = 0; i < 10; i++) {
                cl.insert("{sk:" + i + "}");
            }
            // query
            DBCursor cusor = null;
            try {
                cusor = cl.query(null, "{sk:1}", "{sk:1}", null);
                int count = 0;
                while (cusor.hasNext()) {
                    Assert.assertEquals(cusor.getNext(),
                            (BSONObject) JSON.parse("{sk:" + count + "}"));
                    count++;
                }
                Assert.assertEquals(count, 10);
            }
            finally {
                if (cusor != null) {
                    cusor.close();
                }
            }
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
        Sequoiadb cataDb = null;
        try {
            if (clearFlag) {
                for (String subClName : subClNames) {
                    commCS.dropCollection(subClName);
                }
                commCS.dropCollection(mainClName);
            }
            cataDb = new Sequoiadb(cataSlave.hostName() + ":" + cataSlave.svcName(), "", "");
            DBCollection catacl = cataDb.getCollectionSpace("SYSCAT")
                    .getCollection("SYSCOLLECTIONS");
            catacl.delete("{deleteFlag:1}");
        }
        catch (BaseException e) {
            Assert.fail(e.getMessage() + "\r\n" + Utils.getStackString(e));
        }
        finally {
            if (commSdb != null) {
                commSdb.disconnect();
            }
            if (cataDb != null) {
                cataDb.disconnect();
            }
            System.out.println(
                    "the TestCase Name:" + this.getClass().getName() + ". the TestCase end at:"
                            + new SimpleDateFormat("YYYY-MM-dd HH:mm:ss.SSS").format(new Date()));
        }
    }

    class Detach extends OperateTask {

        public Detach(String name) {
            super(name);
        }

        @Override
        public void exec() throws Exception {
            Sequoiadb sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "");
            try {
                for (String name : subClNames) {
                    mainCL.detachCollection(csName + "." + name);
                }
            }
            finally {
                if (sdb != null) {
                    sdb.disconnect();
                }
            }
        }

        @Override
        public void faultNotify(BSONObject status) throws FaultException {

        }
    }

    class InsertToCataCL extends OperateTask {

        public InsertToCataCL(String name) {
            super(name);
        }

        @Override
        public void faultNotify(BSONObject status) throws FaultException {
            // TODO Auto-generated method stub
        }

        @Override
        public void exec() throws Exception {
            fillUpCatalogSYSCL("PAD", pad_HK);
        }

    }

}
