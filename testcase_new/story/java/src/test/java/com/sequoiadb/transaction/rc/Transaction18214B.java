package com.sequoiadb.transaction.rc;

import java.util.ArrayList;
import java.util.List;

import org.bson.BSONObject;
import org.bson.types.ObjectId;
import org.bson.util.JSON;
import org.testng.Assert;
import org.testng.SkipException;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import com.sequoiadb.base.CollectionSpace;
import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.DBCursor;
import com.sequoiadb.base.DBLob;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.testcommon.CommLib;
import com.sequoiadb.testcommon.SdbTestBase;
import com.sequoiadb.testcommon.SdbThreadBase;
import com.sequoiadb.transaction.TransUtils;

/**
 * @testcase seqDB-18214:lob操作与事务操作并发
 * @date 2019-4-11
 * @author yinzhen
 *
 */
@Test(groups = "rc")
public class Transaction18214B extends SdbTestBase {
    private Sequoiadb sdb = null;
    private String clName = "cl18214B";
    private DBCollection cl = null;
    private Sequoiadb db1 = null;
    private Sequoiadb db2 = null;
    private DBCollection cl1 = null;
    private List<String> groupNames;

    @BeforeClass
    public void setUp() {
        sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "");
        if (CommLib.isStandAlone(sdb)) {
            throw new SkipException("STANDALONE MODE");
        }
        groupNames = CommLib.getDataGroupNames(sdb);
        if (2 > groupNames.size()) {
            throw new SkipException("groups less than 2");
        }

        cl = sdb.getCollectionSpace(csName).createCollection(clName,
                (BSONObject) JSON.parse("{Group:'" + groupNames.get(0) + "', ShardingKey:{b:1}, ShardingType:'hash'}"));
        cl.createIndex("idx18214", "{a:1}", false, false);
        cl.insert((BSONObject) JSON.parse("{_id:1, a:1, b:1}"));
        cl.insert((BSONObject) JSON.parse("{_id:2, a:2, b:2}"));
        cl.split(groupNames.get(0), groupNames.get(1), 50);
    }

    @AfterClass
    public void tearDown() {
        if (!db1.isClosed()) {
            db1.close();
        }
        if (!db2.isClosed()) {
            db2.close();
        }
        CollectionSpace cs = sdb.getCollectionSpace(csName);
        if (cs.isCollectionExist(clName)) {
            cs.dropCollection(clName);
        }
        if (!sdb.isClosed()) {
            sdb.close();
        }
    }

    @Test
    public void test() {
        try {
            // 开启并发事务
            db1 = new Sequoiadb(SdbTestBase.coordUrl, "", "");
            db2 = new Sequoiadb(SdbTestBase.coordUrl, "", "");
            db1.beginTransaction();
            cl1 = db1.getCollectionSpace(csName).getCollection(clName);

            OperatorTh operatorTh = new OperatorTh();
            operatorTh.start();

            DropCLTh dropCLTh = new DropCLTh();
            dropCLTh.start();

            Assert.assertTrue(operatorTh.isSuccess(), operatorTh.getErrorMsg());
            Assert.assertTrue(dropCLTh.isSuccess(), dropCLTh.getErrorMsg());

            db1.commit();
        } finally {
            db1.commit();
        }
    }

    private class OperatorTh extends SdbThreadBase {

        @Override
        public void exec() throws Exception {
            cl1.insert((BSONObject) JSON.parse("{_id:3, a:3, b:3}"));
            cl1.delete("{a:1}", "{'':'idx18214'}");
            cl1.update("{a:2}", "{$set:{a:10}}", "{}'':'idx18214'");

            List<BSONObject> expList = new ArrayList<>();
            expList.add((BSONObject) JSON.parse("{_id:2, a:10, b:2}"));
            expList.add((BSONObject) JSON.parse("{_id:3, a:3, b:3}"));

            DBCursor cursor = cl1.query();
            List<BSONObject> actList = TransUtils.getReadActList(cursor);
            Assert.assertEquals(actList, expList);
        }
    }

    // 在事务内删除集合
    private class DropCLTh extends SdbThreadBase {

        @Override
        public void exec() throws Exception {
            Thread.sleep(1000);
            DBCollection cl = db2.getCollectionSpace(csName).getCollection(clName);
            DBLob lob = cl.createLob();
            lob.write("test lob to test".getBytes());
            lob.close();
            ObjectId oid = new ObjectId();
            DBLob lob2 = cl.createLob(oid);
            lob2.close();
            cl.removeLob(oid);
            cl.truncateLob(lob.getID(), 12);
        }
    }
}
