package com.sequoiadb.transaction.rc;

import java.util.ArrayList;
import java.util.List;

import org.bson.BSONObject;
import org.bson.util.JSON;
import org.testng.Assert;
import org.testng.SkipException;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.DBCursor;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.testcommon.CommLib;
import com.sequoiadb.testcommon.SdbTestBase;
import com.sequoiadb.testcommon.SdbThreadBase;
import com.sequoiadb.transaction.TransUtils;

/**
 * @testcase seqDB-18212:删除集合空间与事务操作并发
 * @date 2019-4-11
 * @author yinzhen
 *
 */
@Test(groups = "rc")
public class Transaction18212B extends SdbTestBase {
    private Sequoiadb sdb = null;
    private String csName = "cs18212B";
    private Sequoiadb db1 = null;
    private Sequoiadb db2 = null;
    private Sequoiadb db3 = null;
    private Sequoiadb db4 = null;
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

        sdb.createCollectionSpace(csName);
        createCL("cl18212A");
        createCL("cl18212B");
        createCL("cl18212C");
    }

    @AfterClass
    public void tearDown() {
        try {
            sdb.dropCollectionSpace(csName);
        } finally {
            sdb.close();
            db1.close();
            db2.close();
            db3.close();
            db4.close();
        }
    }

    @Test
    public void test() {
        try {
            // 开启并发事务
            db1 = new Sequoiadb(SdbTestBase.coordUrl, "", "");
            db2 = new Sequoiadb(SdbTestBase.coordUrl, "", "");
            db3 = new Sequoiadb(SdbTestBase.coordUrl, "", "");
            db4 = new Sequoiadb(SdbTestBase.coordUrl, "", "");
            db1.beginTransaction();
            db2.beginTransaction();
            db3.beginTransaction();

            OperatorTh operatorTh1 = new OperatorTh(db1, "cl18212A");
            operatorTh1.start();

            OperatorTh operatorTh2 = new OperatorTh(db2, "cl18212B");
            operatorTh2.start();

            OperatorTh operatorTh3 = new OperatorTh(db3, "cl18212C");
            operatorTh3.start();

            DropCLTh dropCLTh = new DropCLTh();
            dropCLTh.start();

            Assert.assertTrue(operatorTh1.isSuccess(), operatorTh1.getErrorMsg());
            Assert.assertTrue(operatorTh2.isSuccess(), operatorTh2.getErrorMsg());
            Assert.assertTrue(operatorTh3.isSuccess(), operatorTh3.getErrorMsg());
            Assert.assertTrue(dropCLTh.isSuccess(), dropCLTh.getErrorMsg());

            db1.commit();
            db2.commit();
            db3.commit();
        } finally {
            db1.commit();
            db2.commit();
            db3.commit();
        }
    }

    private void createCL(String clName) {
        DBCollection cl = sdb.getCollectionSpace(csName).createCollection(clName, (BSONObject) JSON
                .parse("{Group:'" + groupNames.get(0) + "', ShardingKey:{b:1}, ShardingType:'range'}"));
        cl.createIndex("idx18212", "{a:1}", false, false);
        cl.insert((BSONObject) JSON.parse("{_id:1, a:1, b:1}"));
        cl.insert((BSONObject) JSON.parse("{_id:2, a:2, b:2}"));
        cl.split(groupNames.get(0), groupNames.get(1), 50);
    }

    private class OperatorTh extends SdbThreadBase {
        private String clName;
        private Sequoiadb db;

        private OperatorTh(Sequoiadb db, String clName) {
            this.clName = clName;
            this.db = db;
        }

        @Override
        public void exec() throws Exception {
            DBCollection cl = db.getCollectionSpace(csName).getCollection(clName);
            cl.insert((BSONObject) JSON.parse("{_id:3, a:3, b:3}"));
            cl.delete("{a:1}", "{'':'idx18212'}");
            cl.update("{a:2}", "{$set:{a:10}}", "{}'':'idx18212'");

            List<BSONObject> expList = new ArrayList<>();
            expList.add((BSONObject) JSON.parse("{_id:2, a:10, b:2}"));
            expList.add((BSONObject) JSON.parse("{_id:3, a:3, b:3}"));

            DBCursor cursor = cl.query();
            List<BSONObject> actList = TransUtils.getReadActList(cursor);
            Assert.assertEquals(actList, expList);
        }
    }

    // 在事务内删除集合
    private class DropCLTh extends SdbThreadBase {

        @Override
        public void exec() throws Exception {
            try {
                Thread.sleep(2000);
                db4.dropCollectionSpace(csName);
                throw new BaseException(-999, "DROP CS ERROR");
            } catch (BaseException e) {
                Assert.assertEquals(e.getErrorCode(), -190, e.getMessage());
            }
        }
    }
}
