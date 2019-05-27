/**
 * Copyright (c) 2019, SequoiaDB Ltd.
 * File Name:Fulltext12127.java
 * 创建/删除集合与创建/删除全文索引并发 
 *
 *  @author liuxiaoxuan
 * Date:2019年5月10日上午11:33:44
 *  @version 1.00
 */
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

//TODO :集合空间及集合可以定义为List，通过循环这2个list去做对应的操作；整个实现逻辑上会更清晰；请修改后再通知检视
//其他检视意见请参考其他用例15844等
public class Fulltext12127 extends SdbTestBase {
    private Sequoiadb db = null;
    private CollectionSpace cs1 = null;
    private CollectionSpace cs2 = null;
    private DBCollection cl1 = null;
    private DBCollection cl2 = null;
    private DBCollection cl3 = null;
    private DBCollection cl4 = null;

    // TODO :多个集合空间名及多个集合，是否可以定义List来实现？
    private String csName1 = "cs12127_01";
    private String csName2 = "cs12127_02";
    private String clName1 = "ES_12127_cl1_01";
    private String clName2 = "ES_12127_cl1_02";
    private String clName3 = "ES_12127_cl2_01";
    private String clName4 = "ES_12127_cl2_02";
    private String clName5 = "ES_12127_newcl";

    private String textIndexName = "fulltext12127";
    private Client esClient = null;
    ThreadExecutor te = new ThreadExecutor();
    List<DropTextIndexThread> dropIndexThreads = new ArrayList<>();
    List<CreateTextIndexThread> createIndexThreads = new ArrayList<>();
    List<CreateCLThread> createCLThreads = new ArrayList<>();
    List<DropCLThread> dropCLThreads = new ArrayList<>();

    @BeforeClass
    public void setUp() {
        esClient = FullTextESUtils.createTransportClient(esHostName, Integer.parseInt(esServiceName));

        db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
        if (CommLib.isStandAlone(db)) {
            throw new SkipException("skip StandAlone");
        }

        // create cl
        if (db.isCollectionSpaceExist(csName1)) {
            db.dropCollectionSpace(csName1);
        }
        if (db.isCollectionSpaceExist(csName2)) {
            db.dropCollectionSpace(csName2);
        }
        cs1 = db.createCollectionSpace(csName1);
        cs2 = db.createCollectionSpace(csName2);
        // TODO :多个集合空间及多个集合，是否可以循环创建并插入记录
        createCollections();
    }

    @AfterClass
    public void tearDown() {
        for (DropTextIndexThread thread : dropIndexThreads) {
            if (null != thread) {
                thread.tearDown();
            }
        }
        for (CreateTextIndexThread thread : createIndexThreads) {
            if (null != thread) {
                thread.tearDown();
            }
        }
        for (CreateCLThread thread : createCLThreads) {
            if (null != thread) {
                thread.tearDown();
            }
        }
        for (DropCLThread thread : dropCLThreads) {
            if (null != thread) {
                thread.tearDown();
            }
        }

        db.dropCollectionSpace(csName1);
        db.dropCollectionSpace(csName2);
        db.close();
        esClient.close();
    }

