package com.sequoiadb.fulltextparallel;

import java.util.Date;

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
import com.sequoiadb.threadexecutor.ThreadExecutor;
import com.sequoiadb.threadexecutor.annotation.ExecuteOrder;
import com.sequoiadb.utils.FullTextDBUtils;
import com.sequoiadb.utils.FullTextESUtils;
import com.sequoiadb.utils.FullTextUtils;

/**
 * @FileName seqDB-15858:集合中存在全文索引，并发truncate
 * @Author huangxiaoni
 * @Date 2019.5.8
 */

public class FullText15858 extends SdbTestBase {
    private final int THREAD_NUM = 5;
    private final String CL_NAME = "cl_es_15858";
    private final String IDX_NAME = "idx_es_15858";
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

        cs = sdb.getCollectionSpace(SdbTestBase.csName);
        cl = cs.createCollection(CL_NAME);
        cl.createIndex(IDX_NAME, IDX_KEY, false, false);
        cappedCSName = FullTextDBUtils.getCappedName(cl, IDX_NAME);
        esIndexName = FullTextDBUtils.getESIndexName(cl, IDX_NAME);

        FullTextDBUtils.insertData(cl, RECS_NUM);

        // 确保预置的数据同步到es完成，避免获取lids报索引不存在
        Assert.assertTrue(FullTextUtils.isIndexCreated(esClient, cl, IDX_NAME, RECS_NUM));
    }

    @Test
    private void test() throws Exception {
        ThreadExecutor es = new ThreadExecutor();
        for (int i = 0; i < THREAD_NUM; i++) {
            es.addWorker(new ThreadTruncate());
        }
        es.run();

        // check total count
        long updCnt = cl.getCount();
        Assert.assertEquals(updCnt, 0);

        // check consistency
        // TODO :多次执行truncate操作，按照当前的公共方法，可能会导致该用例随机失败，修改公共方法后， 这里需要同步修改
        // TODO ：公共方法还未提交，待公共方法提交后修改
        Assert.assertTrue(FullTextUtils.isFulltextRebuild(esClient, cl, IDX_NAME));
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

    private class ThreadTruncate {
        @ExecuteOrder(step = 1)
        private void truncate() {
            System.out.println(new Date() + " " + this.getClass().getName().toString());
            try (Sequoiadb db = new Sequoiadb(SdbTestBase.coordUrl, "", "")) {
                DBCollection cl2 = db.getCollectionSpace(SdbTestBase.csName).getCollection(CL_NAME);
                cl2.truncate();
            } catch (BaseException e) {
                if (e.getErrorCode() != -321 && e.getErrorCode() != -190) {
                    throw e;
                }
            }
        }
    }
}
