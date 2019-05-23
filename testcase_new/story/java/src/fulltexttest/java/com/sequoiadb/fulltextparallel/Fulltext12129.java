package com.sequoiadb.fulltextparallel;

import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Date;
import java.util.List;

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
 * @FileName: seqDB-12129:并发创建/删除不同集合上的全文索引
 * @Author zhaoyu
 * @Date 2019-05-11
 */

public class Fulltext12129 extends SdbTestBase {
    private Sequoiadb sdb = null;
    private List<String> csNames = new ArrayList<String>();
    private List<String> clNames = new ArrayList<String>();
    private String csBasicName = "cs12129";
    private String clBasicName = "cl12129";
    private int csNum = 2;
    private int clNum = 4;
    private String indexName = "fulltext12129";
    private Client esClient = null;
    private int insertNum = 20000;
    private ThreadExecutor te = new ThreadExecutor(600000);
    private SimpleDateFormat df = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss.S");

    @BeforeClass
    public void setUp() throws SchException {
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
        // 集合空间下一半集合用于创建全文索引，一半集合用于删除全文索引
        for (String csName : csNames) {
            if (sdb.isCollectionSpaceExist(csName)) {
                sdb.dropCollectionSpace(csName);
            }
            CollectionSpace cs = sdb.createCollectionSpace(csName);
            for (int i = 0; i < clNum; i++) {
                DBCollection cl = cs.createCollection(clNames.get(i));
                cl.createIndex("id", "{id:1}", false, false);
                insertRecord(cl, insertNum);
                if (i < clNum / 2) {
                    cl.createIndex(indexName, "{a:'text',b:'text'}", false, false);
                }
            }
        }
    }

    @AfterClass
    public void tearDown() {
        try {
            List<String> esIndexNames = new ArrayList<String>();
            List<String> cappedCLNames = new ArrayList<String>();
            for (String csName : csNames) {
                CollectionSpace cs = sdb.getCollectionSpace(csName);
                for (String clName : clNames) {
                    DBCollection cl = cs.getCollection(clName);
                    if (cl.isIndexExist(indexName)) {
                        String esIndexName = FullTextDBUtils.getESIndexName(cl, indexName);
                        esIndexNames.add(esIndexName);
                        String cappedCLName = FullTextDBUtils.getCappedName(cl, indexName);
                        cappedCLNames.add(cappedCLName);
                    }
                }

            }
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
        // 获取原始集合所在组及固定集合名，作为后续结果校验的输入
        List<String> cappedCLNames = new ArrayList<>();
        List<String> esIndexNames = new ArrayList<>();
        for (String csName : csNames) {
            CollectionSpace cs = sdb.getCollectionSpace(csName);
            for (int i = 0; i < clNum / 2; i++) {
                DBCollection cl = cs.getCollection(clNames.get(i));
                String cappedCLName = FullTextDBUtils.getCappedName(cl, indexName);
                cappedCLNames.add(cappedCLName);
                List<String> esIndexName = FullTextDBUtils.getESIndexNames(cl, indexName);
                esIndexNames.addAll(esIndexName);
            }
        }

        // 执行并发测试
        for (String csName : csNames) {
            for (int i = 0; i < clNum; i++) {
                if (i < clNum / 2) {
                    te.addWorker(new DropFullIndexThread(csName, clNames.get(i)));
                } else {
                    te.addWorker(new CreateFullIndexThread(csName, clNames.get(i)));
                }
            }
        }
        te.run();

        // 结果校验
        for (String csName : csNames) {
            CollectionSpace cs = sdb.getCollectionSpace(csName);
            for (int i = 0; i < clNum; i++) {
                DBCollection cl = cs.getCollection(clNames.get(i));
                if (i < clNum / 2) {
                    // 固定集合及ES端的全文索引已删除成功
                    Assert.assertTrue(
                            FullTextUtils.isIndexDeleted(sdb, esClient, esIndexNames.get(i), cappedCLNames.get(i)));

                    // 全文检索数据报错-52
                    try {
                        cl.query("{'':{'$Text':{query:{match_all:{}}}}}", "{a:1,c:1}", null, null);
                    } catch (BaseException e) {
                        if (e.getErrorCode() != -6 && e.getErrorCode() != -52) {
                            Assert.fail(e.getMessage());
                        }
                    }
                    System.out.println(
                            "check cs:" + csName + " cl:" + clNames.get(i) + " fulltext in ES and data group success.");

                } else {
                    // 同步符合预期
                    Assert.assertTrue(FullTextUtils.isIndexCreated(esClient, cl, indexName, insertNum));

                    // 全文检索数据符合预期
                    DBCursor cursor = cl.query("{'':{'$Text':{query:{match_all:{}}}}}", "{a:1,c:1}", null, null);
                    int actualRecordNum = 0;
                    while (cursor.hasNext()) {
                        cursor.getNext();
                        actualRecordNum++;
                    }
                    Assert.assertEquals(actualRecordNum, insertNum);

                    // 插入记录
                    insertRecord(cl, insertNum);

                    // 同步符合预期
                    Assert.assertTrue(FullTextUtils.isIndexCreated(esClient, cl, indexName, insertNum * 2));
                    System.out.println("check cs:" + csName + " cl:" + clNames.get(i)
                            + " data group consistency and sync to es success.");

                }

            }
        }

    }

    private class DropFullIndexThread {
        private Sequoiadb db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
        private String csName = null;
        private String clName = null;

        public DropFullIndexThread(String csName, String clName) {
            super();
            this.csName = csName;
            this.clName = clName;
        }

        @ExecuteOrder(step = 1, desc = "删除全文索引")
        public void dropFullIndex() {
            System.out.println(this.getClass().getName().toString() + " start at:" + df.format(new Date()));
            DBCollection cl = db.getCollectionSpace(csName).getCollection(clName);
            cl.dropIndex(indexName);
            System.out.println(this.getClass().getName().toString() + " stop at:" + df.format(new Date()));
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
            DBCollection cl = db.getCollectionSpace(csName).getCollection(clName);
            cl.createIndex(indexName, "{a:'text',b:'text'}", false, false);
            System.out.println(this.getClass().getName().toString() + " stop at:" + df.format(new Date()));
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
