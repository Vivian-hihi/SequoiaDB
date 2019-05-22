package com.sequoiadb.fulltextparallel;

import java.util.ArrayList;
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
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.testcommon.CommLib;
import com.sequoiadb.testcommon.SdbTestBase;
import com.sequoiadb.threadexecutor.ThreadExecutor;
import com.sequoiadb.threadexecutor.annotation.ExecuteOrder;
import com.sequoiadb.threadexecutor.exception.SchException;
import com.sequoiadb.utils.FullTextDBUtils;
import com.sequoiadb.utils.FullTextESUtils;
import com.sequoiadb.utils.FullTextUtils;

/**
 * @FileName: seqDB-15871:并发删除集合
 * @Author zhaoyu
 * @Date 2019-05-13
 */

public class Fulltext15871 extends SdbTestBase {
    private Sequoiadb sdb = null;
    private List<String> csNames = new ArrayList<String>();
    private List<String> clNames = new ArrayList<String>();
    private String csBasicName = "cs15871";
    private String clBasicName = "cl15871";
    private int csNum = 2;
    private int clNum = 2;
    private String indexName = "fulltext15871";
    private Client esClient = null;
    private int insertNum = 50000;
    private DropCLThread dropCL;
    private ThreadExecutor te = new ThreadExecutor(600000);

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

        for (String csName : csNames) {
            if (sdb.isCollectionSpaceExist(csName)) {
                sdb.dropCollectionSpace(csName);
            }
            CollectionSpace cs = sdb.createCollectionSpace(csName);
            for (String clName : clNames) {
                DBCollection cl = cs.createCollection(clName);
                cl.createIndex("id", "{id:1}", false, false);
                insertRecord(cl, insertNum);
                cl.createIndex(indexName, "{a:'text',b:'text'}", false, false);
                dropCL = new DropCLThread(csName, clName);
                te.addWorker(dropCL);
            }
        }

    }

    @AfterClass
    public void tearDown() {
        List<String> esIndexNames = new ArrayList<String>();
        List<String> cappedCLNames = new ArrayList<String>();
        for (String csName : csNames) {
            CollectionSpace cs = sdb.getCollectionSpace(csName);
            for (String clName : clNames) {
                if (cs.isCollectionExist(clName)) {
                    DBCollection cl = cs.getCollection(clName);
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
        System.out.println("esIndexNames:" + esIndexNames + ",cappedCLNames:" + cappedCLNames);
        if (!esIndexNames.isEmpty() && !cappedCLNames.isEmpty()) {
            Assert.assertTrue(FullTextUtils.isIndexDeleted(sdb, esClient, esIndexNames, cappedCLNames));
        }
        sdb.close();
        esClient.close();
    }

    @Test
    public void test() throws Exception {
        // 获取原始集合所在组及固定集合名，作为后续结果校验的输入
        List<String> cappedCLNames = new ArrayList<>();
        List<String> esIndexNames = new ArrayList<>();
        for (String csName : csNames) {
            CollectionSpace cs = sdb.getCollectionSpace(csName);
            for (int i = 0; i < clNum; i++) {
                DBCollection cl = cs.getCollection(clNames.get(i));
                String cappedCLName = FullTextDBUtils.getCappedName(cl, indexName);
                cappedCLNames.add(cappedCLName);
                List<String> esIndexName = FullTextDBUtils.getESIndexNames(cl, indexName);
                esIndexNames.addAll(esIndexName);
            }
        }

        // 执行并发测试
        te.run();

        // 结果校验
        Assert.assertTrue(FullTextUtils.isIndexDeleted(sdb, esClient, esIndexNames, cappedCLNames));

    }

    private class DropCLThread {
        private Sequoiadb db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
        private String csName = null;
        private String clName = null;

        public DropCLThread(String csName, String clName) {
            super();
            this.csName = csName;
            this.clName = clName;
        }

        @ExecuteOrder(step = 1, desc = "删除集合")
        public void dropCLThread() {
            System.out.println("start dropcl thread....");
            db.getCollectionSpace(csName).dropCollection(clName);
            System.out.println("end dropcl thread....");
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
