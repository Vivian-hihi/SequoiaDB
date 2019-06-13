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
import com.sequoiadb.base.DBCursor;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.commlib.CommLib;
import com.sequoiadb.commlib.GroupMgr;
import com.sequoiadb.commlib.NodeWrapper;
import com.sequoiadb.commlib.SdbTestBase;
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.exception.ReliabilityException;
import com.sequoiadb.fulltext.FullTextDBUtils;
import com.sequoiadb.fulltext.FullTextUtils;

public class Update12010 extends SdbTestBase {
    private String clName = "ES_cl_update12010";
    private CollectionSpace cs;
    private DBCollection cl;
    private GroupMgr groupMgr = null;
    private Sequoiadb sdb = null;
    private String groupName;
    private String fullIndexName = "fullIndex_update12010";
    private int insertNum = 5000;
    private List<BSONObject> actRecords = new ArrayList<BSONObject>();
    private List<BSONObject> expectRecords = new ArrayList<BSONObject>();
    private NodeWrapper cLGroupSlave = null;

    @BeforeClass()
    public void setUp() {
        try {
            sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "");
            if (CommLib.isStandAlone(sdb)) {
                throw new SkipException("StandAlone environment!");
            }
            System.out.println("the TestCase Name:" + getClass().getName() + ". the TestCase begin at:"
                    + new SimpleDateFormat("YYYY-MM-dd HH:mm:ss.SSS").format(new Date()));
            groupMgr = GroupMgr.getInstance();

            // CheckBusiness(true),检测当前集群环境，若存在异常返回false，
            if (!groupMgr.checkBusiness(20)) {
                throw new SkipException("checkBusiness return false");
            }
            groupName = groupMgr.getAllDataGroupName().get(0);
            cs = sdb.getCollectionSpace(csName);
            cl = cs.createCollection(clName, (BSONObject) JSON.parse("{Group:'" + groupName + "', ReplSize : 7}"));
        } catch (ReliabilityException e) {
            if (sdb != null) {
                sdb.close();
            }
            Assert.fail(this.getClass().getName() + " setUp error, error description:" + e.getMessage());
        }
    }

    @Test()
    public void test() throws Exception {
        Sequoiadb db = null;
        try {
            db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
            cl.createIndex(fullIndexName, "{'a':'text'}", false, false);
            List<BSONObject> insertRecords = new ArrayList<BSONObject>();
            for (int i = 0; i < insertNum; i++) {
                BSONObject record = (BSONObject) JSON.parse("{_id:" + i + ", a:'a" + i + "'}");
                insertRecords.add(record);
            }
            cl.insert(insertRecords);
            Assert.assertTrue(FullTextUtils.isIndexCreated(cl, fullIndexName, insertNum));

            // update records
            cLGroupSlave = groupMgr.getGroupByName(groupName).getSlave();
            cLGroupSlave.stop();
            boolean updateFlag = update();
            cLGroupSlave.start();
            Assert.assertEquals(groupMgr.checkBusiness(600), true);

            if (updateFlag == false) {
                expectRecords.addAll(insertRecords);
            } else {
                for (int i = 0; i < insertNum; i++) {
                    BSONObject record = (BSONObject) JSON.parse("{_id:" + i + ", a:'a'}");
                    expectRecords.add(record);
                }
            }
            Assert.assertTrue(FullTextUtils.isIndexCreated(cl, fullIndexName, insertNum));
            DBCursor cur = cl.query((BSONObject) JSON.parse("{'':{$Text:{query:{match_all:{}}}}}"), null,
                    (BSONObject) JSON.parse("{_id : 1}"), null, 0);
            actRecords = FullTextDBUtils.getRecordsFromCL(cur);
            checkRecords(expectRecords, actRecords);
        } catch (ReliabilityException e) {
            e.printStackTrace();
            Assert.fail(e.getMessage());
        } finally {
            db.closeAllCursors();
            db.close();
        }

    }

    public boolean update() {
        boolean flag = false;
        Sequoiadb db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
        try {
            DBCollection cl = db.getCollectionSpace(csName).getCollection(clName);
            cl.update(null, "{$set: {a : 'a'}}", "{'':'a'}");
            Assert.fail("update error!");
        } catch (BaseException e) {
            System.out.println("errorCode=========" + e.getErrorCode());
            if (e.getErrorCode() == -252) {
                flag = true;
            } else if (e.getErrorCode() == -105) {
                flag = false;
            } else {
                Assert.fail("error code is wrong!");
            }
        } finally {
            if (db != null) {
                db.close();
            }
        }
        return flag;
    }

    public void checkRecords(List<BSONObject> expRecords, List<BSONObject> actRecords) {
        if (expRecords.size() != actRecords.size()) {
            Assert.fail("records count has error!");
        }

        for (int i = 0; i < expRecords.size(); i++) {
            if (!expRecords.get(i).get("a").equals(actRecords.get(i).get("a"))) {
                Assert.fail("records has error!");
            }
        }
    }

    @AfterClass()
    public void tearDown() {
        Sequoiadb db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
        try {
            CollectionSpace commCS = db.getCollectionSpace(csName);
            commCS.dropCollection(clName);
        } catch (BaseException e) {
            Assert.fail(e.getMessage());
        } finally {
            if (db != null) {
                db.close();
            }
            System.out.println("the TestCase Name:" + this.getClass().getName() + ". the TestCase end at:"
                    + new SimpleDateFormat("YYYY-MM-dd HH:mm:ss.SSS").format(new Date()));
        }
    }

}
