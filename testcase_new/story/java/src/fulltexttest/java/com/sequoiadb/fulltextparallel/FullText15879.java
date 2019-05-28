package com.sequoiadb.fulltextparallel;

import java.util.ArrayList;
import java.util.Date;
import java.util.Random;

import org.bson.BSONObject;
import org.bson.BasicBSONObject;
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
import com.sequoiadb.threadexecutor.ResultStore;
import com.sequoiadb.threadexecutor.ThreadExecutor;
import com.sequoiadb.threadexecutor.annotation.ExecuteOrder;
import com.sequoiadb.utils.FullTextDBUtils;
import com.sequoiadb.utils.FullTextESUtils;
import com.sequoiadb.utils.FullTextUtils;

/**
 * @FileName seqDB-15879:truncate集合记录与split并发
 * @Author huangxiaoni
 * @Date 2019.5.14
 */

public class FullText15879 extends SdbTestBase {
    private Random random = new Random();
    private final String CL_NAME = "cl_es_15879";
    private final String IDX_NAME = "idx_es_15879";
    private final BSONObject IDX_KEY = new BasicBSONObject("a", "text");
    private final int RECS_NUM = 20000;

    private Sequoiadb sdb = null;
    private CollectionSpace cs;
    private DBCollection cl;
    private String cappedCSName;
    private String srcRgName;
    private String dstRgName;

    private Client esClient = null;
    private String esIndexName;
    private int lid;

    @BeforeClass
    private void setUp() throws Exception {
        esClient = FullTextESUtils.createTransportClient(esHostName, Integer.parseInt(esServiceName));
        sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "");

        if (CommLib.isStandAlone(sdb) || CommLib.OneGroupMode(sdb)) {
            throw new SkipException("The mode is standlone, or only one group, skip the testCase.");
        }

        ArrayList<String> rgNames = CommLib.getDataGroupNames(sdb);
        srcRgName = rgNames.get(0);
        dstRgName = rgNames.get(1);

        cs = sdb.getCollectionSpace(SdbTestBase.csName);
        BSONObject options = new BasicBSONObject();
        options.put("ShardingType", "hash");
        options.put("ShardingKey", new BasicBSONObject("a", 1));
        options.put("Group", srcRgName);
        cl = cs.createCollection(CL_NAME, options);
        cl.createIndex(IDX_NAME, IDX_KEY, false, false);
        cappedCSName = FullTextDBUtils.getCappedName(cl, IDX_NAME);
        esIndexName = FullTextDBUtils.getESIndexName(cl, IDX_NAME);

        FullTextDBUtils.insertData(cl, RECS_NUM);

        // 确保预置的数据同步到es完成，避免test中查询的数据未同步完成导致非预期
        Assert.assertTrue(FullTextUtils.isIndexCreated(esClient, cl, IDX_NAME, RECS_NUM));
        lid = FullTextESUtils.getCommitCLLIDFromES(esClient, esIndexName);
    }

    @Test
    private void test() throws Exception {
        ThreadExecutor es = new ThreadExecutor();
        ThreadTruncate threadTruncate = new ThreadTruncate();
        ThreadSplit threadSplit = new ThreadSplit();
        es.addWorker(threadTruncate);
        es.addWorker(threadSplit);
        es.run();

        // check results
        // TODO :线程的执行结果相互不影响，建议在线程中去校验每个线程的执行结果，这样逻辑上会更清晰
        int expRecsNum = 0;
        if (threadTruncate.getRetCode() == 0) {
            Assert.assertTrue(FullTextUtils.isFulltextRebuild(esClient, esIndexName, lid));
        } else if (threadTruncate.getRetCode() != 0) {
            expRecsNum = RECS_NUM;
        }

        // TODO :切分执行完后，需要校验切分后数据节点及ES端的全文索引数据
        int expRgNum = 0;
        if (threadSplit.getRetCode() == 0) {
            expRgNum = 2;
        }
        Assert.assertTrue(FullTextUtils.isIndexCreated(esClient, cl, IDX_NAME, expRecsNum));
        Assert.assertEquals(FullTextDBUtils.getCLGroups(cl).size(), expRgNum);
    }

    @AfterClass
    private void tearDown() throws InterruptedException {
        try {
            FullTextDBUtils.dropCollection(cs, CL_NAME);
            Assert.assertTrue(FullTextUtils.isIndexDeleted(sdb, esClient, esIndexName, cappedCSName));
        } finally {
            if (sdb != null) {
                sdb.close();
            }
            if (esClient != null) {
                esClient.close();
            }
        }
    }

    private class ThreadTruncate extends ResultStore {
        @ExecuteOrder(step = 1)
        private void truncate() throws InterruptedException {
            Thread.sleep(random.nextInt(1000));
            try (Sequoiadb db = new Sequoiadb(SdbTestBase.coordUrl, "", "")) {
                DBCollection cl = db.getCollectionSpace(SdbTestBase.csName).getCollection(CL_NAME);
                System.out.println(new Date() + " begin " + this.getClass().getName().toString());
                cl.truncate();
                System.out.println(new Date() + " end   " + this.getClass().getName().toString());
            } catch (BaseException e) {
                if (e.getErrorCode() != -190 && e.getErrorCode() != -147) {
                    throw e;
                }
                saveResult(-1, e);
            }
        }
    }

    private class ThreadSplit extends ResultStore {
        @ExecuteOrder(step = 1)
        private void split() {
            try (Sequoiadb db = new Sequoiadb(SdbTestBase.coordUrl, "", "")) {
                DBCollection cl = db.getCollectionSpace(SdbTestBase.csName).getCollection(CL_NAME);
                System.out.println(new Date() + " begin " + this.getClass().getName().toString());
                cl.split(srcRgName, dstRgName, 50);
                System.out.println(new Date() + " end   " + this.getClass().getName().toString());
            } catch (BaseException e) {
                if (e.getErrorCode() != -321) {
                    throw e;
                }
                saveResult(-1, e);
            }
        }
    }
}