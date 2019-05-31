package com.sequoiadb.fulltextparallel;

import java.util.ArrayList;
import java.util.List;

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
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.testcommon.CommLib;
import com.sequoiadb.testcommon.SdbTestBase;
import com.sequoiadb.threadexecutor.ThreadExecutor;
import com.sequoiadb.threadexecutor.annotation.ExecuteOrder;
import com.sequoiadb.utils.FullTextDBUtils;
import com.sequoiadb.utils.FullTextESUtils;
import com.sequoiadb.utils.FullTextUtils;
import com.sequoiadb.utils.StringUtils;

/**
 * @FileName FullText15847.java 增删改记录过程中反复创建删除全文索引 
 * @Author luweikang
 * @Date 2019年5月10日
 */
public class FullText15847 extends SdbTestBase {

    private Sequoiadb sdb = null;
    private CollectionSpace cs = null;
    private DBCollection cl = null;
    private String clName = "es_15847";
    private Client esClient = null;
    private String indexName = "fulltextIndex15847";
    private String cappedName = null;
    private String esIndexName = null;
    private int insertNum = 40000;
    private int testInsertNum = 10000;
    private int updateNum = insertNum / 2;
    private int deleteNum = insertNum / 2;

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
    }

    @Test
    public void test() throws Exception {
        ThreadExecutor thread = new ThreadExecutor();
        thread.addWorker(new TextIndexThread());
        thread.addWorker(new InsertThread());
        thread.addWorker(new UpdateThread());
        thread.addWorker(new DeleteThread());
        thread.run();

        Assert.assertTrue(FullTextUtils.isIndexCreated(esClient, cl, indexName, insertNum - deleteNum + testInsertNum));

        cappedName = FullTextDBUtils.getCappedName(cl, indexName);
        esIndexName = FullTextDBUtils.getESIndexName(cl, indexName);
// TODO cappedName和esIndexName需要在setUp创建索引后获取，否则有可能在TextIndexThread索引被删除成功但是创建失败会获取不到。
// TODO 另外，校验结果需要根据TextIndexThread索引删除/创建成功分别校验，因为最终索引有可能删除成功创建失败或最终创建成功，结果会不一样。
    }

    @AfterClass
    public void tearDown() throws Exception {
        try {
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

    private class TextIndexThread {

        @ExecuteOrder(step = 1)
        private void createIndex() {// TODO 方法名跟实际操作不符（删除、创建索引）
            BSONObject indexObj = new BasicBSONObject();
            indexObj.put("a", "text");
            indexObj.put("b", "text");
            indexObj.put("c", "text");
            indexObj.put("d", "text");
            indexObj.put("e", "text");
            try (Sequoiadb db = new Sequoiadb(SdbTestBase.coordUrl, "", "")) {
                DBCollection cl = db.getCollectionSpace(csName).getCollection(clName);
                for (int i = 0; i < 5; i++) {
                    cl.dropIndex(indexName);
                    cl.createIndex(indexName, indexObj, false, false);
                }
            } catch (BaseException e) {
                if (e.getErrorCode() != -147 && e.getErrorCode() != -190) {
                    throw e;
                }
            }
        }
    }

    private class InsertThread {

        @ExecuteOrder(step = 1)
        private void insert() throws InterruptedException {
            try (Sequoiadb db = new Sequoiadb(SdbTestBase.coordUrl, "", "")) {
                DBCollection cl = db.getCollectionSpace(csName).getCollection(clName);
                List<BSONObject> insertObjs = new ArrayList<BSONObject>();
                String strB = StringUtils.getRandomString(8);
                String strC = StringUtils.getRandomString(32);
                String strD = StringUtils.getRandomString(64);
                int insertNum1 = insertNum + testInsertNum / 10;
                for (int i = 0; i < 10; i++) {
                    for (int j = insertNum; j < insertNum1; j++) {
                        int recordNum = i * ( testInsertNum / 10 ) + j;
                        insertObjs.add((BSONObject) JSON.parse("{recordId: " + recordNum + ", a: '" + clName + recordNum
                                + "', b: '" + strB + "', c: '" + strC + "', d: '" + strD + "'}"));
                    }
                    cl.insert(insertObjs, 0);
                    insertObjs.clear();
                }
            }
        }
    }

    private class UpdateThread {

        @ExecuteOrder(step = 1)
        private void update() throws InterruptedException {
            try (Sequoiadb db = new Sequoiadb(SdbTestBase.coordUrl, "", "")) {
                DBCollection cl = db.getCollectionSpace(csName).getCollection(clName);
                cl.update("{recordId: {$gte: 0, $lt: " + updateNum + "}}", "{$set: {b: 'text'}}", null);
            }
        }
    }

    private class DeleteThread {

        @ExecuteOrder(step = 1)
        private void delete() throws InterruptedException {
            try (Sequoiadb db = new Sequoiadb(SdbTestBase.coordUrl, "", "")) {
                DBCollection cl = db.getCollectionSpace(csName).getCollection(clName);
                cl.delete("{recordId: {$gte: " + deleteNum + ", $lt: " + insertNum + "}}");
            }
        }
    }

}
