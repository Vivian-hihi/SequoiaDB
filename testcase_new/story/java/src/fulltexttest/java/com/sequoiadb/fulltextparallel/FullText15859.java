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

/**
 * @FileName seqDB-15859:集合中存在全文索引，并发全文检索/普通查询记录
 * @Author huangxiaoni
 * @Date 2019.5.8
 */

public class FullText15859 extends SdbTestBase {
    private final static int THREAD_NUM = 2;
    private final static String CL_NAME = "cl_es_15859";
    private final static String IDX_NAME = "idx_es_15859";
    private final static BSONObject IDX_KEY = (BSONObject) JSON.parse("{a:'text',b:'text',c:'text',d:'text'}");
    private final static int RECS_NUM = 50000;

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

        // 确保预置的数据同步到es完成，避免test中查询的数据未同步完成导致非预期
        Assert.assertTrue(FullTextUtils.isIndexCreated(esClient, cl, IDX_NAME, RECS_NUM));
    }

    @Test
    private void test() throws Exception {
        ThreadExecutor es = new ThreadExecutor();
        for (int i = 0; i < THREAD_NUM; i++) {
            es.addWorker(new ThreadFullTextSearch());
            es.addWorker(new ThreadQuery());
        }
        es.run();

        // check consistency
        Assert.assertTrue(FullTextUtils.isIndexCreated(esClient, cl, IDX_NAME, RECS_NUM));
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

    private class ThreadFullTextSearch {
        private int rcRecsNum = 0;

        @ExecuteOrder(step = 1, desc = "全文检索")
        private void fullTextSearch() {
            try (Sequoiadb db = new Sequoiadb(SdbTestBase.coordUrl, "", "")) {
                DBCollection cl2 = db.getCollectionSpace(SdbTestBase.csName).getCollection(CL_NAME);
                // TODO :匹配条件不要强依赖公共方法里面具体的字段值，避免公共方法后续变动，影响用例的测试点；其他用例类似
                // TODO :其他用例删除和更新有用到具体值做匹配条件，对具体值肯定会存在依赖，公共方法特别是记录内容本身就要谨慎修改，用例暂不修改
                BSONObject matcher = new BasicBSONObject("", new BasicBSONObject("$Text",
                        new BasicBSONObject("query", new BasicBSONObject("match", new BasicBSONObject("a", CL_NAME)))));
                BSONObject orderby = new BasicBSONObject("a", 1);
                System.out.println(new Date() + " begin " + this.getClass().getName().toString());
                DBCursor cursor = cl2.query(matcher, null, orderby, null);
                while (cursor.hasNext()) {
                    cursor.getNext();
                    rcRecsNum++;
                }
                System.out.println(new Date() + " end   " + this.getClass().getName().toString());
            }
        }

        @ExecuteOrder(step = 2, desc = "检查查询返回结果")
        private void checkResult() {
            Assert.assertEquals(rcRecsNum, RECS_NUM);
        }
    }

    private class ThreadQuery {
        private int rcRecsNum = 0;

        @ExecuteOrder(step = 1, desc = "普通查询")
        private void fullTextSearch() {
            try (Sequoiadb db = new Sequoiadb(SdbTestBase.coordUrl, "", "")) {
                DBCollection cl2 = db.getCollectionSpace(SdbTestBase.csName).getCollection(CL_NAME);
                BSONObject matcher = new BasicBSONObject("a", new BasicBSONObject("$exists", 1));
                BSONObject orderby = new BasicBSONObject("a", 1);
                BSONObject hint = new BasicBSONObject("", IDX_NAME);
                System.out.println(new Date() + " begin " + this.getClass().getName().toString());
                DBCursor cursor = cl2.query(matcher, null, orderby, hint);
                while (cursor.hasNext()) {
                    cursor.getNext();
                    rcRecsNum++;
                }
                System.out.println(new Date() + " end   " + this.getClass().getName().toString());
            }
        }

        @ExecuteOrder(step = 2, desc = "检查查询返回结果")
        private void checkResult() {
            Assert.assertEquals(rcRecsNum, RECS_NUM);
        }
    }
}
