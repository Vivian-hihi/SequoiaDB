/**
 * Copyright (c) 2019, SequoiaDB Ltd.
 * File Name:Fulltext15843.java
 * 删除全文索引与增删改记录并发
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
import com.sequoiadb.utils.StringUtils;

/**
 * FileName Fulltext15843.java test content: 删除全文索引与增删改记录并发
 * 
 * @author liuxiaoxuan
 * @Date 2019.05.10
 */
public class Fulltext15843 extends SdbTestBase {
    private Sequoiadb db = null;
    private CollectionSpace cs = null;
    private DBCollection cl = null;
    private String clName = "ES_15843";
    private String textIndexName = "fulltext15843";
    private Client esClient = null;
    ThreadExecutor te = new ThreadExecutor();
    DropTextIndexThread dropIndexThread;
    CurdThread curdThread;

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
        if (null != curdThread) {
            curdThread.tearDown();
        }
        FullTextDBUtils.dropCollection(cs, clName);
        // TODO：放到finally里面去关连接，避免teardown失败连接没有关掉
        db.close();
        esClient.close();
    }

    @Test
    public void test() throws Exception {
        // TODO:1.我觉得像这些注释都没有必要，复杂逻辑的才需要添加注释，其他用例请一并修改
        // TODO:2.1w条记录做并发，请务必确认实际上有并发
        // TODO:需要覆盖到同步原始集合、同步固定集合的记录，可以通过，先插记录后建索引(可能正在同步原始集合)、先建索引后插记录(同步固定集合)去覆盖；
        // insert
        FullTextDBUtils.insertData(cl, 10000);
        // create fulltext
        BSONObject indexObj = new BasicBSONObject();
        indexObj.put("a", "text");
        cl.createIndex(textIndexName, indexObj, false, false);

        // get capped name
        String cappedName = FullTextDBUtils.getCappedName(cl, textIndexName);
        // get es index name
        List<String> esIndexNames = FullTextDBUtils.getESIndexNames(cl, textIndexName);

        dropIndexThread = new DropTextIndexThread(csName, clName);
        curdThread = new CurdThread(csName, clName);

        te.addWorker(dropIndexThread);
        te.addWorker(curdThread);

        te.run();

        int errorcode = dropIndexThread.getRetCode();
        System.out.println("test() errorcode:　" + errorcode);

        if (0 != errorcode) {
            Assert.assertTrue(FullTextUtils.isIndexCreated(esClient, cl, textIndexName, (int) cl.getCount()));

            // query
            BSONObject matcher = (BSONObject) JSON
                    .parse("{'':{'$Text':{'query':{'match':{'a' : 'fulltext15843_after_update'}}}}}");
            DBCursor cursor = cl.query(matcher, null, null, null);
            int count = 0;
            while (cursor.hasNext()) {
                // TODO :定义的变量没有使用，去掉，其他用例存在类似的问题，请一并修改
                // 用例补充了需要插入及全文检索，手工用例已修改，自动化同步修改，其他用例也一并修改吧
                BSONObject object = (BSONObject) cursor.getNext();
                count++;
            }
            System.out.println(csName + "." + clName + "'s count: " + count);
        } else {
            Assert.assertTrue(FullTextUtils.isIndexDeleted(db, esClient, esIndexNames.get(0), cappedName));

            // query after drop fulltext
            DBCursor cursor = null;
            try {
                BSONObject matcher = (BSONObject) JSON
                        .parse("{'':{'$Text':{'query':{'match':{'a' : 'fulltext15843_after_update'}}}}}");
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
    }

    class DropTextIndexThread extends ResultStore {
        private Sequoiadb db = null;
        private DBCollection cl = null;

        // TODO:一个集合空间及集合的情况下，不需要这个构造函数，cl的连接通过全局定义的csName及clName去在具体的方法中获取
        // 其他用例如果有类似的情况，请一并修改
        public DropTextIndexThread(String csName, String clName) {
            db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
            cl = db.getCollectionSpace(csName).getCollection(clName);
        }

        @ExecuteOrder(step = 1, desc = "删除全文索引")
        public void dropTextIndex() {
            // TODO:打印如下信息：System.out.println(this.getClass().getName().toString()
            // + " start at:" + df.format(new Date()));
            // TODO:注释统一使用中文吧
            System.out.println("--------------run DropTextIndexThread--------------");
            // when processing cappedcl data, maybe return -147
            try {
                cl.dropIndex(textIndexName);
            } catch (BaseException e) {
                // TODO:isExpectedErr变量不需要定义，统一使用之前说的那种方式吧，逻辑上便于理解，线程中assert，
                // 如果不使用throw的方式，一定要将异常打印出来，否则无法定位到具体的代码行，增加了定位问题的难度
                boolean isExpectedErr = false;
                if (-147 == e.getErrorCode() || -190 == e.getErrorCode()) {
                    isExpectedErr = true;
                    saveResult(e.getErrorCode(), e);
                }
                Assert.assertTrue(isExpectedErr, "actual exception: " + e.getErrorCode());
            }

        }

        // TODO：建议放到finally里面，不需要单独写一个方法来关闭连接
        public void tearDown() {
            if (null != db && !db.isClosed()) {
                db.close();
            }
        }
    }

    class CurdThread {
        private Sequoiadb db = null;
        private DBCollection cl = null;

        public CurdThread(String csName, String clName) {
            db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
            cl = db.getCollectionSpace(csName).getCollection(clName);
        }

        @ExecuteOrder(step = 1, desc = "往原始集合插入数据")
        public void insert() {
            System.out.println("--------------run CurdThread insert--------------");
            List<BSONObject> insertObjs = new ArrayList<BSONObject>();
            int insertRecordNum = 10000;
            String strA = StringUtils.getRandomString(64);
            for (int i = 0; i < insertRecordNum; i++) {
                insertObjs.add((BSONObject) JSON.parse("{ a: '" + strA + "', b: 'new_insert_15843_" + i + "'}"));
            }
            cl.insert(insertObjs, 0);
        }

        @ExecuteOrder(step = 1, desc = "更新全文索引记录")
        public void update() {
            System.out.println("--------------run CurdThread update--------------");
            BSONObject modifier = new BasicBSONObject();
            BSONObject value = new BasicBSONObject();
            BSONObject matcher = new BasicBSONObject();
            BSONObject subMatcher = new BasicBSONObject();
            value.put("a", "fulltext15843_after_update");
            modifier.put("$set", value);
            subMatcher.put("$lt", 500);
            matcher.put("id", subMatcher);
            cl.update(matcher, modifier, null);
        }

        @ExecuteOrder(step = 1, desc = "删除全文索引记录")
        public void delete() {
            System.out.println("--------------run CurdThread delete--------------");
            BSONObject matcher = new BasicBSONObject();
            BSONObject subMatcher = new BasicBSONObject();
            subMatcher.put("$gt", 5000);
            matcher.put("id", subMatcher);
            cl.delete(matcher);
        }

        public void tearDown() {
            if (null != db && !db.isClosed()) {
                db.close();
            }
        }
    }
}
