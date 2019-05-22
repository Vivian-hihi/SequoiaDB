package com.sequoiadb.fulltextparallel;

import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.atomic.AtomicInteger;

import org.bson.BSONObject;
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
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.testcommon.CommLib;
import com.sequoiadb.testcommon.SdbTestBase;
import com.sequoiadb.threadexecutor.ThreadExecutor;
import com.sequoiadb.threadexecutor.annotation.ExecuteOrder;
import com.sequoiadb.threadexecutor.exception.SchException;
import com.sequoiadb.utils.FullTextDBUtils;
import com.sequoiadb.utils.FullTextESUtils;
import com.sequoiadb.utils.FullTextUtils;

/**
 * @FileName: seqDB-15797:集合中存在全文索引，增删改记录与删除集合空间并发
 * @Author zhaoyu
 * @Date 2019-05-10
 */
//TODO:问题同 15796 用例
public class Fulltext15797 extends SdbTestBase {
    private Sequoiadb sdb = null;
    private CollectionSpace cs = null;
    private DBCollection cl = null;
    private String csName = "cs15797";
    private String clName = "cl15797";
    private String indexName = "fulltext15797";
    private Client esClient = null;
    private int insertNum = 30000;
    private AtomicInteger atomic = new AtomicInteger(insertNum);
    private Insert insert;
    private Update update;
    private Delete delete;
    private Query query;
    private DropCS dropCS;
    private ThreadExecutor te = new ThreadExecutor(600000);

    @BeforeClass
    public void setUp() throws SchException {
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

        cl.createIndex("id", "{id:1}", false, false);
        cl.createIndex(indexName, "{a:'text',b:'text'}", false, false);
        insertRecord(cl, insertNum);

        insert = new Insert();
        update = new Update();
        delete = new Delete();
        query = new Query();
        dropCS = new DropCS();

        te.addWorker(insert);
        te.addWorker(update);
        te.addWorker(delete);
        te.addWorker(query);
        te.addWorker(dropCS);
    }

    @AfterClass
    public void tearDown() {
        String esIndexName = null;
        String cappedCLName = null;
        if (sdb.isCollectionSpaceExist(csName)) {
            DBCollection cl = sdb.getCollectionSpace(csName).getCollection(clName);
            esIndexName = FullTextDBUtils.getESIndexName(cl, indexName);
            cappedCLName = FullTextDBUtils.getCappedName(cl, indexName);
        }
        FullTextDBUtils.dropCollectionSpace(sdb, csName);
        if (esIndexName != null && cappedCLName != null) {
            Assert.assertTrue(FullTextUtils.isIndexDeleted(sdb, esClient, esIndexName, cappedCLName));
        }
        sdb.close();
        esClient.close();
    }

    @Test
    public void test() throws Exception {
        // 获取原始集合所在组及固定集合名，作为后续结果校验的输入
        String cappedCLName = FullTextDBUtils.getCappedName(cl, indexName);
        String esIndexName = FullTextDBUtils.getESIndexName(cl, indexName);

        // 执行并发测试
        te.run();

        // 如果集合未删除成功，那么校验集合中主备节点一致性,否则固定集合空间删除成功，ES端索引删除成功
        if (sdb.getCollectionSpace(csName).isCollectionExist(clName)) {
            cl.insert("{a:'insert',b:'insert'}");
            Assert.assertTrue(FullTextUtils.isIndexCreated(esClient, cl, indexName, atomic.incrementAndGet()));
        } else {
            // 主备节点上固定集合空间删除成功
            Assert.assertTrue(FullTextUtils.isIndexDeleted(sdb, esClient, esIndexName, cappedCLName));
        }
    }

    private class Insert {
        private Sequoiadb db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
        private DBCollection cl = db.getCollectionSpace(csName).getCollection(clName);

