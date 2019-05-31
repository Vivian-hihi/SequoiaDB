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
import com.sequoiadb.threadexecutor.ThreadExecutor;
import com.sequoiadb.threadexecutor.annotation.ExecuteOrder;
import com.sequoiadb.utils.FullTextDBUtils;
import com.sequoiadb.utils.FullTextESUtils;
import com.sequoiadb.utils.FullTextUtils;

/**
 * @FileName seqDB-12127:创建/删除集合与创建/删除全文索引并发
 * @Author
 * @Date liuxiaoxuan 2019.5.10
 */
public class Fulltext12127 extends SdbTestBase {
    private Sequoiadb db = null;
    private List<CollectionSpace> css = new ArrayList<>();
    private List<DBCollection> cls = new ArrayList<>();
    private List<String> csNames = new ArrayList<>();
    private List<String> clNames = new ArrayList<>();
    private String textIndexName = "fulltext12127";
    private Client esClient = null;
    ThreadExecutor te = new ThreadExecutor();

    @BeforeClass
    public void setUp() {
        esClient = FullTextESUtils.createTransportClient(esHostName, Integer.parseInt(esServiceName));

        db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
        if (CommLib.isStandAlone(db)) {
            throw new SkipException("skip StandAlone");
        }

        // 创建集合空间和集合，总共两个集合空间，每个集合空间对应2个集合
        for (int i = 0; i < 2; i++) {
            csNames.add("cs12127_" + i);
            if (db.isCollectionSpaceExist(csNames.get(i))) {
                db.dropCollectionSpace(csNames.get(i));
            }
            css.add(db.createCollectionSpace(csNames.get(i)));
        }
        for (int i = 0; i < 4; i++) {
            clNames.add("12127_cl_" + i);
            cls.add(css.get(i % 2).createCollection(clNames.get(i)));
        }

        // 插入数据并创建全文索引
        for (DBCollection cl : cls) {
            FullTextDBUtils.insertData(cl, 10000);
        }

        BSONObject indexObj = new BasicBSONObject();
        indexObj.put("a", "text");
        cls.get(0).createIndex(textIndexName, indexObj, false, false);
        cls.get(1).createIndex(textIndexName, indexObj, false, false);
    }

    @AfterClass
    public void tearDown() {
        try {
            for (String csName : csNames) {
                db.dropCollectionSpace(csName);
            }
        }finally {
            if (db != null) {
                db.close();
            }
            if (esClient != null) {
                esClient.close();
            }
        }  
    }

