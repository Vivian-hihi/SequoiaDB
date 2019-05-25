package com.sequoiadb.fulltextparallel;

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
import com.sequoiadb.utils.FullTextDBUtils;
import com.sequoiadb.utils.FullTextESUtils;
import com.sequoiadb.utils.FullTextUtils;

/**
 * @testcase seqDB-12116:同一集合并发创建相同的全文索引
 * @date 2019-4-30
 * @author yinzhen
 *
 */
public class FullText12116 extends SdbTestBase {
    private static final String CLNAME = "cl12116";
    private ThreadExecutor thExecutor = new ThreadExecutor(600000);
    private Sequoiadb sdb;
    private DBCollection cl;
    private String fullIdxName = "idx12116";
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
        // TODO： 数据量不要沿用大数据的数据量来测试，从覆盖测试点及用例执行效率来考虑，可以根据并发的情况自己定一个数据量，建议至少2w吧
        FullTextDBUtils.insertData(cl, FullTextUtils.INSERT_NUMS);
    }

    @Test
    public void test() throws Exception {
        thExecutor.addWorker(new DropFullIdxTh1());
        thExecutor.addWorker(new DropFullIdxTh2());
        thExecutor.addWorker(new DropFullIdxTh2());

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
            // TODO： es的连接也要在这里关闭，其他用例需要一并修改了
            sdb.close();
        }
    }

    // TODO:
    // 1.创建同一个全文索引，类名修改一下;
    // 2.是不是定义为private比较好？
    // 3.通过一个类来实现，需要返回线程的执行结果，判断执行结果只有1个成功，更严谨；在并发线程执行完后在test()里面去校验
    // 4.线程里自己创建的db连接自己关闭
    // 5.结果校验可以统一调用公共方法
    // 6.Assert.fail()，需要调用e.print，或者使用throw 抛出来
    class DropFullIdxTh1 {
        private Sequoiadb db;
        private Sequoiadb db2;
        private DBCollection cl;
        private DBCollection cl2;

        private DropFullIdxTh1() {
            db = new Sequoiadb(coordUrl, "", "");
            db2 = new Sequoiadb(coordUrl, "", "");
            cl = db.getCollectionSpace(csName).getCollection(CLNAME);
            cl2 = db2.getCollectionSpace(csName).getCollection(CLNAME);
        }

        @ExecuteOrder(step = 1, desc = "多线程并发创建同一个全文索引")
        private void dropFullIdx() {
            try {
                cl.createIndex(fullIdxName, "{'a':'text','b':'text','c':'text', 'd':'text', 'e':'text', 'f':'text'}",
                        false, false);
            } catch (BaseException e) {
                Assert.assertEquals(e.getErrorCode(), -42);
            }
        }

        @ExecuteOrder(step = 2, desc = "主备节点上索引信息一致，且固定集合中主备节点信息一致，ES端的数据正确")
        private void checkIndex() throws Exception {
            try {
                esIndexName = FullTextDBUtils.getESIndexName(cl, fullIdxName);
                cappedCLName = FullTextDBUtils.getCappedName(cl, fullIdxName);
                Assert.assertTrue(FullTextUtils.isIndexCreated(esClient, cl, fullIdxName, FullTextUtils.INSERT_NUMS));
                checkFullIdx();

                DBCursor dbCursor = cl.query("{}", "{}", "{_id:1}", "{}");
                DBCursor esCursor = cl2.query("{'':{'$Text':{'query':{'match_all':{}}}}}", "{}", "{_id:1}",
                        "{'':'" + fullIdxName + "'}");
                Assert.assertTrue(FullTextUtils.isCLRecordsConsistency(dbCursor, esCursor));
            } finally {
                db.close();
            }
        }
    }

    class DropFullIdxTh2 {
        private Sequoiadb db;
        private DBCollection cl;

        private DropFullIdxTh2() {
            db = new Sequoiadb(coordUrl, "", "");
            cl = db.getCollectionSpace(csName).getCollection(CLNAME);
        }

        @ExecuteOrder(step = 1, desc = "多线程并发创建同一个全文索引")
        private void dropFullIdx() {
            try {
                cl.createIndex(fullIdxName, "{'a':'text','b':'text','c':'text', 'd':'text', 'e':'text', 'f':'text'}",
                        false, false);
            } catch (BaseException e) {
                Assert.assertEquals(e.getErrorCode(), -42);
            } finally {
                db.close();
            }
        }
    }

    // 校验集合的全文索引信息在主备节点是否一致
    private void checkFullIdx() throws InterruptedException {
        List<String> nodeAddrs = CommLib.getNodeAddress(sdb, groupName);
        BSONObject indexInfo = null;
        for (String nodeAddr : nodeAddrs) {
            Sequoiadb data = null;
            try {
                data = new Sequoiadb(nodeAddr, "", "");
                DBCollection cl = data.getCollectionSpace(csName).getCollection(CLNAME);
                int doTimes = 0;
                while (!cl.isIndexExist(fullIdxName)) {
                    doTimes++;
                    Thread.sleep(100);
                    Assert.assertNotEquals(doTimes, 600);
                }
                if (indexInfo == null) {
                    indexInfo = cl.getIndexInfo(fullIdxName);
                    continue;
                }
                Assert.assertEquals(cl.getIndexInfo(fullIdxName), indexInfo);
            } finally {
                data.close();
            }
        }
    }
}