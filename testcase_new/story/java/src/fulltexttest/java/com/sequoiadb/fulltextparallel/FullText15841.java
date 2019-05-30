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
 * @FileName FullText15841.java 创建全文索引与truncate并发
 * @Author luweikang
 * @Date 2019年5月10日
 */
public class FullText15841 extends SdbTestBase {

    private Sequoiadb sdb = null;
    private CollectionSpace cs = null;
    private DBCollection cl = null;
    private String clName = "es_15841";
    private Client esClient = null;
    private String indexName = "fulltextIndex15841";
    private BSONObject indexObj = null;
    private int insertNum = 100000;

    @BeforeClass
    public void setUp() {
        esClient = FullTextESUtils.createTransportClient(esHostName, Integer.parseInt(esServiceName));
        sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "");
        if (CommLib.isStandAlone(sdb)) {
            throw new SkipException("skip StandAlone");
        }
        cs = sdb.getCollectionSpace(csName);
        cl = cs.createCollection(clName);

        FullTextDBUtils.insertData(cl, insertNum);

        indexObj = new BasicBSONObject();
        indexObj.put("a", "text");
        indexObj.put("b", "text");
        indexObj.put("c", "text");
        indexObj.put("d", "text");
        indexObj.put("e", "text");
    }

    @Test
    public void test() throws Exception {

        ThreadExecutor thread = new ThreadExecutor();
        CreateIndexThread createIndexThread = new CreateIndexThread();
        TruncateThread truncateThread = new TruncateThread();
        thread.addWorker(createIndexThread);
        thread.addWorker(truncateThread);
        thread.run();
        if (createIndexThread.getRetCode() != 0) {
            cl.createIndex(indexName, indexObj, false, false);
        }
        if (truncateThread.getRetCode() == 0) {
            Assert.assertTrue(FullTextUtils.isIndexCreated(esClient, cl, indexName, 0));
        } else {
            Assert.assertTrue(FullTextUtils.isIndexCreated(esClient, cl, indexName, insertNum));
        }

    }

    @AfterClass
    public void tearDown() throws Exception {
        try {
            String cappedName = FullTextDBUtils.getCappedName(cl, indexName);
            String esIndexName = FullTextDBUtils.getESIndexName(cl, indexName);
            FullTextDBUtils.dropCollection(cs, clName);
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

    private class CreateIndexThread extends ResultStore {

        @ExecuteOrder(step = 1)
        private void createIndex() {
            try (Sequoiadb db = new Sequoiadb(SdbTestBase.coordUrl, "", "")) {
                DBCollection cl = db.getCollectionSpace(csName).getCollection(clName);
                cl.createIndex(indexName, indexObj, false, false);
            } catch (BaseException e) {
                if (e.getErrorCode() != -321) {
                    throw e;
                }
                saveResult(-1, e);
            }
        }
    }

    private class TruncateThread extends ResultStore {

        @ExecuteOrder(step = 1)
        private void truncate() throws InterruptedException {
            try (Sequoiadb db = new Sequoiadb(SdbTestBase.coordUrl, "", "")) {
                Thread.sleep(1000 + new Random().nextInt(100));
                DBCollection cl = db.getCollectionSpace(csName).getCollection(clName);
                cl.truncate();
            } catch (BaseException e) {
                e.printStackTrace();
                saveResult(-1, e);
            }
        }
    }

}