    @Test
    public void test() throws Exception {
        // insert
        FullTextDBUtils.insertData(cl1, 10000);
        FullTextDBUtils.insertData(cl2, 10000);
        FullTextDBUtils.insertData(cl3, 10000);
        FullTextDBUtils.insertData(cl4, 10000);
        // create fulltext
        BSONObject indexObj = new BasicBSONObject();
        indexObj.put("a", "text");
        cl1.createIndex(textIndexName, indexObj, false, false);
        cl3.createIndex(textIndexName, indexObj, false, false);
        Assert.assertTrue(FullTextUtils.isIndexCreated(esClient, cl1, textIndexName, 10000));
        Assert.assertTrue(FullTextUtils.isIndexCreated(esClient, cl3, textIndexName, 10000));

        // get capped name
        String cappedName1 = FullTextDBUtils.getCappedName(cl1, textIndexName);
        String cappedName2 = FullTextDBUtils.getCappedName(cl3, textIndexName);
        // get es index name
        List<String> esIndexNames1 = FullTextDBUtils.getESIndexNames(cl1, textIndexName);
        List<String> esIndexNames2 = FullTextDBUtils.getESIndexNames(cl3, textIndexName);

        // drop index
        dropIndexThreads.add(new DropTextIndexThread(csName1, clName1));
        dropIndexThreads.add(new DropTextIndexThread(csName2, clName3));
        // create index
        createIndexThreads.add(new CreateTextIndexThread(csName1, clName2));
        createIndexThreads.add(new CreateTextIndexThread(csName2, clName4));
        // create cl
        createCLThreads.add(new CreateCLThread(csName1, clName5));
        createCLThreads.add(new CreateCLThread(csName2, clName5));
        // drop cl
        dropCLThreads.add(new DropCLThread(csName1, clName1));
        dropCLThreads.add(new DropCLThread(csName2, clName4));

        for (DropTextIndexThread thread : dropIndexThreads) {
            te.addWorker(thread);
        }
        for (CreateTextIndexThread thread : createIndexThreads) {
            te.addWorker(thread);
        }
        for (CreateCLThread thread : createCLThreads) {
            te.addWorker(thread);
        }
        for (DropCLThread thread : dropCLThreads) {
            te.addWorker(thread);
        }

        // concurrent run
        te.run();

        Assert.assertTrue(FullTextUtils.isIndexDeleted(db, esClient, esIndexNames1.get(0), cappedName1));
        Assert.assertTrue(FullTextUtils.isIndexDeleted(db, esClient, esIndexNames2.get(0), cappedName2));
        FullTextUtils.isIndexCreated(esClient, cl2, textIndexName, 10000);

        // query
        BSONObject matcher = (BSONObject) JSON.parse("{'':{'$Text':{'query':{'match':{'a' : 'ES_12127_cl1_021'}}}}}");
        DBCursor cursor = cl2.query(matcher, null, null, null);
        int count = 0;
        while (cursor.hasNext()) {
            BSONObject object = (BSONObject) cursor.getNext();
            count++;
        }
        System.out.println(csName1 + "." + clName2 + "'s count: " + count);
        if (cursor != null) {
            cursor.close();
        }

        // query after drop fulltext
        try {
            matcher = (BSONObject) JSON.parse("{'':{'$Text':{'query':{'match':{'a' : 'ES_12127_cl2_01'}}}}}");
            cursor = cl3.query(matcher, null, null, null);
            Assert.fail("query should fail");
        } catch (BaseException e) {
            Assert.assertEquals(e.getErrorCode(), -52, "actual exception: " + e.getErrorCode());
        } finally {
            if (cursor != null) {
                cursor.close();
            }
        }
    }

    public void createCollections() {
        cl1 = cs1.createCollection(clName1);
        cl2 = cs1.createCollection(clName2);
        cl3 = cs2.createCollection(clName3);
        cl4 = cs2.createCollection(clName4);
    }

    class DropTextIndexThread {
        private Sequoiadb db = null;
        private DBCollection cl = null;

        public DropTextIndexThread(String csName, String clName) {
            db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
            cl = db.getCollectionSpace(csName).getCollection(clName);
        }

        @ExecuteOrder(step = 1, desc = "删除全文索引")
        public void dropTextIndex() {
            System.out.println("--------------run DropTextIndexThread--------------");
            try {
                cl.dropIndex(textIndexName);
            } catch (BaseException e) {
                Assert.assertEquals(e.getErrorCode(), -23, "actual exception: " + e.getErrorCode());
            }
        }

        public void tearDown() {
            if (null != db && !db.isClosed()) {
                db.close();
            }
        }
    }

    class CreateTextIndexThread {
        private Sequoiadb db = null;
        private DBCollection cl = null;

        public CreateTextIndexThread(String csName, String clName) {
            db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
            cl = db.getCollectionSpace(csName).getCollection(clName);
        }

        @ExecuteOrder(step = 1, desc = "创建全文索引")
        public void createTextIndex() {
            System.out.println("--------------run CreateTextIndexThread--------------");
            BSONObject indexObj = new BasicBSONObject();
            indexObj.put("a", "text");
            try {
                cl.createIndex(textIndexName, indexObj, false, false);
            } catch (BaseException e) {
                Assert.assertEquals(e.getErrorCode(), -23, "actual exception: " + e.getErrorCode());
            }
        }

        public void tearDown() {
            if (null != db && !db.isClosed()) {
                db.close();
            }
        }
    }

    class CreateCLThread {
        private Sequoiadb db = null;
        private String csName = null;
        private String clName = null;

        public CreateCLThread(String csName, String clName) {
            db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
            this.csName = csName;
            this.clName = clName;
        }

        @ExecuteOrder(step = 1, desc = "创建集合")
        public void createCL() {
            System.out.println("--------------run CreateCLThread--------------");
            db.getCollectionSpace(csName).createCollection(clName);
        }

        public void tearDown() {
            if (null != db && !db.isClosed()) {
                db.close();
            }
        }
    }

    class DropCLThread extends ResultStore {
        private Sequoiadb db = null;
        private String csName = null;
        private String clName = null;

        public DropCLThread(String csName, String clName) {
            db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
            this.csName = csName;
            this.clName = clName;
        }

        @ExecuteOrder(step = 1, desc = "删除集合")
        public void dropCL() {
            System.out.println("--------------run DropCLThread--------------");
            db.getCollectionSpace(csName).dropCollection(clName);
        }

        public void tearDown() {
            if (null != db && !db.isClosed()) {
                db.close();
            }
        }
    }
}
