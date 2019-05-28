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
import com.sequoiadb.testcommon.CommLib;
import com.sequoiadb.testcommon.SdbTestBase;
import com.sequoiadb.threadexecutor.ResultStore;
import com.sequoiadb.threadexecutor.ThreadExecutor;
import com.sequoiadb.threadexecutor.annotation.ExecuteOrder;
import com.sequoiadb.utils.FullTextDBUtils;
import com.sequoiadb.utils.FullTextESUtils;
import com.sequoiadb.utils.FullTextUtils;

/**
 * @FileName seqDB-15880:truncate集合记录与阻塞sync并发
 * @Author huangxiaoni
 * @Date 2019.5.14
 */
// TODO :建议创建自己的集合空间，sync指定集合空间刷盘，避免对其他用例的影响；
public class FullText15880 extends SdbTestBase {
    private Random random = new Random();
    private final String CL_NAME = "cl_es_15880";
    private final String IDX_NAME = "idx_es_15880";
    private final BSONObject IDX_KEY = new BasicBSONObject("a", "text");
    private final int RECS_NUM = 20000;

    private Sequoiadb sdb = null;
    private CollectionSpace cs;
    private DBCollection cl;
    private String cappedCSName;

    private Client esClient = null;
    private String esIndexName;
    private int lid;

    @BeforeClass
    private void setUp() throws Exception {
        esClient = FullTextESUtils.createTransportClient(esHostName, Integer.parseInt(esServiceName));
        sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "");

        if (CommLib.isStandAlone(sdb)) {
            throw new SkipException("Skip standAlone mode");
        }

        cs = sdb.getCollectionSpace(SdbTestBase.csName);
        cl = cs.createCollection(CL_NAME);
        cl.createIndex(IDX_NAME, IDX_KEY, false, false);
        FullTextDBUtils.insertData(cl, RECS_NUM);
        cappedCSName = FullTextDBUtils.getCappedName(cl, IDX_NAME);
        esIndexName = FullTextDBUtils.getESIndexName(cl, IDX_NAME);

        // 确保预置的数据同步到es完成，避免获取lids报索引不存在
        Assert.assertTrue(FullTextUtils.isIndexCreated(esClient, cl, IDX_NAME, RECS_NUM));
        lid = FullTextESUtils.getCommitCLLIDFromES(esClient, esIndexName);
    }

    @Test
    private void test() throws Exception {
        ThreadExecutor es = new ThreadExecutor();
        ThreadTruncate threadTruncate = new ThreadTruncate();
        ThreadDBSync threadDBSync = new ThreadDBSync();
        es.addWorker(threadTruncate);
        es.addWorker(threadDBSync);
        es.run();

        // check results
        Assert.assertTrue(FullTextUtils.isFulltextRebuild(esClient, esIndexName, lid));
        Assert.assertTrue(FullTextUtils.isIndexCreated(esClient, cl, IDX_NAME, 0));
    }

    @AfterClass
    private void tearDown() throws InterruptedException {
        try {
            FullTextDBUtils.dropCollection(cs, CL_NAME);
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
        private void truncate() throws InterruptedException {
            Thread.sleep(random.nextInt(100));
            try (Sequoiadb db = new Sequoiadb(SdbTestBase.coordUrl, "", "")) {
                DBCollection cl2 = db.getCollectionSpace(SdbTestBase.csName).getCollection(CL_NAME);
                System.out.println(new Date() + " begin " + this.getClass().getName().toString());
                cl2.truncate();
                System.out.println(new Date() + " end   " + this.getClass().getName().toString());
            }
        }
    }

    private class ThreadDBSync {
        @ExecuteOrder(step = 1)
        private void createIndex() {
            try (Sequoiadb db = new Sequoiadb(SdbTestBase.coordUrl, "", "")) {
                System.out.println(new Date() + " begin " + this.getClass().getName().toString());
                for (int i = 0; i < 3; i++) {
                    db.sync(new BasicBSONObject("Block", true));
                }
                System.out.println(new Date() + " end   " + this.getClass().getName().toString());
            }
        }
    }
}