    @Test
    public void test() throws Exception {
        String cappedName1 = FullTextDBUtils.getCappedName(cls.get(0), textIndexName);
        String cappedName2 = FullTextDBUtils.getCappedName(cls.get(1), textIndexName);
        List<String> esIndexNames1 = FullTextDBUtils.getESIndexNames(cls.get(0), textIndexName);
        List<String> esIndexNames2 = FullTextDBUtils.getESIndexNames(cls.get(1), textIndexName);

        te.addWorker(new DropTextIndexThread(csNames.get(0), clNames.get(0)));
        te.addWorker(new DropTextIndexThread(csNames.get(1), clNames.get(1)));
        te.addWorker(new CreateTextIndexThread(csNames.get(0), clNames.get(2)));
        te.addWorker(new CreateTextIndexThread(csNames.get(1), clNames.get(3)));
        te.addWorker(new CreateCLThread(csNames.get(0), "12127_cl_4"));
        te.addWorker(new CreateCLThread(csNames.get(1), "12127_cl_5"));
        te.addWorker(new DropCLThread(csNames.get(0), clNames.get(0)));
        te.addWorker(new DropCLThread(csNames.get(1), clNames.get(3)));

        te.run();

        Assert.assertTrue(FullTextUtils.isIndexDeleted(db, esClient, esIndexNames1.get(0), cappedName1));
        Assert.assertTrue(FullTextUtils.isIndexDeleted(db, esClient, esIndexNames2.get(0), cappedName2));
        FullTextUtils.isIndexCreated(esClient, cls.get(2), textIndexName, 10000);

        // 全文检索
        BSONObject matcher = (BSONObject) JSON.parse("{'':{'$Text':{'query':{'match_all':{}}}}}");
        DBCursor cursor = cls.get(2).query(matcher, null, null, null);
        int count = 0;
        while (cursor.hasNext()) {
            cursor.getNext();
            count++;
        }
        if (cursor != null) {
            cursor.close();
        }
        Assert.assertEquals(count, (int) cls.get(2).getCount());

        FullTextDBUtils.insertData(cls.get(1), 100);

        // 在删除全文索引后查询，会报错
        try {
            cursor = cls.get(1).query(matcher, null, null, null);
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

    class DropTextIndexThread {
        private String csName;
        private String clName;

        public DropTextIndexThread(String csName, String clName) {
            this.csName = csName;
            this.clName = clName;
        }

        @ExecuteOrder(step = 1, desc = "删除全文索引")
        public void dropTextIndex() {
            System.out.println(this.getClass().getName().toString() + " begin at:"
                    + new SimpleDateFormat("yyyy-MM-dd HH:mm:ss").format(new Date()));
            try (Sequoiadb sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "")) {
                DBCollection cl = sdb.getCollectionSpace(csName).getCollection(clName);
                cl.dropIndex(textIndexName);
            } catch (BaseException e) {
                Assert.assertEquals(e.getErrorCode(), -23, "actual exception: " + e.getErrorCode());
            } finally {
                System.out.println(this.getClass().getName().toString() + " end at:"
                        + new SimpleDateFormat("yyyy-MM-dd HH:mm:ss").format(new Date()));
            }
        }
    }

    class CreateTextIndexThread {
        private String csName;
        private String clName;

        public CreateTextIndexThread(String csName, String clName) {
            this.csName = csName;
            this.clName = clName;
        }

        @ExecuteOrder(step = 1, desc = "创建全文索引")
        public void createTextIndex() {
            System.out.println(this.getClass().getName().toString() + " begin at:"
                    + new SimpleDateFormat("yyyy-MM-dd HH:mm:ss").format(new Date()));

            try (Sequoiadb sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "")) {
                DBCollection cl = sdb.getCollectionSpace(csName).getCollection(clName);
                BSONObject indexObj = new BasicBSONObject();
                indexObj.put("a", "text");
                cl.createIndex(textIndexName, indexObj, false, false);
            } catch (BaseException e) {
                Assert.assertEquals(e.getErrorCode(), -23, "actual exception: " + e.getErrorCode());
            } finally {
                System.out.println(this.getClass().getName().toString() + " end at:"
                        + new SimpleDateFormat("yyyy-MM-dd HH:mm:ss").format(new Date()));
            }
        }
    }

    class CreateCLThread {
        private String csName = null;
        private String clName = null;

        public CreateCLThread(String csName, String clName) {
            this.csName = csName;
            this.clName = clName;
        }

        @ExecuteOrder(step = 1, desc = "创建集合")
        public void createCL() {
            System.out.println(this.getClass().getName().toString() + " begin at:"
                    + new SimpleDateFormat("yyyy-MM-dd HH:mm:ss").format(new Date()));
            try (Sequoiadb sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "")) {
                sdb.getCollectionSpace(csName).createCollection(clName);
            } finally {
                System.out.println(this.getClass().getName().toString() + " end at:"
                        + new SimpleDateFormat("yyyy-MM-dd HH:mm:ss").format(new Date()));
            }
        }
    }

    class DropCLThread {
        private String csName = null;
        private String clName = null;

        public DropCLThread(String csName, String clName) {
            db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
            this.csName = csName;
            this.clName = clName;
        }

        @ExecuteOrder(step = 1, desc = "删除集合")
        public void dropCL() {
            System.out.println(this.getClass().getName().toString() + " begin at:"
                    + new SimpleDateFormat("yyyy-MM-dd HH:mm:ss").format(new Date()));
            try (Sequoiadb sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "")) {
                sdb.getCollectionSpace(csName).dropCollection(clName);
            } finally {
                System.out.println(this.getClass().getName().toString() + " end at:"
                        + new SimpleDateFormat("yyyy-MM-dd HH:mm:ss").format(new Date()));
            }
        }
    }
}
