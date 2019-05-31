package com.sequoiadb.fulltextparallel;

import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Date;
import java.util.List;
import java.util.Random;

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
import com.sequoiadb.base.DBLob;
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
import org.elasticsearch.client.*;

/**
 * @FileName seqDB-12122:部分集合上存在全文索引，多个集合同时执行增删改/truncate/lob操作
 * @Author
 * @Date liuxiaoxuan 2019.5.10
 */
public class Fulltext12122 extends SdbTestBase {
    private Sequoiadb db = null;
    private List<CollectionSpace> css = new ArrayList<>();
    private List<DBCollection> cls = new ArrayList<>();
    private List<String> csNames = new ArrayList<>();
    private List<String> clNames = new ArrayList<>();
    private String textIndexName = "fulltext12122";
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
            csNames.add("cs12122_" + i);
            if (db.isCollectionSpaceExist(csNames.get(i))) {
                db.dropCollectionSpace(csNames.get(i));
            }
            css.add(db.createCollectionSpace(csNames.get(i)));
        }
        for (int i = 0; i < 4; i++) {
            clNames.add("12122_cl_" + i);
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
        } finally {
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
        Assert.assertTrue(FullTextUtils.isIndexCreated(esClient, cls.get(0), textIndexName, 10000));
        Assert.assertTrue(FullTextUtils.isIndexCreated(esClient, cls.get(1), textIndexName, 10000));

        TruncateThread truncateThread = new TruncateThread(csNames.get(0), clNames.get(0));
        te.addWorker(truncateThread);
        te.addWorker(new TruncateThread(csNames.get(1), clNames.get(3)));
        te.addWorker(new LobThread(csNames.get(0), clNames.get(0)));
        te.addWorker(new LobThread(csNames.get(1), clNames.get(3)));
        te.addWorker(new CurdThread(csNames.get(0), clNames.get(0)));
        te.addWorker(new CurdThread(csNames.get(1), clNames.get(1)));
        te.addWorker(new CurdThread(csNames.get(0), clNames.get(2)));
        te.addWorker(new CurdThread(csNames.get(1), clNames.get(3)));

        te.run();
        // 当truncate成功后，检查全文索引是否有重建
        if(0 == truncateThread.getRetCode()){
            Assert.assertTrue(FullTextUtils.isFulltextRebuild(esClient, cls.get(0), textIndexName));
        }
        // 检查最终ES端全文索引是否完成同步、原始集合和固定集合主备数据是否一致
        Assert.assertTrue(
                FullTextUtils.isIndexCreated(esClient, cls.get(0), textIndexName, (int) cls.get(0).getCount()));
        Assert.assertTrue(
                FullTextUtils.isIndexCreated(esClient, cls.get(1), textIndexName, (int) cls.get(1).getCount()));

        // 全文检索
        BSONObject matcher = (BSONObject) JSON.parse("{'':{'$Text':{'query':{'match_all':{}}}}}");
        DBCursor cursor = cls.get(1).query(matcher, null, null, null);
        int count = 0;
        while (cursor.hasNext()) {
            cursor.getNext();
            count++;
        }
        Assert.assertEquals(count, (int) cls.get(1).getCount());
    }

    class TruncateThread extends ResultStore {
        private String csName;
        private String clName;

        public TruncateThread(String csName, String clName) {
            this.csName = csName;
            this.clName = clName;
        }

        @ExecuteOrder(step = 1, desc = "清空原始集合")
        public void truncate() {
            System.out.println(this.getClass().getName().toString() + " begin at:"
                    + new SimpleDateFormat("yyyy-MM-dd HH:mm:ss").format(new Date()));
            try (Sequoiadb sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "")) {
                DBCollection cl = sdb.getCollectionSpace(csName).getCollection(clName);
                cl.truncate();
            } catch (BaseException e) {
                if (-321 != e.getErrorCode() && -190 != e.getErrorCode()) {
                    Assert.fail("actual exception: " + e.getErrorCode());
                }
                saveResult( e.getErrorCode(), e );
            } finally {
                System.out.println(this.getClass().getName().toString() + " end at:"
                        + new SimpleDateFormat("yyyy-MM-dd HH:mm:ss").format(new Date()));
            }
        }

    }

    class LobThread {
        private String csName;
        private String clName;

        public LobThread(String csName, String clName) {
            this.csName = csName;
            this.clName = clName;
        }

        @ExecuteOrder(step = 1, desc = "插入lob")
        public void createLob() {
            System.out.println(this.getClass().getName().toString() + " begin at:"
                    + new SimpleDateFormat("yyyy-MM-dd HH:mm:ss").format(new Date()));
            DBLob lob = null;
            try (Sequoiadb sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "")) {
                DBCollection cl = sdb.getCollectionSpace(csName).getCollection(clName);
                String lobStringBuff = StringUtils.getRandomString(new Random().nextInt(1024));
                lob = cl.createLob();
                lob.write(lobStringBuff.getBytes());
            } catch (BaseException e) {
                if (-321 != e.getErrorCode() && -190 != e.getErrorCode()) {
                    Assert.fail("actual exception: " + e.getErrorCode());
                }
            } finally {
                if (lob != null) {
                    lob.close();
                }
                System.out.println(this.getClass().getName().toString() + " end at:"
                        + new SimpleDateFormat("yyyy-MM-dd HH:mm:ss").format(new Date()));
            }
        }
    }

    class CurdThread {
        private Sequoiadb sdb = null;
        private DBCollection cl = null;

        public CurdThread(String csName, String clName) {
            sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "");
            cl = sdb.getCollectionSpace(csName).getCollection(clName);
        }

        @ExecuteOrder(step = 1, desc = "插入记录")
        public void insert() {
            System.out.println(this.getClass().getName().toString() + " insert begin at:"
                    + new SimpleDateFormat("yyyy-MM-dd HH:mm:ss").format(new Date()));
            List<BSONObject> insertObjs = new ArrayList<BSONObject>();
            int insertRecordNum = 10000;
            String strA = StringUtils.getRandomString(64);
            for (int i = 0; i < insertRecordNum; i++) {
                insertObjs.add((BSONObject) JSON.parse("{ a: '" + strA + "', b: 'new_insert_12122_" + i + "'}"));
            }

            try {
                cl.insert(insertObjs, 0);
            } catch (BaseException e) {
                if (-321 != e.getErrorCode() && -190 != e.getErrorCode()) {
                    Assert.fail("actual exception: " + e.getErrorCode());
                }
            } finally {
                System.out.println(this.getClass().getName().toString() + " insert end at:"
                        + new SimpleDateFormat("yyyy-MM-dd HH:mm:ss").format(new Date()));
            }
        }

        @ExecuteOrder(step = 1, desc = "更新记录")
        public void update() {
            System.out.println(this.getClass().getName().toString() + " udpate begin at:"
                    + new SimpleDateFormat("yyyy-MM-dd HH:mm:ss").format(new Date()));
            BSONObject modifier = new BasicBSONObject();
            BSONObject value = new BasicBSONObject();
            BSONObject matcher = new BasicBSONObject();
            BSONObject subMatcher = new BasicBSONObject();
            value.put("a", "12122_after_update");
            modifier.put("$set", value);
            subMatcher.put("$lt", 2000);
            matcher.put("id", subMatcher);

            try {
                cl.update(matcher, modifier, null);
            } catch (BaseException e) {
                if (-321 != e.getErrorCode() && -190 != e.getErrorCode()) {
                    Assert.fail("actual exception: " + e.getErrorCode());
                }
            } finally {
                System.out.println(this.getClass().getName().toString() + " udpate end at:"
                        + new SimpleDateFormat("yyyy-MM-dd HH:mm:ss").format(new Date()));
            }
        }

        @ExecuteOrder(step = 1, desc = "删除记录")
        public void delete() {
            System.out.println(this.getClass().getName().toString() + " delete begin at:"
                    + new SimpleDateFormat("yyyy-MM-dd HH:mm:ss").format(new Date()));
            BSONObject matcher = new BasicBSONObject();
            BSONObject subMatcher = new BasicBSONObject();
            subMatcher.put("$gt", 5000);
            matcher.put("id", subMatcher);

            try {
                cl.delete(matcher);
            } catch (BaseException e) {
                if (-321 != e.getErrorCode() && -190 != e.getErrorCode()) {
                    Assert.fail("actual exception: " + e.getErrorCode());
                }
            } finally {
                System.out.println(this.getClass().getName().toString() + " delete end at:"
                        + new SimpleDateFormat("yyyy-MM-dd HH:mm:ss").format(new Date()));
            }
        }

        @ExecuteOrder(step = 2, desc = "清理环境")
        public void tearDown() {
            if (sdb != null) {
                sdb.close();
            }
        }
    }
}
