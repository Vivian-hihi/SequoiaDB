package com.sequoiadb.fulltextparallel;

import java.util.Date;
import java.util.Random;

import org.bson.BSONObject;
import org.bson.BasicBSONObject;
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
import com.sequoiadb.threadexecutor.ResultStore;
import com.sequoiadb.threadexecutor.ThreadExecutor;
import com.sequoiadb.threadexecutor.annotation.ExecuteOrder;
import com.sequoiadb.utils.FullTextDBUtils;
import com.sequoiadb.utils.FullTextESUtils;
import com.sequoiadb.utils.FullTextUtils;

/**
 * @FileName seqDB-15876:truncate集合记录与删除集合空间并发
 * @Author huangxiaoni
 * @Date 2019.5.14
 */

public class FullText15876 extends SdbTestBase {
    private Random random = new Random();
    private final String CS_NAME = "cs_es_15876";
    private final String CL_NAME = "cl_es_15876";
    private final String IDX_NAME = "idx_es_15876";
    private final BSONObject IDX_KEY = new BasicBSONObject("a", "text");
    private final int RECS_NUM = 20000;

    private Sequoiadb sdb = null;
    private CollectionSpace cs;
    private DBCollection cl;
    private String cappedCSName;

    private Client esClient = null;
    private String esIndexName;

    @BeforeClass
    private void setUp() throws Exception {
        esClient = FullTextESUtils.createTransportClient(esHostName, Integer.parseInt(esServiceName));
        sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "");

        if (CommLib.isStandAlone(sdb)) {
            throw new SkipException("Skip standAlone mode");
        }

        if (sdb.isCollectionSpaceExist(CS_NAME)) {
            // 清理后重新创建，避免前一次跑的结果对其有影响
            sdb.dropCollectionSpace(CS_NAME);
        }
        cs = sdb.createCollectionSpace(CS_NAME);
        cl = cs.createCollection(CL_NAME);
        cl.createIndex(IDX_NAME, IDX_KEY, false, false);
        cappedCSName = FullTextDBUtils.getCappedName(cl, IDX_NAME);
        esIndexName = FullTextDBUtils.getESIndexName(cl, IDX_NAME);

        FullTextDBUtils.insertData(cl, RECS_NUM);
    }

    @Test
    private void test() throws Exception {
        ThreadExecutor es = new ThreadExecutor();
        ThreadTruncate threadTruncate = new ThreadTruncate();
        ThreadDropCS threadDropCS = new ThreadDropCS();
        es.addWorker(threadTruncate);
        es.addWorker(threadDropCS);
        es.run();

        // check results
        if (threadDropCS.getRetCode() == 0) {
            Assert.assertFalse(sdb.isCollectionSpaceExist(CS_NAME));
            Assert.assertTrue(FullTextUtils.isIndexDeleted(sdb, esClient, esIndexName, cappedCSName));
        } else if (threadDropCS.getRetCode() != 0) {
            int cnt = (int) cl.getCount();
            if (threadTruncate.getRetCode() == 0 ) {
                Assert.assertTrue(FullTextUtils.isFulltextRebuild(esClient, cl, IDX_NAME));
                Assert.assertEquals(cnt, 0);
            } else if (threadTruncate.getRetCode() != 0 ) {
                Assert.assertEquals(cnt, RECS_NUM);
            }
            Assert.assertTrue(FullTextUtils.isIndexCreated(esClient, cl, IDX_NAME, cnt));
        }
    }

    @AfterClass
    private void tearDown() throws InterruptedException {
        try {
            try {
                sdb.dropCollectionSpace(CS_NAME);
            } catch (BaseException e) {
                if (e.getErrorCode() != -34) {
                    throw e;
                }
            }
            Assert.assertTrue(FullTextUtils.isIndexDeleted(sdb, esClient, esIndexName, cappedCSName));
        } finally {
            if (sdb != null) {
                sdb.close();
            }
            if (esClient != null) {
                esClient.close();
            }
        }
    }

    private class ThreadTruncate extends ResultStore {
        @ExecuteOrder(step = 1)
        private void truncate() {
            try (Sequoiadb db = new Sequoiadb(SdbTestBase.coordUrl, "", "")) {
                DBCollection cl2 = db.getCollectionSpace(CS_NAME).getCollection(CL_NAME);
                System.out.println(new Date() + " begin " + this.getClass().getName().toString());
                cl2.truncate();
                System.out.println(new Date() + " end   " + this.getClass().getName().toString());
            } catch (BaseException e) {
                if (e.getErrorCode() != -23 && e.getErrorCode() != -248 && e.getErrorCode() != -190
                        && e.getErrorCode() != -147) {
                    throw e;
                }
                saveResult(-1, e);
            }
        }
    }

    private class ThreadDropCS extends ResultStore {
        @ExecuteOrder(step = 1)
        private void dropCS() throws InterruptedException {
            Thread.sleep(random.nextInt(10));
            try (Sequoiadb db = new Sequoiadb(SdbTestBase.coordUrl, "", "")) {
                System.out.println(new Date() + " begin " + this.getClass().getName().toString());
                db.dropCollectionSpace(CS_NAME);
                System.out.println(new Date() + " end   " + this.getClass().getName().toString());
            } catch (BaseException e) {
                if (e.getErrorCode() != -147 && e.getErrorCode() != -190 && e.getErrorCode() != -321) {
                    throw e;
                }
                saveResult(-1, e);
            }
        }
    }
}