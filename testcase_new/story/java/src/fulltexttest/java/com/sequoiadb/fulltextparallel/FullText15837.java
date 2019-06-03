package com.sequoiadb.fulltextparallel;

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
import com.sequoiadb.base.DBCursor;
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
 * @FileName FullText15837.java 删除全文索引与sync操作并发
 * @Author luweikang
 * @Date 2019年5月6日
 */
public class FullText15837 extends SdbTestBase {

    private Sequoiadb sdb = null;
    private CollectionSpace cs = null;
    private DBCollection cl = null;
    private String csName = "cs_15837";
    private String clName = "es_15837";
    private Client esClient = null;
    private String indexName = "fulltextIndex15837";
    private String cappedName = null;
    private String esIndexName = null;
    private int insertNum = 100000;

    @BeforeClass
    public void setUp() {
        esClient = FullTextESUtils.createTransportClient(esHostName, Integer.parseInt(esServiceName));
        sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "");
        if (CommLib.isStandAlone(sdb)) {
            throw new SkipException("skip StandAlone");
        }

        if (sdb.isCollectionSpaceExist(csName)) {
            sdb.dropCollectionSpace(csName);
        }
        cs = sdb.createCollectionSpace(csName);
        cl = cs.createCollection(clName);

        FullTextDBUtils.insertData(cl, insertNum);
    }

    @Test
    public void test() throws Exception {

        BSONObject indexObj = new BasicBSONObject();
        indexObj.put("a", "text");
        indexObj.put("b", "text");
        indexObj.put("c", "text");
        indexObj.put("d", "text");
        indexObj.put("e", "text");
        cl.createIndex(indexName, indexObj, false, false);

        Assert.assertTrue(FullTextUtils.isIndexCreated(esClient, cl, indexName, insertNum));

        cappedName = FullTextDBUtils.getCappedName(cl, indexName);
        esIndexName = FullTextDBUtils.getESIndexName(cl, indexName);

        ThreadExecutor thread = new ThreadExecutor();
        thread.addWorker(new DropIndexThread());
        thread.addWorker(new SyncThread());
        thread.run();

        Assert.assertTrue(FullTextUtils.isIndexDeleted(sdb, esClient, esIndexName, cappedName));

        FullTextDBUtils.insertData(cl, 10000);

        Assert.assertEquals(cl.getCount(), insertNum + 10000);

        DBCursor cur = null;
        try {
            cur = cl.query("{'': {'$Text': {'query': {'match_all': {}}}}}", null, "{'recordId': 1}",
                    "{'': '" + indexName + "'}");
            if (cur.hasNext()) {
                cur.getNext();
            }
            Assert.fail("use not exist fulltext search should be failed!");
        } catch (BaseException e) {
            Assert.assertEquals(e.getErrorCode(), -52, e.getMessage());
        } finally {
            if (cur != null) {
                cur.close();
            }
        }
    }

    @AfterClass
    public void tearDown() throws Exception {
        try {
            FullTextDBUtils.dropCollection(cs, clName);
            if (sdb.isCollectionSpaceExist(csName)) {
                sdb.dropCollectionSpace(csName);
            }
            Assert.assertTrue(FullTextUtils.isIndexDeleted(sdb, esClient, esIndexName, cappedName));
        } finally {
            if (sdb != null) {
                sdb.close();
            }
            if (esClient != null) {
                esClient.close();
            }
        }
    }

    private class DropIndexThread {

        @ExecuteOrder(step = 1)
        private void createIndex() {
            try (Sequoiadb db = new Sequoiadb(SdbTestBase.coordUrl, "", "")) {
                DBCollection cl = db.getCollectionSpace(csName).getCollection(clName);
                cl.dropIndex(indexName);
            }
        }
    }

    private class SyncThread {

        @ExecuteOrder(step = 1)
        private void syncData() {
            try (Sequoiadb db = new Sequoiadb(SdbTestBase.coordUrl, "", "")) {
                BSONObject options = new BasicBSONObject();
                options.put("Block", true);
                options.put("CollectionSpace", csName);
                db.sync(options);
            }
        }
    }

}
