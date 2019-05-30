package com.sequoiadb.fulltextparallel;

import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Date;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
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
import com.sequoiadb.threadexecutor.ResultStore;
import com.sequoiadb.threadexecutor.ThreadExecutor;
import com.sequoiadb.threadexecutor.annotation.ExecuteOrder;
import com.sequoiadb.utils.FullTextDBUtils;
import com.sequoiadb.utils.FullTextESUtils;
import com.sequoiadb.utils.FullTextUtils;

/**
 * @FileName: seqDB-15872:不同集合上创建/删除全文索引与增删改全文索引记录并发
 * @Author zhaoyu
 * @Date 2019-05-14
 */

public class Fulltext15872 extends SdbTestBase {
    private Sequoiadb sdb = null;
    private List<String> csNames = new ArrayList<String>();
    private List<String> clNames = new ArrayList<String>();
    private String csBasicName = "cs15872";
    private String clBasicName = "cl15872";
    private int csNum = 2;
    private int clNum = 4;
    private String indexName = "fulltext15872";
    private Client esClient = null;
    private int insertNum = 20000;
    private ThreadExecutor te = new ThreadExecutor(3600000);
    // 统计各集合中的记录数，map的key为集合名、value为集合中的记录数
    private Map<String, AtomicInteger> insertNumInCLMap = new HashMap<String, AtomicInteger>();
    private SimpleDateFormat df = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss.S");
    private List<String> esIndexNames = new ArrayList<String>();
    private List<String> cappedCLNames = new ArrayList<String>();

    @BeforeClass
    public void setUp() {
        esClient = FullTextESUtils.createTransportClient(esHostName, Integer.parseInt(esServiceName));
        sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "");
        if (CommLib.isStandAlone(sdb)) {
            throw new SkipException("skip StandAlone");
        }

        for (int i = 0; i < csNum; i++) {
            String csName = csBasicName + "_" + i;
            csNames.add(csName);
        }

