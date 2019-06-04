package com.sequoiadb.fulltextparallel;

import java.util.concurrent.atomic.AtomicInteger;

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
 * @FileName seqDB-12128:存在全文索引，并发删除同一集合
 * @Author yinzhen
 * @Date 2019-4-30
 */
public class FullText12128 extends SdbTestBase {
    private String clName = "cl12128";
    private Sequoiadb sdb;
    private DBCollection cl;
    private String fullIdxName = "idx12128";
    private String cappedCLName;
    private String esIndexName;
    private Client esClient;
    private AtomicInteger atoint = new AtomicInteger(0);
    private int insertNum = 20000;

    @BeforeClass
    public void setUp() {
        sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "");
        if (CommLib.isStandAlone(sdb)) {
            throw new SkipException("STANDALONE MODE");
        }

        esClient = FullTextESUtils.createTransportClient(SdbTestBase.esHostName,
                Integer.parseInt(SdbTestBase.esServiceName));
        cl = sdb.getCollectionSpace(csName).createCollection(clName);
        cl.createIndex(fullIdxName, "{'a':'text','b':'text','c':'text', 'd':'text', 'e':'text', 'f':'text'}", false,
                false);

        esIndexName = FullTextDBUtils.getESIndexName(cl, fullIdxName);
        cappedCLName = FullTextDBUtils.getCappedName(cl, fullIdxName);
        FullTextDBUtils.insertData(cl, insertNum);
    }

    @Test
    public void test() throws Exception {
        ThreadExecutor thExecutor = new ThreadExecutor(600000);
        for (int i = 0; i < 10; i++) {
            thExecutor.addWorker(new DropCL());
        }
        thExecutor.run();
        Assert.assertEquals(atoint.get(), 1);

        // 集合删除成功
        Assert.assertTrue(FullTextUtils.isIndexDeleted(sdb, esClient, esIndexName, cappedCLName));
        Assert.assertFalse(sdb.getCollectionSpace(csName).isCollectionExist(clName));
    }

    @AfterClass
    public void tearDown() throws Exception {
        try {
            CollectionSpace cs = sdb.getCollectionSpace(csName);
            if (cs.isCollectionExist(clName)) {
                FullTextDBUtils.dropCollection(cs, clName);
            }
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

    private class DropCL {
        @ExecuteOrder(step = 1, desc = "并发删除该集合")
        private void dropFullIdx() {
            Sequoiadb db = null;
            try {
                db = new Sequoiadb(coordUrl, "", "");
                db.getCollectionSpace(csName).dropCollection(clName);
                atoint.incrementAndGet();
            } catch (BaseException e) {
                if (e.getErrorCode() != -23 && e.getErrorCode() != -147) {
                    throw e;
                }
            } finally {
                if (db != null) {
                    db.close();
                }
            }
        }
    }
}