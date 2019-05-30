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
 * @FileName seqDB-14415:创建全文索引与创建/删除普通索引并发
 * @Author yinzhen
 * @Date 2019-4-22
 */
public class FullText14415 extends SdbTestBase {
    private String clName = "cl14415";
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
        cl = sdb.getCollectionSpace(csName).createCollection(clName);
        FullTextDBUtils.insertData(cl, 20000);
        cl.createIndex("idx1", "{'d':1, 'e':1}", false, false);
        cl.createIndex("idx2", "{'a':1, 'b':1}", false, false);
    }

    @Test
    public void test() throws Exception {
        ThreadExecutor thExecutor = new ThreadExecutor(600000);
        thExecutor.addWorker(new CreateFullIdx());
        thExecutor.addWorker(new CreateIdx("idx3", "{'d':1, 'f':1}"));
        thExecutor.addWorker(new CreateIdx("idx4", "{'b':1, 'c':1}"));
        thExecutor.addWorker(new DropIdx("idx1"));
        thExecutor.addWorker(new DropIdx("idx2"));

        thExecutor.run();

        // 主备节点上索引信息及固定集合信息一致，ES同步的索引数据正确
        Assert.assertTrue(FullTextUtils.isIndexCreated(esClient, cl, fullIdxName, 20000));

        // 普通索引查询及全文检索
        Sequoiadb db2 = new Sequoiadb(SdbTestBase.coordUrl, "", "");
        try {
            DBCollection cl2 = db2.getCollectionSpace(csName).getCollection(clName);
            DBCursor dbCursor = cl.query("{}", "{}", "{_id:1}", "{}");
            DBCursor esCursor = cl2.query("{'':{'$Text':{'query':{'match_all':{}}}}}", "{}", "{_id:1}",
                    "{'':'" + fullIdxName + "'}");
            Assert.assertTrue(FullTextUtils.isCLRecordsConsistency(dbCursor, esCursor));
        } finally {
            if (db2 != null) {
                db2.close();
            }
        }

        // 在db端执行插入、全文检索
        Sequoiadb db3 = new Sequoiadb(SdbTestBase.coordUrl, "", "");
        try {
            FullTextDBUtils.insertData(cl, 1000);
            Assert.assertEquals(cl.getCount(), 21000);
            Assert.assertTrue(FullTextUtils.isIndexCreated(esClient, cl, fullIdxName, 21000));

            DBCollection cl3 = db3.getCollectionSpace(csName).getCollection(clName);
            DBCursor dbCursor = cl.query("{}", "{}", "{_id:1}", "{}");
            DBCursor esCursor = cl3.query("{'':{'$Text':{'query':{'match_all':{}}}}}", "{}", "{_id:1}",
                    "{'':'" + fullIdxName + "'}");
            Assert.assertTrue(FullTextUtils.isCLRecordsConsistency(dbCursor, esCursor));
        } finally {
            if (db3 != null) {
                db3.close();
            }
        }
    }

    @AfterClass
    public void tearDown() throws Exception {
        try {
            CollectionSpace cs = sdb.getCollectionSpace(csName);
            cs.dropCollection(clName);
            Assert.assertTrue(FullTextUtils.isIndexDeleted(sdb, esClient, esIndexName, cappedCLName));
        } finally {
            if (sdb != null) {
                sdb.close();
            }
            if (esClient != null) {
                esClient.close();
            }
        }
    }

    private class CreateFullIdx {
        @ExecuteOrder(step = 1, desc = "创建全文索引")
        private void createFullIdx() {
            Sequoiadb db = null;
            try {
                db = new Sequoiadb(coordUrl, "", "");
                DBCollection cl = db.getCollectionSpace(csName).getCollection(clName);
                cl.createIndex(fullIdxName, "{'a':'text','b':'text','c':'text'}", false, false);
                esIndexName = FullTextDBUtils.getESIndexName(cl, fullIdxName);
                cappedCLName = FullTextDBUtils.getCappedName(cl, fullIdxName);
            } finally {
                if (db != null) {
                    db.close();
                }
            }
        }
    }

    private class CreateIdx {
        private String idxName;
        private String option;

        private CreateIdx(String idxName, String option) {
            this.idxName = idxName;
            this.option = option;
        }

        @ExecuteOrder(step = 1, desc = "创建普通索引，删除普通索引")
        private void createIdx() {
            Sequoiadb db = null;
            try {
                db = new Sequoiadb(coordUrl, "", "");
                DBCollection cl = db.getCollectionSpace(csName).getCollection(clName);
                Assert.assertFalse(cl.isIndexExist(idxName));
                cl.createIndex(idxName, option, false, false);
                Assert.assertTrue(cl.isIndexExist(idxName));
            } finally {
                if (db != null) {
                    db.close();
                }
            }
        }
    }

    private class DropIdx {
        private String idxName;

        private DropIdx(String idxName) {
            this.idxName = idxName;
        }

        @ExecuteOrder(step = 1, desc = "创建普通索引，删除普通索引")
        private void dropIdx() {
            Sequoiadb db = null;
            try {
                db = new Sequoiadb(coordUrl, "", "");
                DBCollection cl = db.getCollectionSpace(csName).getCollection(clName);
                Assert.assertTrue(cl.isIndexExist(idxName));
                cl.dropIndex(idxName);
                Assert.assertFalse(cl.isIndexExist(idxName));
            } finally {
                if (db != null) {
                    db.close();
                }
            }
        }
    }
}