        @ExecuteOrder(step = 1, desc = "插入记录")
        public void insertRecord() {
            try {
                System.out.println("start insert thread....");
                for (int i = insertNum; i < insertNum * 2; i++) {
                    cl.insert("{id:" + i + ",a:'fulltext15796" + i + "',b:'fulltext15796" + i + "'}");
                    atomic.incrementAndGet();
                }
                System.out.println("end insert thread....");
            } catch (BaseException e) {
                if (e.getErrorCode() != -23 && e.getErrorCode() != -248) {
                    e.printStackTrace();
                    Assert.fail(e.getMessage());
                }
            } finally {
                db.close();
            }
        }

    }

    private class Update {
        private Sequoiadb db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
        private DBCollection cl = db.getCollectionSpace(csName).getCollection(clName);

        @ExecuteOrder(step = 1, desc = "更新所有记录")
        public void updateRecord() {
            try {
                System.out.println("start update thread....");
                cl.update(null, "{$set:{b:'update_15796'}}", null);
                System.out.println("end update thread....");
            } catch (BaseException e) {
                if (e.getErrorCode() != -23 && e.getErrorCode() != -248) {
                    e.printStackTrace();
                    Assert.fail(e.getMessage());
                }
            } finally {
                db.close();
            }
        }

    }

    private class Delete {
        private Sequoiadb db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
        private DBCollection cl = db.getCollectionSpace(csName).getCollection(clName);

        @ExecuteOrder(step = 1, desc = "删除所有记录")
        public void deleteRecord() {
            try {
                System.out.println("start delete thread....");
                for (int i = 0; i < insertNum * 2; i++) {
                    cl.delete("{id:" + i + "}", "{'':'id'}");
                    atomic.decrementAndGet();
                }
                System.out.println("end delete thread....");
            } catch (BaseException e) {
                if (e.getErrorCode() != -23 && e.getErrorCode() != -248) {
                    e.printStackTrace();
                    Assert.fail(e.getMessage());
                }
            } finally

            {
                db.close();
            }
        }
    }

    private class Query {
        private Sequoiadb db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
        private DBCollection cl = db.getCollectionSpace(csName).getCollection(clName);

        @ExecuteOrder(step = 1, desc = "全文检索全部记录")
        public void deleteRecord() {
            DBCursor cursor = null;
            try {
                System.out.println("start query thread....");
                cursor = cl.query("{'':{$Text:{query:{match_all:{}}}}}", "{a:1,b:1}", null, null);
                while (cursor.hasNext()) {
                    cursor.getNext();
                }
                cursor.close();
                System.out.println("end query thread....");
            } catch (BaseException e) {
                // 集合被删除报-23
                // 集合正在被删除报-248
                // 全文索引在ES端还没创建时报-6、-52
                if (e.getErrorCode() != -23 && e.getErrorCode() != -248 && e.getErrorCode() != -6
                        && e.getErrorCode() != -52) {
                    e.printStackTrace();
                    Assert.fail(e.getMessage());
                }
            } finally {
                db.closeAllCursors();
                db.close();
            }
        }
    }

    private class DropCS {
        private Sequoiadb db = new Sequoiadb(SdbTestBase.coordUrl, "", "");

        @ExecuteOrder(step = 1, desc = "删除集合")
        public void dropCS() {
            try {
                try {
                    Thread.sleep(3000);
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
                System.out.println("start dropcs thread....");
                db.dropCollectionSpace(csName);
                System.out.println("end dropcs thread....");
            } catch (BaseException e) {
                if (e.getErrorCode() != -147 && e.getErrorCode() != -190) {
                    e.printStackTrace();
                    Assert.fail(e.getMessage());
                }
            } finally {
                db.close();
            }
        }

    }

    public void insertRecord(DBCollection cl, int insertNums) {
        List<BSONObject> insertObjs = new ArrayList<>();
        for (int i = 0; i < 100; i++) {
            for (int j = 0; j < insertNums / 100; j++) {
                int k = i * 100 + j;
                insertObjs.add((BSONObject) JSON.parse("{id:" + k + ",a: 'test_11981_" + i * 100 + j
                        + "', b: 'test_11981_aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa " + i * 100 + j + "'}"));
            }
            cl.insert(insertObjs, 0);
            insertObjs.clear();
        }
    }

}
