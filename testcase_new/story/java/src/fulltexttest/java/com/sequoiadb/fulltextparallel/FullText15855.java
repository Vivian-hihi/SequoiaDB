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
import com.sequoiadb.testcommon.CommLib;
import com.sequoiadb.testcommon.SdbTestBase;
import com.sequoiadb.threadexecutor.ThreadExecutor;
import com.sequoiadb.threadexecutor.annotation.ExecuteOrder;
import com.sequoiadb.utils.FullTextDBUtils;
import com.sequoiadb.utils.FullTextESUtils;
import com.sequoiadb.utils.FullTextUtils;

/**
 * @FileName seqDB-15855:集合中存在全文索引，并发插入记录
 * @Author huangxiaoni
 * @Date 2019.4.25 
 */

public class FullText15855 extends SdbTestBase {
    private final int TIMEOUT = 300000; // 5min
    private final int THREAD_NUM = 2;
    private final String CL_NAME = "cl_es_15855";
    private final String IDX_NAME = "idx_es_15855";
    private final BSONObject IDX_KEY = new BasicBSONObject("a", "text");
    private final int RECS_NUM = 20000;
    
    private Sequoiadb sdb = null;
    private CollectionSpace cs;
    private DBCollection cl;
    private String cappedCSName;
    
    private Client esClient = null;
    private String esIndexName;

    @BeforeClass
    private void setUp() {
        esClient = FullTextESUtils.createTransportClient(esHostName, Integer.parseInt(esServiceName));
        sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "");

        if (CommLib.isStandAlone(sdb)) {
            throw new SkipException("Skip standAlone mode");
        }

        cs = sdb.getCollectionSpace(SdbTestBase.csName);
        cl = cs.createCollection(CL_NAME);
        cl.createIndex(IDX_NAME, IDX_KEY, false, false);
        cappedCSName = FullTextDBUtils.getCappedName(cl, IDX_NAME);
        esIndexName  = FullTextDBUtils.getESIndexName(cl, IDX_NAME);
    }

    @Test
    private void test() throws Exception {
        ThreadExecutor es = new ThreadExecutor( TIMEOUT );
        for (int i = 0; i < THREAD_NUM; i++) {
            es.addWorker(new ThreadInsert());
        }
        es.run();

        Assert.assertTrue(FullTextUtils.isIndexCreated(esClient, cl, IDX_NAME, 
                RECS_NUM * THREAD_NUM));
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

    private class ThreadInsert {
        @ExecuteOrder(step = 1)
        private void insert() {
            System.out.println(new Date() + " " + this.getClass().getName().toString());
            try(Sequoiadb db = new Sequoiadb(SdbTestBase.coordUrl, "", "")) {
                DBCollection cl2 = db.getCollectionSpace(SdbTestBase.csName).getCollection(CL_NAME);
                FullTextDBUtils.insertData(cl2, RECS_NUM);
            }
        }
    }
}
