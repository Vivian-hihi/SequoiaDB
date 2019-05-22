package com.sequoiadb.fulltextparallel;

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
import com.sequoiadb.testcommon.CommLib;
import com.sequoiadb.testcommon.SdbTestBase;
import com.sequoiadb.threadexecutor.ThreadExecutor;
import com.sequoiadb.threadexecutor.annotation.ExecuteOrder;
import com.sequoiadb.utils.FullTextDBUtils;
import com.sequoiadb.utils.FullTextESUtils;
import com.sequoiadb.utils.FullTextUtils;

/**
 * @testcase seqDB-14415:增删改查与truncate并发
 * @date 2019-4-22
 * @author yinzhen
 *
 */
public class FullText14415 extends SdbTestBase {
    private static final String CLNAME = "cl14415";
    private ThreadExecutor thExecutor = new ThreadExecutor(600000);
    private Sequoiadb sdb;
    private DBCollection cl;
    private String fullIdxName = "idx14415";
    private Client esClient;
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
        cl = sdb.getCollectionSpace(csName).createCollection(CLNAME);
        FullTextDBUtils.insertData(cl, FullTextUtils.INSERT_NUMS);
    }

    @Test
    public void test() throws Exception {
        thExecutor.addWorker(new CreateFullIdx());
        thExecutor.addWorker(new CreateIdx());

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

    class CreateFullIdx {
        private Sequoiadb db;
        private DBCollection cl;

        private CreateFullIdx() {
            db = new Sequoiadb(coordUrl, "", "");
            cl = db.getCollectionSpace(csName).getCollection(CLNAME);
        }

        @ExecuteOrder(step = 1, desc = "创建全文索引")
        private void createFullIdx() {
            cl.createIndex(fullIdxName, "{'a':'text','b':'text','c':'text'}", false, false);
            esIndexName = FullTextDBUtils.getESIndexName(cl, fullIdxName);
            cappedCLName = FullTextDBUtils.getCappedName(cl, fullIdxName);
            Assert.assertTrue(cl.isIndexExist(fullIdxName));
        }

        @ExecuteOrder(step = 2, desc = "主备节点上索引信息及固定集合信息一致，ES同步的索引数据正确")
        private void checkRecords() throws Exception {
            try {
                Assert.assertTrue(FullTextUtils.isIndexCreated(esClient, cl, fullIdxName, FullTextUtils.INSERT_NUMS));
            } finally {
                db.close();
            }
        }
    }

    class CreateIdx {
        private Sequoiadb db;
        private Sequoiadb db2;
        private DBCollection cl;
        private DBCollection cl2;

        private CreateIdx() {
            db = new Sequoiadb(coordUrl, "", "");
            db2 = new Sequoiadb(coordUrl, "", "");
            cl = db.getCollectionSpace(csName).getCollection(CLNAME);
            cl2 = db2.getCollectionSpace(csName).getCollection(CLNAME);
        }

        @ExecuteOrder(step = 1, desc = "创建普通索引，删除普通索引")
        private void operatorIdx() {
            cl.createIndex("idx1", "{'d':1, 'e':1}", false, false);
            cl.createIndex("idx2", "{'c':1, 'd':1}", false, false);
            Assert.assertTrue(cl.isIndexExist("idx1"));
            Assert.assertTrue(cl.isIndexExist("idx2"));
            cl.dropIndex("idx1");
            cl.dropIndex("idx2");
            Assert.assertFalse(cl.isIndexExist("idx1"));
            Assert.assertFalse(cl.isIndexExist("idx2"));
        }

        @ExecuteOrder(step = 3, desc = "普通索引查询及全文检索")
        private void queryAndCheck() {
            try {
                DBCursor dbCursor = cl.query("{}", "{}", "{_id:1}", "{}");
                DBCursor esCursor = cl2.query("{'':{'$Text':{'query':{'match_all':{}}}}}", "{}", "{_id:1}",
                        "{'':'" + fullIdxName + "'}");
                Assert.assertTrue(FullTextUtils.isCLRecordsConsistency(dbCursor, esCursor));
            } finally {
                db.closeAllCursors();
                db.close();
            }
        }
    }
}