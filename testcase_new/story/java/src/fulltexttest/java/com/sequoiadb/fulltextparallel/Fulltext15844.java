/**
 * Copyright (c) 2019, SequoiaDB Ltd.
 * File Name:Fulltext15844.java
 * 删除全文索引与全文检索并发 
 *
 *  @author liuxiaoxuan
 * Date:2019年5月10日上午11:33:44
 *  @version 1.00
 */
package com.sequoiadb.fulltextparallel;

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

public class Fulltext15844 extends SdbTestBase {
    private Sequoiadb db = null;
    private CollectionSpace cs = null;
    private DBCollection cl = null;
    private String clName = "ES_15844";
    private String textIndexName = "fulltext15844";
    private Client esClient = null;
    ThreadExecutor te = new ThreadExecutor();
    DropTextIndexThread dropIndexThread;
    QueryThread queryThread;

    @BeforeClass
    public void setUp() {
        esClient = FullTextESUtils.createTransportClient(esHostName, Integer.parseInt(esServiceName));

        db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
        if (CommLib.isStandAlone(db)) {
            throw new SkipException("skip StandAlone");
        }

        // create cl
        cs = db.getCollectionSpace(csName);
        cl = cs.createCollection(clName);
    }

    @AfterClass
    public void tearDown() {
        if (null != dropIndexThread) {
            dropIndexThread.tearDown();
        }
        if (null != queryThread) {
            queryThread.tearDown();
        }
        FullTextDBUtils.dropCollection(cs, clName);
        // TODO :放到finally里面关连接
        db.close();
        esClient.close();
    }

    @Test
    public void test() throws Exception {
        // insert
        FullTextDBUtils.insertData(cl, 10000);
        // create fulltext
        BSONObject indexObj = new BasicBSONObject();
        indexObj.put("a", "text");
        cl.createIndex(textIndexName, indexObj, false, false);
        Assert.assertTrue(FullTextUtils.isIndexCreated(esClient, cl, textIndexName, 10000));

        // get capped name
        String cappedName = FullTextDBUtils.getCappedName(cl, textIndexName);
        // get es index name
        List<String> esIndexNames = FullTextDBUtils.getESIndexNames(cl, textIndexName);

        dropIndexThread = new DropTextIndexThread(csName, clName);
        queryThread = new QueryThread(csName, clName);

        te.addWorker(dropIndexThread);
        te.addWorker(queryThread);
        // concurrent run
        te.run();

        // TODO :查询如果要校验结果，放到线程里面去校验，这里拿到返回值，除了打印没什么用，去掉
        int errorcode = queryThread.getRetCode();
        System.out.println("test() errorcode:　" + errorcode);

        Assert.assertTrue(FullTextUtils.isIndexDeleted(db, esClient, esIndexNames.get(0), cappedName));

        // query after drop fulltext
        // TODO :需要校验一下插入；
        DBCursor cursor = null;
        try {
            BSONObject matcher = (BSONObject) JSON
                    .parse("{'':{'$Text':{'query':{'match':{'a' : 'fulltext15844_after_update'}}}}}");
            cursor = cl.query(matcher, null, null, null);
            Assert.fail("query should fail");
        } catch (BaseException e) {
            Assert.assertEquals(e.getErrorCode(), -52, "actual exception: " + e.getErrorCode());
        } finally {
            if (cursor != null) {
                cursor.close();
            }
        }
    }

    class DropTextIndexThread extends ResultStore {
        private Sequoiadb db = null;
        private DBCollection cl = null;

        // TODO :不需要该构造函数
        public DropTextIndexThread(String csName, String clName) {
            db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
            cl = db.getCollectionSpace(csName).getCollection(clName);
        }

        @ExecuteOrder(step = 1, desc = "删除全文索引")
        public void dropTextIndex() {
            System.out.println("--------------run DropTextIndexThread--------------");
            cl.dropIndex(textIndexName);
        }

        // TODO :放到finally
        public void tearDown() {
            if (null != db && !db.isClosed()) {
                db.close();
            }
        }
    }

    class QueryThread extends ResultStore {
        private Sequoiadb db = null;
        private DBCollection cl = null;

        // TODO :不需要该构造函数
        public QueryThread(String csName, String clName) {
            db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
            cl = db.getCollectionSpace(csName).getCollection(clName);
        }

        @ExecuteOrder(step = 1, desc = "執行全文检索")
        public void query() {
            System.out.println("--------------run QueryThread insert--------------");
            DBCursor cursor = null;
            // TODO :为什么要带匹配条件？这个条件一定有记录返回吗？
            BSONObject matcher = (BSONObject) JSON
                    .parse("{'':{'$Text':{'query':{'match':{'a' : 'fulltext15844_after_update'}}}}}");
            try {
                cursor = cl.query(matcher, null, null, null);
                // TODO :定义这个count的作用？
                int count = 0;
                while (cursor.hasNext()) {
                    // TODO :定义的变量未使用；
                    BSONObject object = (BSONObject) cursor.getNext();
                    count++;
                }
                System.out.println(csName + "." + clName + "'s count: " + count);
            } catch (BaseException e) {
                saveResult(e.getErrorCode(), e);
                // TODO :实际上这里应该还需要规避-6的错误；
                Assert.assertEquals(e.getErrorCode(), -52, "actual exception: " + e.getErrorCode());
            } finally {
                cursor.close();
            }
        }

        // TODO :放到finally
        public void tearDown() {
            if (null != db && !db.isClosed()) {
                db.close();
            }
        }
    }
}
