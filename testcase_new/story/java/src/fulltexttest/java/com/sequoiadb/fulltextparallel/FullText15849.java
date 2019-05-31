package com.sequoiadb.fulltextparallel;

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
 * @FileName FullText15849.java 集合中存在全文索引，全文检索记录时删除集合
 * @Author luweikang
 * @Date 2019年5月10日
 */
public class FullText15849 extends SdbTestBase {

    private Sequoiadb sdb = null;
    private CollectionSpace cs = null;
    private DBCollection cl = null;
    private String clName = "es_15849";
    private Client esClient = null;
    private String indexName = "fulltextIndex15849";
    private String cappedName = null;
    private String esIndexName = null;
    private int insertNum = 50000;

    @BeforeClass
    public void setUp() throws Exception {
        esClient = FullTextESUtils.createTransportClient(esHostName, Integer.parseInt(esServiceName));
        sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "");
        if (CommLib.isStandAlone(sdb)) {
            throw new SkipException("skip StandAlone");
        }
        cs = sdb.getCollectionSpace(csName);
        cl = cs.createCollection(clName);

        FullTextDBUtils.insertData(cl, insertNum);

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
    }

    @Test
    public void test() throws Exception {

        ThreadExecutor thread = new ThreadExecutor();
        thread.addWorker(new DropCLThread());
        thread.addWorker(new QueryByTextIndexThread());
        thread.run();

        Assert.assertFalse(sdb.getCollectionSpace(csName).isCollectionExist(clName));

        Assert.assertTrue(FullTextUtils.isIndexDeleted(sdb, esClient, esIndexName, cappedName));

    }

    @AfterClass
    public void tearDown() throws Exception {
        try {
            if (sdb.getCollectionSpace(csName).isCollectionExist(clName)) {
                FullTextDBUtils.dropCollection(cs, clName);
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

    private class DropCLThread {

        @ExecuteOrder(step = 1)
        private void DropCL() {
            try (Sequoiadb db = new Sequoiadb(SdbTestBase.coordUrl, "", "")) {
                db.getCollectionSpace(csName).dropCollection(clName);
            }
        }
    }

    private class QueryByTextIndexThread {

        @ExecuteOrder(step = 1)
        private void queryData() throws InterruptedException {
            for (int i = 0; i < 10; i++) {
                try (Sequoiadb db = new Sequoiadb(SdbTestBase.coordUrl, "", "")) {
                    Thread.sleep(1000 + new Random().nextInt(500));
                    DBCollection cl = db.getCollectionSpace(csName).getCollection(clName);
                    DBCursor cur = cl.query("{'': {'$Text': {'query': {'match_all': {}}}}}", null, "{'a': 1}",
                            "{'': '" + indexName + "'}");
                    while (cur.hasNext()) {
                        cur.getNext();
                    }
                    cur.close();
                } catch (BaseException e) {
                    if (e.getErrorCode() != -6 && e.getErrorCode() != -52 && e.getErrorCode() != -23) {
                        throw e;
                    }
                }
            }
        }
    }

}
