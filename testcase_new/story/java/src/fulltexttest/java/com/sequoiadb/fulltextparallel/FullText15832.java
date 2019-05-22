package com.sequoiadb.fulltextparallel;

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
 * @testcase seqDB-15832:删除全文索引与删除集合并发
 * @date 2019-4-30
 * @author yinzhen
 *
 */
public class FullText15832 extends SdbTestBase {
    private static final String CLNAME = "cl15832";
    private ThreadExecutor thExecutor = new ThreadExecutor(600000);
    private Sequoiadb sdb;
    private DBCollection cl;
    private String fullIdxName = "idx15832";
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
        esIndexName = FullTextDBUtils.getESIndexName(cl, fullIdxName);
        cappedCLName = FullTextDBUtils.getCappedName(cl, fullIdxName);

        thExecutor.addWorker(new DropFullIdx());
        thExecutor.addWorker(new DropCL());

        thExecutor.run();
        thExecutor.display();
    }

    @AfterClass
    public void tearDown() {
        try {
            CollectionSpace cs = sdb.getCollectionSpace(csName);
            if (cs.isCollectionExist(CLNAME)) {
                cs.dropCollection(CLNAME);
            }
            Assert.assertTrue(FullTextUtils.isIndexDeleted(sdb, esClient, esIndexName, cappedCLName));
        } finally {
            sdb.close();
        }
    }

    class DropFullIdx {
        private Sequoiadb db;
        private DBCollection cl;

        private DropFullIdx() {
            db = new Sequoiadb(coordUrl, "", "");
            cl = db.getCollectionSpace(csName).getCollection(CLNAME);
        }

        @ExecuteOrder(step = 1, desc = "删除全文索引")
        private void createFullIdx() {
            try {
                cl.dropIndex(fullIdxName);
            } catch (BaseException e) {
                Assert.assertEquals(e.getErrorCode(), -23);
            }
        }

        @ExecuteOrder(step = 2, desc = "原始集合及固定集合均被删除成功，ES上全文索引删除成功，主备节点数据一致，无数据文件残留")
        private void checkRecords() throws InterruptedException {
            try {
                Assert.assertFalse(db.getCollectionSpace(csName).isCollectionExist(CLNAME));
                Assert.assertTrue(FullTextUtils.isIndexDeleted(sdb, esClient, esIndexName, cappedCLName));
            } finally {
                db.close();
            }
        }
    }

    class DropCL {
        private Sequoiadb db;

        private DropCL() {
            db = new Sequoiadb(coordUrl, "", "");
        }

        @ExecuteOrder(step = 1, desc = "删除集合")
        private void dropCL() {
            try {
                db.getCollectionSpace(csName).dropCollection(CLNAME);
            } finally {
                db.close();
            }
        }
    }
}