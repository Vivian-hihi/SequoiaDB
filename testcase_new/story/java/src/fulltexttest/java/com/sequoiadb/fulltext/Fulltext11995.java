package com.sequoiadb.fulltext;

import java.util.ArrayList;
import java.util.List;

import org.bson.BSONObject;
import org.bson.BasicBSONObject;
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
import com.sequoiadb.testcommon.SdbThreadBase;
import com.sequoiadb.utils.FullTextDBUtils;
import com.sequoiadb.utils.FullTextESUtils;
import com.sequoiadb.utils.FullTextUtils;
import com.sequoiadb.utils.StringUtils;

/**
 * FileName: CreateDropSameIndex11995.java test content: 反复重建删除同一个全文索引
 * 
 * @author liuxiaoxuan
 * @Date 2018.11.20
 */
public class Fulltext11995 extends SdbTestBase {

    private Sequoiadb sdb = null;
    private CollectionSpace cs = null;
    private DBCollection cl = null;
    private String clName = "ES_11995";
    private Client esClient = null;

    @BeforeClass
    public void setUp() {
        esClient = FullTextESUtils.createTransportClient(esHostName, Integer.parseInt(esServiceName));
        sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "");
        if (CommLib.isStandAlone(sdb)) {
            throw new SkipException("skip StandAlone");
        }

        // create cl
        cs = sdb.getCollectionSpace(csName);
        cl = cs.createCollection(clName);
    }

    @AfterClass
    public void tearDown() {
        FullTextDBUtils.dropCollection(cs, clName);
        sdb.close();
        esClient.close();
    }

    @Test
    public void test() throws Exception {
        // insert large datas
        insertData(cl, FullTextUtils.INSERT_NUMS);

        String textIndexName = "fulltext11995";
        BSONObject indexObj = new BasicBSONObject();
        indexObj.put("a", "text");
        indexObj.put("b", "text");
        indexObj.put("c", "text");
        indexObj.put("d", "text");
        indexObj.put("e", "text");
        indexObj.put("f", "text");

        String esIndexName = null;
        String cappedName = null;

        // loop create and drop fulltext while processing origin data
        int doTimes = 10;
        while (--doTimes > 0) {
            cl.createIndex(textIndexName, indexObj, false, false);
            cappedName = FullTextDBUtils.getCappedName(cl, textIndexName);
            esIndexName = FullTextDBUtils.getESIndexName(cl, textIndexName);
            FullTextDBUtils.dropFullTextIndex(cl, textIndexName);
            Assert.assertTrue(FullTextUtils.isIndexDeleted(sdb, esClient, esIndexName, cappedName));
        }

        // create and drop fulltext while processing cappedcl data
        doTimes = 5;
        int newInsertNums = 100000;
        while (--doTimes > 0) {
            cl.createIndex(textIndexName, indexObj, false, false);

            InsertThread insertThread = new InsertThread(newInsertNums);
            DropIndexThread dropIdxThread = new DropIndexThread();
            insertThread.start();
            dropIdxThread.start();

            Assert.assertTrue(insertThread.isSuccess(), insertThread.getErrorMsg());
            Assert.assertTrue(dropIdxThread.isSuccess(), dropIdxThread.getErrorMsg());
            Assert.assertTrue(FullTextUtils.isIndexDeleted(sdb, esClient, esIndexName, cappedName));
        }

        // last time create index
        cl.createIndex(textIndexName, indexObj, false, false);
        // check consistency
        Assert.assertTrue(FullTextUtils.isIndexCreated(esClient, cl, textIndexName, (int) cl.getCount()));

        // last time drop index
        FullTextDBUtils.dropFullTextIndex(cl, textIndexName);
        // check fulltext deleted
        Assert.assertTrue(FullTextUtils.isIndexDeleted(sdb, esClient, esIndexName, cappedName));
    }

    public void insertData(DBCollection cl, int insertNums) {
        List<BSONObject> insertObjs = new ArrayList<>();
        for (int i = 0; i < 100; i++) {
            for (int j = 0; j < insertNums / 100; j++) {
                insertObjs.add((BSONObject) JSON.parse("{a: 'test_11995_" + i * j + "', b: '"
                        + StringUtils.getRandomString(32) + "', c: '" + StringUtils.getRandomString(64) + "', d: '"
                        + StringUtils.getRandomString(64) + "', e: '" + StringUtils.getRandomString(128) + "', f: '"
                        + StringUtils.getRandomString(128) + "'}"));
            }
            cl.insert(insertObjs, 0);
            insertObjs.clear();
        }
    }

    private class InsertThread extends SdbThreadBase {

        int insertNums = 0;

        public InsertThread(int insertNums) {
            this.insertNums = insertNums;
        }

        @Override
        public void exec() throws Exception {
            Sequoiadb db = null;
            DBCollection cl = null;
            db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
            cl = db.getCollectionSpace(csName).getCollection(clName);
            // insert records in cappedCL
            insertData(cl, insertNums);
            db.close();
        }
    }

    private class DropIndexThread extends SdbThreadBase {
        @Override
        public void exec() throws Exception {
            try (Sequoiadb db = new Sequoiadb(SdbTestBase.coordUrl, "", "")) {
                DBCollection cl = db.getCollectionSpace(csName).getCollection(clName);
                String textIndexName = "fulltext11995";
                // drop fulltext
                FullTextDBUtils.dropFullTextIndex(cl, textIndexName);
            }
        }

    }
}
