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
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.testcommon.CommLib;
import com.sequoiadb.testcommon.SdbTestBase;
import com.sequoiadb.threadexecutor.ThreadExecutor;
import com.sequoiadb.threadexecutor.annotation.ExecuteOrder;
import com.sequoiadb.utils.FullTextDBUtils;
import com.sequoiadb.utils.FullTextESUtils;
import com.sequoiadb.utils.FullTextUtils;

/**
 * @testcase seqDB-12128:存在全文索引，并发删除同一集合
 * @date 2019-4-30
 * @author yinzhen
 *
 */
public class FullText12128 extends SdbTestBase {
    private static final String CLNAME = "cl12128";
    private ThreadExecutor thExecutor = new ThreadExecutor(600000);
    private Sequoiadb sdb;
    private DBCollection cl;
    private String fullIdxName = "idx12128";
    private String cappedCLName;
    private String esIndexName;
    private Client esClient;

    @BeforeClass
    public void setUp() {
        sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "");
        if (CommLib.isStandAlone(sdb)) {
            throw new SkipException("STANDALONE MODE");
        }

        esClient = FullTextESUtils.createTransportClient(SdbTestBase.esHostName,
                Integer.parseInt(SdbTestBase.esServiceName));
        cl = sdb.getCollectionSpace(csName).createCollection(CLNAME);
        cl.createIndex(fullIdxName, "{'a':'text','b':'text','c':'text', 'd':'text', 'e':'text', 'f':'text'}", false,
                false);

        esIndexName = FullTextDBUtils.getESIndexName(cl, fullIdxName);
        cappedCLName = FullTextDBUtils.getCappedName(cl, fullIdxName);
        FullTextDBUtils.insertData(cl, FullTextUtils.INSERT_NUMS);
    }

    @Test
    public void test() throws Exception {
        thExecutor.addWorker(new DropCLTh1());
        thExecutor.addWorker(new DropCLTh2());
        thExecutor.addWorker(new DropCLTh2());

        thExecutor.run();
        thExecutor.display();
    }

    @AfterClass
    public void tearDown() {
        try {
            CollectionSpace cs = sdb.getCollectionSpace(csName);
            if (cs.isCollectionExist(CLNAME)) {
                cs.dropCollection(CLNAME);
            }
            Assert.assertTrue(FullTextUtils.isIndexDeleted(sdb, esClient, esIndexName, cappedCLName));
        } finally {
            sdb.close();
        }
    }

    class DropCLTh1 {
        private Sequoiadb db;

        private DropCLTh1() {
            db = new Sequoiadb(coordUrl, "", "");
        }

        @ExecuteOrder(step = 1, desc = "并发删除该集合")
        private void dropFullIdx() {
            try {
                db.getCollectionSpace(csName).dropCollection(CLNAME);
            } catch (BaseException e) {
                if (e.getErrorCode() != -23 && e.getErrorCode() != -147) {
                    Assert.fail(e.getMessage());
                }
            }
        }

        @ExecuteOrder(step = 2, desc = "集合删除成功")
        private void checkIndex() throws InterruptedException {
            try {
                Assert.assertFalse(db.getCollectionSpace(csName).isCollectionExist(CLNAME));
            } finally {
                db.close();
            }
        }
    }

    class DropCLTh2 {
        private Sequoiadb db;

        private DropCLTh2() {
            db = new Sequoiadb(coordUrl, "", "");
        }

        @ExecuteOrder(step = 1, desc = "并发删除该集合")
        private void dropFullIdx() {
            try {
                db.getCollectionSpace(csName).dropCollection(CLNAME);
            } catch (BaseException e) {
                if (e.getErrorCode() != -23 && e.getErrorCode() != -147) {
                    Assert.fail(e.getMessage());
                }
            } finally {
                db.close();
            }
        }
    }
}