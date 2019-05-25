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
import com.sequoiadb.base.DBCursor;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.testcommon.CommLib;
import com.sequoiadb.testcommon.SdbTestBase;
import com.sequoiadb.threadexecutor.ThreadExecutor;
import com.sequoiadb.threadexecutor.annotation.ExecuteOrder;
import com.sequoiadb.utils.FullTextDBUtils;
import com.sequoiadb.utils.FullTextESUtils;
import com.sequoiadb.utils.FullTextUtils;
import com.sequoiadb.utils.StringUtils;

/**
 * @testcase seqDB-12120:并发删除同一条记录
 * @date 2019-4-28
 * @author yinzhen
 *
 */
public class FullText12120 extends SdbTestBase {
    private static final String CLNAME = "cl12120";
    private ThreadExecutor thExecutor = new ThreadExecutor(600000);
    private Sequoiadb sdb;
    private DBCollection cl;
    private String fullIdxName = "idx12120";
    private Client esClient;
    private String esIndexName;
    private String cappedCLName;

    @BeforeClass
    public void setUp() {
        sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "");
        if (CommLib.isStandAlone(sdb)) {
            throw new SkipException("STANDALONE MODE");
        }

        esClient = FullTextESUtils.createTransportClient(SdbTestBase.esHostName,
                Integer.parseInt(SdbTestBase.esServiceName));
        cl = sdb.getCollectionSpace(csName).createCollection(CLNAME);
    }

    @Test
    public void test() throws Exception {
        cl.createIndex(fullIdxName, "{'a':'text','b':'text','c':'text', 'd':'text', 'e':'text', 'f':'text'}", false,
                false);
        esIndexName = FullTextDBUtils.getESIndexName(cl, fullIdxName);
        cappedCLName = FullTextDBUtils.getCappedName(cl, fullIdxName);

        insertData(cl, FullTextUtils.INSERT_NUMS);
        cl.insert("{a:'idx12120', b:'b12120'}");
        Assert.assertTrue(FullTextUtils.isIndexCreated(esClient, cl, fullIdxName, FullTextUtils.INSERT_NUMS + 1));

        thExecutor.addWorker(new UpdateDataTh1());
        thExecutor.addWorker(new UpdateDataTh2());

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

    class UpdateDataTh1 {
        private Sequoiadb db;
        private Sequoiadb db2;
        private DBCollection cl;
        private DBCollection cl2;

        private UpdateDataTh1() {
            db = new Sequoiadb(coordUrl, "", "");
            db2 = new Sequoiadb(coordUrl, "", "");
            cl = db.getCollectionSpace(csName).getCollection(CLNAME);
            cl2 = db2.getCollectionSpace(csName).getCollection(CLNAME);
        }

        @ExecuteOrder(step = 1, desc = "多线程并发更新同一条包含全文索引字段的记录")
        private void updateData() {
            cl.update("{a:'idx12120', b:'b12120'}", "{$set:{a:'a12120'}}", "{'':'" + fullIdxName + "'}");
        }

        @ExecuteOrder(step = 2, desc = "es中数据与原集合数据一致")
        private void checkRecords() throws Exception {
            try {
                Assert.assertTrue(
                        FullTextUtils.isIndexCreated(esClient, cl, fullIdxName, FullTextUtils.INSERT_NUMS + 1));
                DBCollection cappedCL = FullTextDBUtils.getCappedCLs(cl, fullIdxName).get(0);
                List<BSONObject> records = FullTextDBUtils.getRecordsFromCL(cappedCL.query());
                Assert.assertEquals(records.get(0).get("Type"), 3);

                DBCursor dbCursor = cl.query("{}", "{}", "{_id:1}", "{}");
                DBCursor esCursor = cl2.query("{'':{'$Text':{'query':{'match_all':{}}}}}", "{}", "{_id:1}",
                        "{'':'" + fullIdxName + "'}");
                Assert.assertTrue(FullTextUtils.isCLRecordsConsistency(dbCursor, esCursor));
            } finally {
                db.closeAllCursors();
                db2.closeAllCursors();
                db.close();
                db2.close();
            }
        }
    }

    class UpdateDataTh2 {
        private Sequoiadb db;
        private DBCollection cl;

        private UpdateDataTh2() {
            db = new Sequoiadb(coordUrl, "", "");
            cl = db.getCollectionSpace(csName).getCollection(CLNAME);
        }

        @ExecuteOrder(step = 1, desc = "多线程并发更新同一条包含全文索引字段的记录")
        private void updateData() {
            try {
                cl.update("{a:'idx12120', b:'b12120'}", "{$set:{a:'a12120'}}", "{'':'" + fullIdxName + "'}");
            } finally {
                db.close();
            }
        }
    }

    private void insertData(DBCollection cl, int insertNums) {
        List<BSONObject> records = new ArrayList<BSONObject>();
        for (int i = 0; i < 100; i++) {
            for (int j = 0; j < insertNums / 100; j++) {
                BSONObject record = (BSONObject) JSON.parse("{a: 'test_12120_" + i * j + "', b: '"
                        + StringUtils.getRandomString(32) + "', c: '" + StringUtils.getRandomString(64) + "', d: '"
                        + StringUtils.getRandomString(64) + "', e: '" + StringUtils.getRandomString(128) + "', f: '"
                        + StringUtils.getRandomString(128) + "'}");
                records.add(record);
            }
            cl.insert(records);
            records.clear();
        }
    }
}