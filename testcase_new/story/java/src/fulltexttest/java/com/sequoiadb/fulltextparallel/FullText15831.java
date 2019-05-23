package com.sequoiadb.fulltextparallel;

import org.bson.BSONObject;
import org.bson.util.JSON;
import org.elasticsearch.client.Client;
import org.testng.Assert;
import org.testng.SkipException;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

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
//TODO：其他检视意见同 15826 用例
/**
 * @testcase seqDB-15831:创建全文索引与删除集合空间并发
 * @date 2019-4-30
 * @author yinzhen
 *
 */
public class FullText15831 extends SdbTestBase {
    private static final String CLNAME = "cl15831";
    private ThreadExecutor thExecutor = new ThreadExecutor(600000);
    private Sequoiadb sdb;
    private DBCollection cl;
    private String fullIdxName = "idx15831";
    private String csName = "cs15831";
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
        cl = sdb.createCollectionSpace(csName).createCollection(CLNAME,
                (BSONObject) JSON.parse("{Group:'" + groupName + "'}"));
        FullTextDBUtils.insertData(cl, FullTextUtils.INSERT_NUMS);
    }

    @Test 
    public void test() throws Exception {
        thExecutor.addWorker(new CreateFullIdx());
        thExecutor.addWorker(new DropCS());

        thExecutor.run();
        thExecutor.display();
    }

    @AfterClass
    public void tearDown() {
        try {
            if (sdb.isCollectionSpaceExist(csName)) {
                sdb.dropCollectionSpace(csName);
            }
            if (cappedCLName != null && esIndexName != null) {
                Assert.assertTrue(FullTextUtils.isIndexDeleted(sdb, esClient, esIndexName, cappedCLName));
            }
        } finally {
            sdb.close();
        }
    }

    class CreateFullIdx {
        private Sequoiadb db;
        private DBCollection cl;

        private CreateFullIdx() {
            db = new Sequoiadb(coordUrl, "", "");
            cl = db.getCollectionSpace(csName).getCollection(CLNAME);
        }
      //TODO:同  15830 用例
        @ExecuteOrder(step = 1, desc = "创建全文索引")
        private void createFullIdx() {
            try {
                cl.createIndex(fullIdxName, "{'a':'text','b':'text','c':'text', 'd':'text', 'e':'text', 'f':'text'}",
                        false, false);
                esIndexName = FullTextDBUtils.getESIndexName(cl, fullIdxName);
                cappedCLName = FullTextDBUtils.getCappedName(cl, fullIdxName);
            } catch (BaseException e) {
                if (e.getErrorCode() != -248 && e.getErrorCode() != -23) {//TODO:是不是还会报-34
                    Assert.fail(e.getMessage());
                }
            }
        }

        @ExecuteOrder(step = 2, desc = "原始集合空间及固定集合均被删除成功，ES上全文索引删除成功，主备节点数据一致，无数据文件残留")
        private void checkRecords() throws InterruptedException {
            try {
                Assert.assertFalse(db.isCollectionSpaceExist(csName));
                if (cappedCLName != null && esIndexName != null) {
                    Assert.assertTrue(FullTextUtils.isIndexDeleted(sdb, esClient, esIndexName, cappedCLName));
                }
            } finally {
                db.close();
            }
        }
    }

    class DropCS {
        private Sequoiadb db;

        private DropCS() {
            db = new Sequoiadb(coordUrl, "", "");
        }

        @ExecuteOrder(step = 1, desc = "删除集合空间")
        private void dropCS() {
            try {
                db.dropCollectionSpace(csName);
            } finally {
                db.close();
            }
        }
    }
}