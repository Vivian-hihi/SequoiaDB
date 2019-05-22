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
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.testcommon.CommLib;
import com.sequoiadb.testcommon.SdbTestBase;
import com.sequoiadb.threadexecutor.ThreadExecutor;
import com.sequoiadb.threadexecutor.annotation.ExecuteOrder;
import com.sequoiadb.utils.FullTextDBUtils;
import com.sequoiadb.utils.FullTextESUtils;
import com.sequoiadb.utils.FullTextUtils;
import com.sequoiadb.utils.StringUtils;

/**
 * @testcase seqDB-12124:创建全文索引与增删改记录并发
 * @date 2019-4-28
 * @author yinzhen
 *
 */
public class FullText12124 extends SdbTestBase {
    private static final String CLNAME = "cl12124";
    private ThreadExecutor thExecutor = new ThreadExecutor(600000);
    private Sequoiadb sdb;
    private DBCollection cl;
    private String fullIdxName = "idx12124";
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
        insertData(cl, FullTextUtils.INSERT_NUMS);
    }

    @Test
    public void test() throws Exception {
        thExecutor.addWorker(new CreateFullIdx());
        thExecutor.addWorker(new InsertData());
        thExecutor.addWorker(new UpdateData());
        thExecutor.addWorker(new DeleteData());

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

    class CreateFullIdx {
        private Sequoiadb db;
        private Sequoiadb db2;
        private DBCollection cl;
        private DBCollection cl2;

        private CreateFullIdx() {
            db = new Sequoiadb(coordUrl, "", "");
            db2 = new Sequoiadb(coordUrl, "", "");
            cl = db.getCollectionSpace(csName).getCollection(CLNAME);
            cl2 = db2.getCollectionSpace(csName).getCollection(CLNAME);
        }

        @ExecuteOrder(step = 1, desc = "创建全文索引")
        private void createFullIdx() {
            cl.createIndex(fullIdxName, "{'a':'text','b':'text','c':'text', 'd':'text', 'e':'text', 'f':'text'}", false,
                    false);
            esIndexName = FullTextDBUtils.getESIndexName(cl, fullIdxName);
            cappedCLName = FullTextDBUtils.getCappedName(cl, fullIdxName);
            Assert.assertTrue(cl.isIndexExist(fullIdxName));
        }

        @ExecuteOrder(step = 2, desc = "原集合、固定集合中记录正确且主备节点数据一致，ES中最终同步的记录正确")
        private void checkRecords() throws BaseException, Exception {
            try {
                Assert.assertTrue(FullTextUtils.isIndexCreated(esClient, cl, fullIdxName, (int) cl.getCount()));

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

    class InsertData {
        private Sequoiadb db;
        private DBCollection cl;

        private InsertData() {
            db = new Sequoiadb(coordUrl, "", "");
            cl = db.getCollectionSpace(csName).getCollection(CLNAME);
        }

        @ExecuteOrder(step = 1, desc = "插入包含全文索引字段的记录")
        private void insertRecords() {
            try {
                insertData(cl, 10000);
            } finally {
                db.close();
            }
        }
    }

    class UpdateData {
        private Sequoiadb db;
        private DBCollection cl;

        private UpdateData() {
            db = new Sequoiadb(coordUrl, "", "");
            cl = db.getCollectionSpace(csName).getCollection(CLNAME);
        }

        @ExecuteOrder(step = 1, desc = "更新包含全文索引字段的记录")
        private void updateRecords() {
            try {
                cl.update("{a:'test_12124_0'}", "{$set:{b:'b_12124'}}", "{'':'" + fullIdxName + "'}");
            } finally {
                db.close();
            }
        }
    }

    class DeleteData {
        private Sequoiadb db;
        private DBCollection cl;

        private DeleteData() {
            db = new Sequoiadb(coordUrl, "", "");
            cl = db.getCollectionSpace(csName).getCollection(CLNAME);
        }

        @ExecuteOrder(step = 1, desc = "创建普通索引，删除普通索引")
        private void deleteReocrds() {
            try {
                cl.delete("{a:'test_12124_2'}", "{'':'" + fullIdxName + "'}");
            } finally {
                db.close();
            }
        }
    }

    private void insertData(DBCollection cl, int insertNums) {
        List<BSONObject> records = new ArrayList<BSONObject>();
        for (int i = 0; i < 100; i++) {
            for (int j = 0; j < insertNums / 100; j++) {
                BSONObject record = (BSONObject) JSON.parse("{a: 'test_12124_" + i * j + "', b: '"
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