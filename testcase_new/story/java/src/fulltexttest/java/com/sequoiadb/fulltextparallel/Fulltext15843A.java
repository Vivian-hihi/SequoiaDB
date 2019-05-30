package com.sequoiadb.fulltextparallel;

import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Date;
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
 * FileName Fulltext15843A.java test content: 在ES正在同步原始集合中的记录时，删除全文索引与增删改记录并发
 * 
 * @author liuxiaoxuan
 * @Date 2019.05.10
 */
public class Fulltext15843A extends SdbTestBase {
    private Sequoiadb db = null;
    private CollectionSpace cs = null;
    private DBCollection cl = null;
    private String clName = "ES_15843_A";
    private String textIndexName = "fulltext15843A";
    private Client esClient = null;
    ThreadExecutor te = new ThreadExecutor();

    @BeforeClass
    public void setUp() {
        esClient = FullTextESUtils.createTransportClient(esHostName, Integer.parseInt(esServiceName));

        db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
        if (CommLib.isStandAlone(db)) {
            throw new SkipException("skip StandAlone");
        }

        cs = db.getCollectionSpace(csName);
        cl = cs.createCollection(clName);
        FullTextDBUtils.insertData(cl, 100000);

        BSONObject indexObj = new BasicBSONObject();
        indexObj.put("a", "text");
        cl.createIndex(textIndexName, indexObj, false, false);
    }

    @AfterClass
    public void tearDown() {
        FullTextDBUtils.dropCollection(cs, clName);
        if (db != null) {
            db.close();
        }
        if (esClient != null) {
            esClient.close();
        }
    }

    @Test
    public void test() throws Exception {
        String cappedName = FullTextDBUtils.getCappedName(cl, textIndexName);
        List<String> esIndexNames = FullTextDBUtils.getESIndexNames(cl, textIndexName);

        te.addWorker(new DropTextIndexThread());
        te.addWorker(new CurdThread());

        te.run();

        Assert.assertTrue(FullTextUtils.isIndexDeleted(db, esClient, esIndexNames.get(0), cappedName));

        FullTextDBUtils.insertData(cl, 100);

        // 删除全文索引后，执行全文检索报错
        DBCursor cursor = null;
        try {
            BSONObject matcher = (BSONObject) JSON.parse("{'':{'$Text':{'query':{'match_all':{}}}}}");
            cursor = cl.query(matcher, null, null, null);
            Assert.fail("query should fail");
        } catch (BaseException e) {
            if (-6 != e.getErrorCode() && -52 != e.getErrorCode()) {
                Assert.fail("actual exception: " + e.getErrorCode());
            }
        } finally {
            if (cursor != null) {
                cursor.close();
            }
        }
    }

    class DropTextIndexThread extends ResultStore {

        @ExecuteOrder(step = 1, desc = "删除全文索引")
        public void dropTextIndex() {
            System.out.println(this.getClass().getName().toString() + " begin at:"
                    + new SimpleDateFormat("yyyy-MM-dd HH:mm:ss").format(new Date()));
            try (Sequoiadb sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "")) {
                DBCollection cl = sdb.getCollectionSpace(csName).getCollection(clName);
                cl.dropIndex(textIndexName);
            } finally {
                System.out.println(this.getClass().getName().toString() + " end at:"
                        + new SimpleDateFormat("yyyy-MM-dd HH:mm:ss").format(new Date()));
            }
        }
    }

    class CurdThread {
        private Sequoiadb sdb = null;
        private DBCollection cl = null;

        public CurdThread() {
            sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "");
            cl = sdb.getCollectionSpace(csName).getCollection(clName);
        }

        @ExecuteOrder(step = 1, desc = "往原始集合插入数据")
        public void insert() {
            System.out.println(this.getClass().getName().toString() + " insert begin at:"
                    + new SimpleDateFormat("yyyy-MM-dd HH:mm:ss").format(new Date()));
            List<BSONObject> insertObjs = new ArrayList<BSONObject>();
            int insertRecordNum = 10000;
            String strA = StringUtils.getRandomString(64);
            for (int i = 0; i < insertRecordNum; i++) {
                insertObjs.add((BSONObject) JSON.parse("{ a: '" + strA + "', b: 'new_insert_15843_" + i + "'}"));
            }
            cl.insert(insertObjs, 0);
            System.out.println(this.getClass().getName().toString() + " insert end at:"
                    + new SimpleDateFormat("yyyy-MM-dd HH:mm:ss").format(new Date()));
        }

        @ExecuteOrder(step = 1, desc = "更新全文索引记录")
        public void update() {
            System.out.println(this.getClass().getName().toString() + " update begin at:"
                    + new SimpleDateFormat("yyyy-MM-dd HH:mm:ss").format(new Date()));
            BSONObject modifier = new BasicBSONObject();
            BSONObject value = new BasicBSONObject();
            BSONObject matcher = new BasicBSONObject();
            BSONObject subMatcher = new BasicBSONObject();
            value.put("a", "fulltext15843_after_update");
            modifier.put("$set", value);
            subMatcher.put("$lt", 500);
            matcher.put("id", subMatcher);
            cl.update(matcher, modifier, null);
            System.out.println(this.getClass().getName().toString() + " update end at:"
                    + new SimpleDateFormat("yyyy-MM-dd HH:mm:ss").format(new Date()));
        }

        @ExecuteOrder(step = 1, desc = "删除全文索引记录")
        public void delete() {
            System.out.println(this.getClass().getName().toString() + " delete begin at:"
                    + new SimpleDateFormat("yyyy-MM-dd HH:mm:ss").format(new Date()));
            BSONObject matcher = new BasicBSONObject();
            BSONObject subMatcher = new BasicBSONObject();
            subMatcher.put("$gt", 5000);
            matcher.put("id", subMatcher);
            cl.delete(matcher);
            System.out.println(this.getClass().getName().toString() + " delete end at:"
                    + new SimpleDateFormat("yyyy-MM-dd HH:mm:ss").format(new Date()));
        }

        @ExecuteOrder(step = 2, desc = "清理环境")
        public void tearDown() {
            if (sdb != null) {
                sdb.close();
            }
        }
    }
}
