package com.sequoiadb.fulltext.parallel;

import java.text.SimpleDateFormat;
import java.util.Date;

import org.bson.BSONObject;
import org.bson.BasicBSONObject;
import org.bson.util.JSON;
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
import com.sequoiadb.fulltext.utils.FullTextDBUtils;
import com.sequoiadb.fulltext.utils.FullTextUtils;
import com.sequoiadb.testcommon.CommLib;
import com.sequoiadb.testcommon.SdbTestBase;
import com.sequoiadb.threadexecutor.ResultStore;
import com.sequoiadb.threadexecutor.ThreadExecutor;
import com.sequoiadb.threadexecutor.annotation.ExecuteOrder;

/**
 * @FileName seqDB-15844:删除全文索引与全文检索并发
 * @Author
 * @Date liuxiaoxuan 2019.5.10
 */
public class Fulltext15844 extends SdbTestBase {
    private final int TIMEOUT = 600000;
    private Sequoiadb db = null;
    private CollectionSpace cs = null;
    private DBCollection cl = null;
    private String clName = "ES_15844";
    private String textIndexName = "fulltext15844";
    ThreadExecutor te = new ThreadExecutor(TIMEOUT);

    @BeforeClass
    public void setUp() {

        db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
        if (CommLib.isStandAlone(db)) {
            throw new SkipException("skip StandAlone");
        }

        cs = db.getCollectionSpace(csName);
        cl = cs.createCollection(clName);
        FullTextDBUtils.insertData(cl, 10000);

        BSONObject indexObj = new BasicBSONObject();
        indexObj.put("a", "text");
        cl.createIndex(textIndexName, indexObj, false, false);
    }

    @AfterClass
    public void tearDown() {
        try {
            FullTextDBUtils.dropCollection(cs, clName);
            // TODO :检查固定集合及ES端全文索引是否残留
        } finally {
            if (db != null) {
                db.close();
            }
        }
    }

    @Test
    public void test() throws Exception {
        Assert.assertTrue(FullTextUtils.isIndexCreated(cl, textIndexName, 10000));

        String cappedName = FullTextDBUtils.getCappedName(cl, textIndexName);
        String esIndexName = FullTextDBUtils.getESIndexName(cl, textIndexName);

        DropTextIndexThread dropTextIndexThread = new DropTextIndexThread();
        te.addWorker(dropTextIndexThread);
        te.addWorker(new QueryThread());

        te.run();

        if (dropTextIndexThread.getRetCode() != 0) {
            // 全文索引删除失败，全文检索结果正确
            DBCursor cursor = null;
            try {
                int count = 0;
                BSONObject matcher = (BSONObject) JSON.parse("{'':{'$Text':{'query':{'match_all':{}}}}}");
                cursor = cl.query(matcher, null, null, null);
                while (cursor.hasNext()) {
                    cursor.getNext();
                    count++;
                }
                Assert.assertEquals(count, (int) cl.getCount());
            } finally {
                if (cursor != null) {
                    cursor.close();
                }
            }
        } else {
            // 全文索引被删除，执行全文检索报错
            Assert.assertTrue(FullTextUtils.isIndexDeleted(db, esIndexName, cappedName));
            FullTextDBUtils.insertData(cl, 100);
            DBCursor cursor = null;
            try {
                BSONObject matcher = (BSONObject) JSON.parse("{'':{'$Text':{'query':{'match_all':{}}}}}");
                cursor = cl.query(matcher, null, null, null);
                Assert.fail("query should fail");
            } catch (BaseException e) {
                if (-6 != e.getErrorCode() && -52 != e.getErrorCode()) {
                    e.printStackTrace();
                    Assert.fail(" fulltext query fail ");
                }
            } finally {
                if (cursor != null) {
                    cursor.close();
                }
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
            } catch (BaseException e) {
                if (-147 != e.getErrorCode()) {
                    // TODO :打印出栈信息，或者直接抛异常
                    Assert.fail("actual exception: " + e.getErrorCode());
                }
                saveResult(e.getErrorCode(), e);
            } finally {
                System.out.println(this.getClass().getName().toString() + " end at:"
                        + new SimpleDateFormat("yyyy-MM-dd HH:mm:ss").format(new Date()));
            }

        }
    }

    class QueryThread {

        @ExecuteOrder(step = 1, desc = "執行全文检索")
        public void query() {
            System.out.println(this.getClass().getName().toString() + " begin at:"
                    + new SimpleDateFormat("yyyy-MM-dd HH:mm:ss").format(new Date()));
            Sequoiadb sdb = null;
            DBCursor cursor = null;
            BSONObject matcher = (BSONObject) JSON.parse("{'':{'$Text':{'query':{'match_all':{}}}}}");
            try {
                sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "");
                cursor = cl.query(matcher, null, null, null);
                int count = 0;
                while (cursor.hasNext()) {
                    cursor.getNext();
                    count++;
                }
                Assert.assertEquals(count, (int) cl.getCount());
            } catch (BaseException e) {
                if (-6 != e.getErrorCode() && -52 != e.getErrorCode()) {
                    Assert.fail("actual exception: " + e.getErrorCode());
                }
            } finally {
                if (cursor != null) {
                    cursor.close();
                }
                if (sdb != null) {
                    sdb.close();
                }
                System.out.println(this.getClass().getName().toString() + " end at:"
                        + new SimpleDateFormat("yyyy-MM-dd HH:mm:ss").format(new Date()));
            }
        }
    }
}
