package com.sequoiadb.fulltextparallel;

import org.elasticsearch.client.Client;
import org.testng.Assert;
import org.testng.SkipException;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import com.sequoiadb.base.CollectionSpace;
import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.testcommon.CommLib;
import com.sequoiadb.testcommon.SdbTestBase;
import com.sequoiadb.threadexecutor.ThreadExecutor;
import com.sequoiadb.threadexecutor.annotation.ExecuteOrder;
import com.sequoiadb.utils.FullTextDBUtils;
import com.sequoiadb.utils.FullTextESUtils;
import com.sequoiadb.utils.FullTextUtils;

/**
 * @FileName seqDB-15826:删除全文索引与创建/删除普通索引并发
 * @Author yinzhen
 * @Date 2019-4-30
 */
public class FullText15826 extends SdbTestBase {
    private String clName = "cl15826";
    private Sequoiadb sdb;
    private DBCollection cl;
    private String fullIdxName = "idx15826";
    private Client esClient;
    private String cappedCLName;
    private String esIndexName;
    private int insertNum = 20000;

    @BeforeClass
    public void setUp() throws Exception {
        sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "");
        if (CommLib.isStandAlone(sdb)) {
            throw new SkipException("STANDALONE MODE");
        }

        esClient = FullTextESUtils.createTransportClient(SdbTestBase.esHostName,
                Integer.parseInt(SdbTestBase.esServiceName));
        cl = sdb.getCollectionSpace(csName).createCollection(clName);
        FullTextDBUtils.insertData(cl, insertNum);

        // 创建索引
        cl.createIndex(fullIdxName, "{'a':'text','b':'text','c':'text'}", false, false);
        cl.createIndex("idx1", "{'a':1, 'b':1}", false, false);
        cl.createIndex("idx2", "{'e':1, 'f':1}", false, false);
        FullTextUtils.isIndexCreated(esClient, cl, fullIdxName, insertNum);

        esIndexName = FullTextDBUtils.getESIndexName(cl, fullIdxName);
        cappedCLName = FullTextDBUtils.getCappedName(cl, fullIdxName);
    }

    @Test
    public void test() throws Exception {
        ThreadExecutor thExecutor = new ThreadExecutor(600000);
        thExecutor.addWorker(new DropFullIdx());
        thExecutor.addWorker(new CreateIdx("idx3", "{'d':1, 'f':1}"));
        thExecutor.addWorker(new CreateIdx("idx4", "{'b':1, 'c':1}"));
        thExecutor.addWorker(new DropIdx("idx1"));
        thExecutor.addWorker(new DropIdx("idx2"));

        thExecutor.run();

        // 主备节点上索引信息及固定集合信息一致，ES同步的索引数据正确
        Assert.assertTrue(FullTextUtils.isIndexDeleted(sdb, esClient, esIndexName, cappedCLName));
        Assert.assertTrue(FullTextUtils.isCLDataConsistency(cl));
    }

    @AfterClass
    public void tearDown() throws Exception {
        try {
            CollectionSpace cs = sdb.getCollectionSpace(csName);
            FullTextDBUtils.dropCollection(cs, clName);
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

    private class DropFullIdx {
        @ExecuteOrder(step = 1, desc = "删除全文索引")
        private void dropFullIdx() {
            Sequoiadb db = null;
            try {
                db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
                DBCollection cl = db.getCollectionSpace(csName).getCollection(clName);
                cl.dropIndex(fullIdxName);
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
