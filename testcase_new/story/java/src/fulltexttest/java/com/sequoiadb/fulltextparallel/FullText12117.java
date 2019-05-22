package com.sequoiadb.fulltextparallel;

import java.util.List;

import org.bson.BSONObject;
import org.bson.util.JSON;
import org.elasticsearch.client.Client;
import org.testng.Assert;
import org.testng.SkipException;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import com.sequoiadb.base.CollectionSpace;
import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.DBCursor;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.testcommon.CommLib;
import com.sequoiadb.testcommon.SdbTestBase;
import com.sequoiadb.threadexecutor.ThreadExecutor;
import com.sequoiadb.threadexecutor.annotation.ExecuteOrder;
import com.sequoiadb.utils.FullTextDBUtils;
import com.sequoiadb.utils.FullTextESUtils;
import com.sequoiadb.utils.FullTextUtils;

/**
 * @testcase seqDB-12117:并发删除同一条记录
 * @date 2019-4-28
 * @author yinzhen
 *
 */
public class FullText12117 extends SdbTestBase {
    private static final String CLNAME = "cl12117";
    private ThreadExecutor thExecutor = new ThreadExecutor(600000);
    private Sequoiadb sdb;
    private DBCollection cl;
    private String fullIdxName = "idx12117";
    private Client esClient;
    private String groupName;
    private String esIndexName;
    private String cappedCLName;

    @BeforeClass
    public void setUp() {
        sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "");
        if (CommLib.isStandAlone(sdb)) {
            throw new SkipException("STANDALONE MODE");
        }

        esClient = FullTextESUtils.createTransportClient(SdbTestBase.esHostName,
                Integer.parseInt(SdbTestBase.esServiceName));
        groupName = CommLib.getDataGroupNames(sdb).get(0);
        cl = sdb.getCollectionSpace(csName).createCollection(CLNAME,
                (BSONObject) JSON.parse("{Group:'" + groupName + "'}"));
        FullTextDBUtils.insertData(cl, FullTextUtils.INSERT_NUMS);
    }

    @Test
    public void test() throws Exception {
        thExecutor.addWorker(new CreateFullIdxTh1());
        thExecutor.addWorker(new CreateFullIdxTh2());
        thExecutor.addWorker(new CreateFullIdxTh3());

        thExecutor.run();
        thExecutor.display();
    }

    @AfterClass
    public void tearDown() {
        try {
            CollectionSpace cs = sdb.getCollectionSpace(csName);
            cs.dropCollection(CLNAME);
            Assert.assertTrue(FullTextUtils.isIndexDeleted(sdb, esClient, esIndexName, cappedCLName));
        } finally {
            sdb.close();
        }
    }

    class CreateFullIdxTh1 {
        private Sequoiadb db;
        private Sequoiadb db2;
        private DBCollection cl;
        private DBCollection cl2;

        private CreateFullIdxTh1() {
            db = new Sequoiadb(coordUrl, "", "");
            db2 = new Sequoiadb(coordUrl, "", "");
            cl = db.getCollectionSpace(csName).getCollection(CLNAME);
            cl2 = db2.getCollectionSpace(csName).getCollection(CLNAME);
        }

        @ExecuteOrder(step = 1, desc = "多线程并发创建不同的全文索引")
        private void createFullIdx() {
            try {
                cl.createIndex("idx12117_1", "{'a':'text','b':'text','c':'text', 'd':'text', 'e':'text', 'f':'text'}",
                        false, false);
                fullIdxName = "idx12117_1";
            } catch (BaseException e) {
                Assert.assertEquals(e.getErrorCode(), -42);
            }
        }

        @ExecuteOrder(step = 2, desc = "主备节点上索引信息及固定集合中数据正确且一致，ES上同步的索引数据正确，全文检索结果正确")
        private void checkRecords() throws Exception {
            try {
                esIndexName = FullTextDBUtils.getESIndexName(cl, fullIdxName);
                cappedCLName = FullTextDBUtils.getCappedName(cl, fullIdxName);
                Assert.assertTrue(FullTextUtils.isIndexCreated(esClient, cl, fullIdxName, FullTextUtils.INSERT_NUMS));
                checkFullIdx();

                DBCursor dbCursor = cl.query("{}", "{}", "{_id:1}", "{}");
                DBCursor esCursor = cl2.query("{'':{'$Text':{'query':{'match_all':{}}}}}", "{}", "{_id:1}",
                        "{'':'" + fullIdxName + "'}");
                Assert.assertTrue(FullTextUtils.isCLRecordsConsistency(dbCursor, esCursor));
            } finally {
                db.closeAllCursors();
                db2.closeAllCursors();
                db.close();
                db2.close();
            }
        }
    }

    class CreateFullIdxTh2 {
        private Sequoiadb db;
        private DBCollection cl;

        private CreateFullIdxTh2() {
            db = new Sequoiadb(coordUrl, "", "");
            cl = db.getCollectionSpace(csName).getCollection(CLNAME);
        }

        @ExecuteOrder(step = 1, desc = "多线程并发创建不同的全文索引")
        private void createFullIdx() {
            try {
                cl.createIndex("idx12117_2", "{'a':'text','b':'text','c':'text', 'd':'text', 'e':'text', 'f':'text'}",
                        false, false);
                fullIdxName = "idx12117_2";
            } catch (BaseException e) {
                Assert.assertEquals(e.getErrorCode(), -42);
            } finally {
                db.close();
            }
        }
    }

    class CreateFullIdxTh3 {
        private Sequoiadb db;
        private DBCollection cl;

        private CreateFullIdxTh3() {
            db = new Sequoiadb(coordUrl, "", "");
            cl = db.getCollectionSpace(csName).getCollection(CLNAME);
        }

        @ExecuteOrder(step = 1, desc = "多线程并发创建不同的全文索引")
        private void createFullIdx() {
            try {
                cl.createIndex("idx12117_3", "{'a':'text','b':'text','c':'text', 'd':'text', 'e':'text', 'f':'text'}",
                        false, false);
                fullIdxName = "idx12117_3";
            } catch (BaseException e) {
                Assert.assertEquals(e.getErrorCode(), -42);
            } finally {
                db.close();
            }
        }
    }

    private void checkFullIdx() {
        List<String> nodeAddrs = CommLib.getNodeAddress(sdb, groupName);
        for (String nodeAddr : nodeAddrs) {
            Sequoiadb data = null;
            try {
                data = new Sequoiadb(nodeAddr, "", "");
                DBCollection cl = data.getCollectionSpace(csName).getCollection(CLNAME);
                Assert.assertTrue(cl.isIndexExist(fullIdxName));
            } finally {
                data.close();
            }
        }
    }
}