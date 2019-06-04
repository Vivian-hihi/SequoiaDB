package com.sequoiadb.fulltextparallel;

import java.util.concurrent.atomic.AtomicInteger;

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
 * @FileName seqDB-12116:同一集合并发创建相同的全文索引
 * @Author yinzhen
 * @Date 2019-4-30
 */
public class FullText12116 extends SdbTestBase {
    private String clName = "cl12116";
    private Sequoiadb sdb;
    private DBCollection cl;
    private String fullIdxName = "idx12116";
    private Client esClient;
    private String cappedCLName;
    private String esIndexName;
    private AtomicInteger atoint = new AtomicInteger(0);

    @BeforeClass
    public void setUp() {
        sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "");
        if (CommLib.isStandAlone(sdb)) {
            throw new SkipException("STANDALONE MODE");
        }

        esClient = FullTextESUtils.createTransportClient(SdbTestBase.esHostName,
                Integer.parseInt(SdbTestBase.esServiceName));
        cl = sdb.getCollectionSpace(csName).createCollection(clName);
        // TODO :记录数建议定义为一个变量，因为用例中多处有使用到，其他用例类似
        FullTextDBUtils.insertData(cl, 20000);
    }

    @Test
    public void test() throws Exception {
        ThreadExecutor thExecutor = new ThreadExecutor(600000);
        // 并发10个线程，只有一个线程使count+1
        for (int i = 0; i < 10; i++) {
            thExecutor.addWorker(new CreateFullIdx());
        }
        thExecutor.run();
        Assert.assertEquals(atoint.get(), 1);

        // 主备节点上索引信息一致，且固定集合中主备节点信息一致
        esIndexName = FullTextDBUtils.getESIndexName(cl, fullIdxName);
        cappedCLName = FullTextDBUtils.getCappedName(cl, fullIdxName);
        Assert.assertTrue(FullTextUtils.isIndexCreated(esClient, cl, fullIdxName, 20000));

        // ES端的数据正确
        // TODO :需要加注释，为啥定义了2个连接，其他用例类似
        Sequoiadb db2 = new Sequoiadb(SdbTestBase.coordUrl, "", "");
        try {
            DBCollection cl2 = db2.getCollectionSpace(csName).getCollection(clName);
            DBCursor dbCursor = cl.query("{}", "{}", "{_id:1}", "{}");
            DBCursor esCursor = cl2.query("{'':{'$Text':{'query':{'match_all':{}}}}}", "{}", "{_id:1}",
                    "{'':'" + fullIdxName + "'}");
            Assert.assertTrue(FullTextUtils.isCLRecordsConsistency(dbCursor, esCursor));
        } finally {
            if (db2 != null) {
                db2.close();
            }
        }

        // 在db端执行插入、全文检索
        // TODO :这里应该是可以沿用db2吧，其他用例类似
        Sequoiadb db3 = new Sequoiadb(SdbTestBase.coordUrl, "", "");
        try {
            FullTextDBUtils.insertData(cl, 1000);
            Assert.assertEquals(cl.getCount(), 21000);
            Assert.assertTrue(FullTextUtils.isIndexCreated(esClient, cl, fullIdxName, 21000));

            DBCollection cl3 = db3.getCollectionSpace(csName).getCollection(clName);
            DBCursor dbCursor = cl.query("{}", "{}", "{_id:1}", "{}");
            DBCursor esCursor = cl3.query("{'':{'$Text':{'query':{'match_all':{}}}}}", "{}", "{_id:1}",
                    "{'':'" + fullIdxName + "'}");
            Assert.assertTrue(FullTextUtils.isCLRecordsConsistency(dbCursor, esCursor));
        } finally {
            if (db3 != null) {
                db3.close();
            }
        }
    }

    @AfterClass
    public void tearDown() throws Exception {
        try {
            CollectionSpace cs = sdb.getCollectionSpace(csName);
            FullTextDBUtils.dropCollection(cs, clName);
            Assert.assertTrue(FullTextUtils.isIndexDeleted(sdb, esClient, esIndexName, cappedCLName));
        } finally {
            if (sdb != null) {
                sdb.close();
            }
            if (esClient != null) {
                esClient.close();
            }
        }
    }

    private class CreateFullIdx {
        @ExecuteOrder(step = 1, desc = "多线程并发创建同一个全文索引")
        private void createFullIdx() {
            Sequoiadb db = null;
            try {
                db = new Sequoiadb(coordUrl, "", "");
                DBCollection cl = db.getCollectionSpace(csName).getCollection(clName);
                cl.createIndex(fullIdxName, "{'a':'text','b':'text','c':'text', 'd':'text', 'e':'text', 'f':'text'}",
                        false, false);
                atoint.incrementAndGet();
            } catch (BaseException e) {
                // TODO :使用了assert，失败时需要将栈信息打印出来，其他用例需同步修改
                Assert.assertEquals(e.getErrorCode(), -42);
            } finally {
                if (db != null) {
                    db.close();
                }
            }
        }
    }
}