        for (int i = 0; i < clNum; i++) {
            String clName = clBasicName + "_" + i;
            clNames.add(clName);
        }
        // 集合空间下的集合一半用于创建全文索引、一半用于删除全文索引
        for (String csName : csNames) {
            if (sdb.isCollectionSpaceExist(csName)) {
                sdb.dropCollectionSpace(csName);
            }
            CollectionSpace cs = sdb.createCollectionSpace(csName);
            for (int i = 0; i < clNum; i++) {
                DBCollection cl = cs.createCollection(clNames.get(i));
                cl.createIndex("id", "{id:1}", false, false);
                insertRecord(cl, insertNum);

                AtomicInteger numInCL = new AtomicInteger(insertNum);
                insertNumInCLMap.put(csName + "_" + clNames.get(i), numInCL);
                if (i < clNum / 2) {
                    cl.createIndex(indexName, "{a:'text',b:'text'}", false, false);
                }
            }
        }
    }

    @AfterClass
    public void tearDown() {
        try {
            for (String csName : csNames) {
                FullTextDBUtils.dropCollectionSpace(sdb, csName);
            }
            if (!esIndexNames.isEmpty() && !cappedCLNames.isEmpty()) {
                Assert.assertTrue(FullTextUtils.isIndexDeleted(sdb, esClient, esIndexNames, cappedCLNames));
            }
        } finally {
            sdb.close();
            esClient.close();
        }
    }

    @Test
    public void test() throws Exception {
        // 执行并发测试
        for (String csName : csNames) {
            for (int i = 0; i < clNum; i++) {
                if (i < clNum / 2) {
                    te.addWorker(new DropFullIndexThread(csName, clNames.get(i)));
                } else {
                    te.addWorker(new CreateFullIndexThread(csName, clNames.get(i)));
                }

                te.addWorker(new InsertThread(csName, clNames.get(i)));
                te.addWorker(new UpdateThread(csName, clNames.get(i)));
                te.addWorker(new DeleteThread(csName, clNames.get(i)));
                te.addWorker(new QueryThread(csName, clNames.get(i)));
            }
        }
        te.run();

        // 结果校验
        for (String csName : csNames) {
            CollectionSpace cs = sdb.getCollectionSpace(csName);
            for (String clName : clNames) {
                DBCollection cl = cs.getCollection(clName);
                if (cl.isIndexExist(indexName)) {
                    // 记录所有集合中的ES端的全文索引名及固定集合名
                    String esIndexName = FullTextDBUtils.getESIndexName(cl, indexName);
                    esIndexNames.add(esIndexName);
                    String cappedCLName = FullTextDBUtils.getCappedName(cl, indexName);
                    cappedCLNames.add(cappedCLName);

                    // 同步符合预期
                    AtomicInteger numInCL = (AtomicInteger) insertNumInCLMap.get(csName + "_" + clName);
                    Assert.assertTrue(FullTextUtils.isIndexCreated(esClient, cl, indexName, numInCL.get()));

                    // 全文检索数据记录正确
                    DBCursor cursor = cl.query("{'':{'$Text':{query:{match_all:{}}}}}", "{a:1,c:1}", null, null);
                    int actNum = 0;
                    while (cursor.hasNext()) {
                        cursor.getNext();
                        actNum++;
                    }
                    Assert.assertEquals(actNum, numInCL.get());

                    // 插入记录
                    insertRecord(cl, insertNum);
                    numInCL.addAndGet(insertNum);

                    // 同步符合预期
                    Assert.assertTrue(FullTextUtils.isIndexCreated(esClient, cl, indexName, numInCL.get()));
                }

            }

        }

    }

    private class DropFullIndexThread extends ResultStore {
        private String csName = null;
        private String clName = null;
        private String cappedCLName = null;
        private String esIndexName = null;

        public DropFullIndexThread(String csName, String clName) {
            super();
            this.csName = csName;
            this.clName = clName;
        }

        @ExecuteOrder(step = 1, desc = "删除全文索引")
        public void dropFullIndex() {
            Sequoiadb db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
            System.out.println(this.getClass().getName().toString() + " start at:" + df.format(new Date()));
            try {
                DBCollection cl = db.getCollectionSpace(csName).getCollection(clName);
                cappedCLName = FullTextDBUtils.getCappedName(cl, indexName);
                esIndexName = FullTextDBUtils.getESIndexName(cl, indexName);
                cl.dropIndex(indexName);
            } catch (BaseException e) {
                if (e.getErrorCode() != -147 && e.getErrorCode() != -190) {
                    e.printStackTrace();
                    Assert.fail(e.getMessage());
                }
                saveResult(e.getErrorCode(), e);
            } finally {
                db.close();
            }
            System.out.println(this.getClass().getName().toString() + " stop at:" + df.format(new Date()));

        }

        @ExecuteOrder(step = 2, desc = "结果校验")
        public void checkResult() {
            Sequoiadb db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
            Client es = FullTextESUtils.createTransportClient(esHostName, Integer.parseInt(esServiceName));

            try {
                DBCollection cl = db.getCollectionSpace(csName).getCollection(clName);
                if (getRetCode() == 0) {
                    // 全文索引删除成功，校验固定集合及ES端的全文索引同步删除
                    // 全文索引删除失败，由于需校验记录数，统一放到并发执行完成后进行校验
                    Assert.assertTrue(FullTextUtils.isIndexDeleted(db, es, esIndexName, cappedCLName));

                    // 全文检索数据报错
                    try {
                        cl.query("{'':{'$Text':{query:{match_all:{}}}}}", "{a:1,c:1}", null, null);
                    } catch (BaseException e) {
                        if (e.getErrorCode() != -6 && e.getErrorCode() != -52) {
                            e.printStackTrace();
                            Assert.fail(e.getMessage());
                        }
                    }
                } else {
                    Assert.assertTrue(cl.isIndexExist(indexName));
                }
            } finally {
                db.close();
                es.close();
            }

        }
    }

    private class CreateFullIndexThread {
        private Sequoiadb db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
        private String csName = null;
        private String clName = null;

        public CreateFullIndexThread(String csName, String clName) {
            super();
            this.csName = csName;
            this.clName = clName;
        }

        @ExecuteOrder(step = 1, desc = "创建全文索引")
        public void createFullIndex() {
            System.out.println(this.getClass().getName().toString() + " start at:" + df.format(new Date()));
            try {
                DBCollection cl = db.getCollectionSpace(csName).getCollection(clName);
                cl.createIndex(indexName, "{a:'text',b:'text'}", false, false);
                Assert.assertTrue(cl.isIndexExist(indexName));
            } finally {
                db.close();
            }
            System.out.println(this.getClass().getName().toString() + " stop at:" + df.format(new Date()));
        }

    }

    private class InsertThread {
        private Sequoiadb db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
        private String csName = null;
        private String clName = null;

        public InsertThread(String csName, String clName) {
            super();
            this.csName = csName;
            this.clName = clName;
        }

        @ExecuteOrder(step = 1, desc = "插入记录")
        public void insertRecord() {
            try {
                System.out.println(this.getClass().getName().toString() + " start at:" + df.format(new Date()));
                DBCollection cl = db.getCollectionSpace(csName).getCollection(clName);
                for (int i = insertNum; i < insertNum * 2; i++) {
                    cl.insert("{id:" + i + ",a:'fulltext15872" + i + "',b:'fulltext15872" + i + "'}");
                    AtomicInteger numInCL = (AtomicInteger) insertNumInCLMap.get(csName + "_" + clName);
                    numInCL.incrementAndGet();
                }
                System.out.println(this.getClass().getName().toString() + " stop at:" + df.format(new Date()));
            } finally {
                db.close();
            }
        }

    }

    private class UpdateThread {
        private Sequoiadb db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
        private String csName = null;
        private String clName = null;

        public UpdateThread(String csName, String clName) {
            super();
            this.csName = csName;
            this.clName = clName;
        }

        @ExecuteOrder(step = 1, desc = "更新所有记录")
        public void updateRecord() {
            try {
                System.out.println(this.getClass().getName().toString() + " start at:" + df.format(new Date()));
                DBCollection cl = db.getCollectionSpace(csName).getCollection(clName);
                cl.update(null, "{$set:{b:'update_15796'}}", null);
                System.out.println(this.getClass().getName().toString() + " stop at:" + df.format(new Date()));
            } finally {
                db.close();
            }
        }

    }

    private class DeleteThread {
        private Sequoiadb db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
        private String csName = null;
        private String clName = null;

        public DeleteThread(String csName, String clName) {
            super();
            this.csName = csName;
            this.clName = clName;
        }

        @ExecuteOrder(step = 1, desc = "删除所有记录")
        public void deleteRecord() {
            try {
                System.out.println(this.getClass().getName().toString() + " start at:" + df.format(new Date()));
                DBCollection cl = db.getCollectionSpace(csName).getCollection(clName);
                for (int i = 0; i < insertNum * 2; i++) {
                    cl.delete("{id:" + i + "}", "{'':'id'}");
                    AtomicInteger numInCL = (AtomicInteger) insertNumInCLMap.get(csName + "_" + clName);
                    numInCL.decrementAndGet();
                }

                System.out.println(this.getClass().getName().toString() + " stop at:" + df.format(new Date()));
            } finally {
                db.close();
            }
        }
    }

    private class QueryThread {
        private Sequoiadb db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
        private String csName = null;
        private String clName = null;

        public QueryThread(String csName, String clName) {
            super();
            this.csName = csName;
            this.clName = clName;
        }

        @ExecuteOrder(step = 1, desc = "全文检索全部记录")
        public void queryRecord() {
            DBCursor cursor = null;
            try {
                System.out.println(this.getClass().getName().toString() + " start at:" + df.format(new Date()));
                DBCollection cl = db.getCollectionSpace(csName).getCollection(clName);
                cursor = cl.query("{'':{$Text:{query:{match_all:{}}}}}", "{a:1,b:1}", null, null);
                while (cursor.hasNext()) {
                    cursor.getNext();
                }
                cursor.close();
                System.out.println(this.getClass().getName().toString() + " stop at:" + df.format(new Date()));
            } catch (BaseException e) {
                if (e.getErrorCode() != -6 && e.getErrorCode() != -52) {
                    e.printStackTrace();
                    Assert.fail(e.getMessage());
                }
            } finally {
                db.closeAllCursors();
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
                        + "', b: 'test_11981_aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa " + i * 100 + j
                        + "',c:'text'}"));
            }
            cl.insert(insertObjs, 0);
            insertObjs.clear();
        }
    }
}
