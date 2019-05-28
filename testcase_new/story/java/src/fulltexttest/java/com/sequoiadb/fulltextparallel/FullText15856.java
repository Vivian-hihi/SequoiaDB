package com.sequoiadb.fulltextparallel;

import java.util.Date;

import org.bson.BSONObject;
import org.bson.BasicBSONObject;
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
import com.sequoiadb.testcommon.CommLib;
import com.sequoiadb.testcommon.SdbTestBase;
import com.sequoiadb.threadexecutor.ThreadExecutor;
import com.sequoiadb.threadexecutor.annotation.ExecuteOrder;
import com.sequoiadb.utils.FullTextDBUtils;
import com.sequoiadb.utils.FullTextESUtils;
import com.sequoiadb.utils.FullTextUtils;
import com.sequoiadb.utils.StringUtils;

/**
 * @FileName seqDB-15856:集合中存在全文索引，并发更新记录
 * @Author huangxiaoni
 * @Date 2019.5.8
 */

public class FullText15856 extends SdbTestBase {
    private final int THREAD_NUM = 5;
    private final int TIMEOUT = 300000; // 5min
    private final String CL_NAME = "cl_es_15856";
    private final String IDX_NAME = "idx_es_15856";
    private final BSONObject IDX_KEY = (BSONObject) JSON.parse("{a:'text',b:'text',c:'text'}");
    private final int RECS_NUM = 30000;

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
        esIndexName = FullTextDBUtils.getESIndexName(cl, IDX_NAME);

        FullTextDBUtils.insertData(cl, RECS_NUM);
    }

    @Test
    private void test() throws Exception {
        // modifier1
        String modVal1 = StringUtils.getRandomString(16);
        BSONObject modifier1 = new BasicBSONObject("$set", new BasicBSONObject("b", modVal1));
        // modifier2
        String modVal2 = StringUtils.getRandomString(32);
        BSONObject modifier2 = new BasicBSONObject("$set", new BasicBSONObject("c", modVal2));
        // thread
        ThreadExecutor es = new ThreadExecutor(TIMEOUT);
        for (int i = 0; i < THREAD_NUM; i++) {
            es.addWorker(new ThreadUpdate(modifier1));
            es.addWorker(new ThreadUpdate(modifier2));
        }
        es.run();

        // check consistency
        Assert.assertTrue(FullTextUtils.isIndexCreated(esClient, cl, IDX_NAME, RECS_NUM));
        // check total count for update
        BSONObject matcher = new BasicBSONObject();
        matcher.put("b", modVal1);
        matcher.put("c", modVal2);
        long updCnt = cl.getCount(matcher);
        Assert.assertEquals(updCnt, RECS_NUM);
        // check fullTextSearch
        Assert.assertEquals(this.fullTextSearch(new BasicBSONObject("b", modVal1)), RECS_NUM);
        Assert.assertEquals(this.fullTextSearch(new BasicBSONObject("c", modVal2)), RECS_NUM);
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

    private class ThreadUpdate {
        private BSONObject modifier;

        private ThreadUpdate(BSONObject modifier) {
            this.modifier = modifier;
        }

        @ExecuteOrder(step = 1)
        private void update() {
            System.out.println(new Date() + " " + this.getClass().getName().toString());
            try (Sequoiadb db = new Sequoiadb(SdbTestBase.coordUrl, "", "")) {
                DBCollection cl2 = db.getCollectionSpace(SdbTestBase.csName).getCollection(CL_NAME);
                BSONObject matcher = new BasicBSONObject("a", new BasicBSONObject("$exists", 1));
                BSONObject hint = new BasicBSONObject("", IDX_NAME);
                cl2.update(matcher, modifier, hint);
            }
        }
    }
    
    private int fullTextSearch(BSONObject obj) {
        int rcRecsNum = 0;
        BSONObject matcher = new BasicBSONObject("", new BasicBSONObject("$Text",
                new BasicBSONObject("query", 
                        new BasicBSONObject("match", obj))));
        DBCursor cursor = cl.query(matcher, null, null, null);
        while (cursor.hasNext()) {
            cursor.getNext();
            rcRecsNum++;
        }
        return rcRecsNum;
    }
}
