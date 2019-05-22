package com.sequoiadb.fulltext;

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
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.testcommon.CommLib;
import com.sequoiadb.testcommon.SdbTestBase;
import com.sequoiadb.threadexecutor.ThreadExecutor;
import com.sequoiadb.threadexecutor.annotation.ExecuteOrder;
import com.sequoiadb.utils.FullTextDBUtils;
import com.sequoiadb.utils.FullTextESUtils;
import com.sequoiadb.utils.FullTextUtils;

/**
 * @testcase seqDB-15825:同一集合并发删除相同的全文索引
 * @date 2019-4-30
 * @author yinzhen
 *
 */
public class FullText15825 extends SdbTestBase {
    private static final String CLNAME = "cl15825";
    private ThreadExecutor thExecutor = new ThreadExecutor(600000);
    private Sequoiadb sdb;
    private DBCollection cl;
    private String fullIdxName = "idx15825";
    private Client esClient;
    private String groupName;
    private String cappedCLName;
    private String esIndexName;

    @BeforeClass
    public void setUp() {
        sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "");
        if (CommLib.isStandAlone(sdb)) {
            throw new SkipException("STANDALONE MODE");
        }

        esClient = FullTextESUtils.createTransportClient(SdbTestBase.esHostName,
                Integer.parseInt(SdbTestBase.esServiceName));
        groupName = CommLib.getDataGroupNames(sdb).get(0);
        cl = sdb.getCollectionSpace(csName).createCollection(CLNAME,
                (BSONObject) JSON.parse("{Group:'" + groupName + "'}"));
        FullTextDBUtils.insertData(cl, FullTextUtils.INSERT_NUMS);
        cl.createIndex(fullIdxName, "{'a':'text','b':'text','c':'text', 'd':'text', 'e':'text', 'f':'text'}", false,
                false);
    }

    @Test
    public void test() throws Exception {
        thExecutor.addWorker(new DropFullIdx1());
        thExecutor.addWorker(new DropFullIdx2());
        thExecutor.addWorker(new DropFullIdx2());

        thExecutor.run();
        thExecutor.display();
    }

    @AfterClass
    public void tearDown() {
        try {
            CollectionSpace cs = sdb.getCollectionSpace(csName);
            cs.dropCollection(CLNAME);
            Assert.assertTrue(FullTextUtils.isIndexDeleted(sdb, esClient, esIndexName, cappedCLName));
        } finally {
            sdb.close();
        }
    }

    class DropFullIdx1 {
        private Sequoiadb db;
        private DBCollection cl;

        private DropFullIdx1() {
            db = new Sequoiadb(coordUrl, "", "");
            cl = db.getCollectionSpace(csName).getCollection(CLNAME);
        }

        @ExecuteOrder(step = 1, desc = "多线程并发删除已存在的同一个全文索引")
        private void dropFullIdx() {
            try {
                esIndexName = FullTextDBUtils.getESIndexName(cl, fullIdxName);
                cappedCLName = FullTextDBUtils.getCappedName(cl, fullIdxName);
                cl.dropIndex(fullIdxName);
            } catch (BaseException e) {
                Assert.assertEquals(e.getErrorCode(), -47);
            }
        }

        @ExecuteOrder(step = 2, desc = "索引信息被删除，同时固定集合删除成功，主备节点数据一致，ES端最终无索引数据")
        private void checkIndex() throws InterruptedException {
            try {
                Assert.assertTrue(FullTextUtils.isIndexDeleted(sdb, esClient, esIndexName, cappedCLName));
                Assert.assertTrue(FullTextUtils.isCLDataConsistency(cl));
                checkFullIdx();
                FullTextUtils.isCLDataConsistency(cl);
            } finally {
                db.close();
            }
        }
    }

    class DropFullIdx2 {
        private Sequoiadb db;
        private DBCollection cl;

        private DropFullIdx2() {
            db = new Sequoiadb(coordUrl, "", "");
            cl = db.getCollectionSpace(csName).getCollection(CLNAME);
        }

        @ExecuteOrder(step = 1, desc = "多线程创建删除同一个全文索引")
        private void dropFullIdx() {
            try {
                esIndexName = FullTextDBUtils.getESIndexName(cl, fullIdxName);
                cappedCLName = FullTextDBUtils.getCappedName(cl, fullIdxName);
                cl.dropIndex(fullIdxName);
            } catch (BaseException e) {
                Assert.assertEquals(e.getErrorCode(), -47);
            } finally {
                db.close();
            }
        }
    }

    private void checkFullIdx() throws InterruptedException {
        List<String> nodeAddrs = CommLib.getNodeAddress(sdb, groupName);
        for (String nodeAddr : nodeAddrs) {
            Sequoiadb data = null;
            try {
                data = new Sequoiadb(nodeAddr, "", "");
                DBCollection cl = data.getCollectionSpace(csName).getCollection(CLNAME);
                int doTimes = 0;
                while (cl.isIndexExist(fullIdxName)) {
                    doTimes++;
                    Thread.sleep(100);
                    Assert.assertNotEquals(doTimes, 600);
                }
            } finally {
                data.close();
            }
        }
    }
}