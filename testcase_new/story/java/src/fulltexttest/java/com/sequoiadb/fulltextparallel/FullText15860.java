package com.sequoiadb.fulltextparallel;

import java.util.ArrayList;
import java.util.List;
import java.util.Random;

import org.bson.BSONObject;
import org.bson.BasicBSONObject;
import org.bson.types.ObjectId;
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
import com.sequoiadb.base.DBLob;
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
 * @FileName FullText15860.java 集合中存在全文索引，并发增删改/全文检索/查询/truncate记录/lob操作
 * @Author luweikang
 * @Date 2019年5月10日
 */
public class FullText15860 extends SdbTestBase {

    private Sequoiadb sdb = null;
    private CollectionSpace cs = null;
    private DBCollection cl = null;
    private String clName = "es_15860";
    private Client esClient = null;
    private String indexName = "fulltextIndex15860";
    private int insertNum = 20000;
    private long lobSize = 1024 * 1024 * 10;
    private List<ObjectId> lobTruncateList = new ArrayList<ObjectId>();
    private List<ObjectId> lobRemoveList = new ArrayList<ObjectId>();
    private List<ObjectId> lobReadList = new ArrayList<ObjectId>();

    @BeforeClass
    public void setUp() {
        esClient = FullTextESUtils.createTransportClient(esHostName, Integer.parseInt(esServiceName));
        sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "");
        if (CommLib.isStandAlone(sdb)) {
            throw new SkipException("skip StandAlone");
        }
        cs = sdb.getCollectionSpace(csName);
        cl = cs.createCollection(clName);

        List<ObjectId> lobList = writeLob(cl, 100);
        lobTruncateList.addAll(lobList.subList(0, 49));
        lobRemoveList.addAll(lobList.subList(50, 69));
        lobReadList.addAll(lobList.subList(70, 99));
    }

    @Test
    public void test() throws Exception {

        FullTextDBUtils.insertData(cl, insertNum);

        BSONObject indexObj = new BasicBSONObject();
        indexObj.put("a", "text");
        indexObj.put("b", "text");
        indexObj.put("c", "text");
        indexObj.put("d", "text");
        indexObj.put("e", "text");
        cl.createIndex(indexName, indexObj, false, false);

        Assert.assertTrue(FullTextUtils.isIndexCreated(esClient, cl, indexName, insertNum));

        ThreadExecutor thread = new ThreadExecutor();
        thread.addWorker(new QueryByTextIndexThread());
        thread.addWorker(new InsertThread());
        thread.addWorker(new UpdateThread());
        thread.addWorker(new DeleteThread());
        thread.addWorker(new TruncateThread());
        thread.addWorker(new TruncateLobThread());
        thread.addWorker(new PutLobThread());
        thread.addWorker(new RemoveLobThread());
        thread.addWorker(new GetLoBThread());
        thread.run();

        Assert.assertTrue(FullTextUtils.isIndexCreated(esClient, cl, indexName, 0));

        // TODO:插入记录及lob、全文检索，检查集合做基本操作功能正确
    }

    @AfterClass
    public void tearDown() {
        try {
            FullTextDBUtils.dropCollection(cs, clName);
        } finally {
            if (sdb != null) {
                sdb.close();
            }
            if (esClient != null) {
                esClient.close();
            }
        }
    }

    private class InsertThread {

        @ExecuteOrder(step = 1)
        private void insert() throws InterruptedException {
            try (Sequoiadb db = new Sequoiadb(SdbTestBase.coordUrl, "", "")) {
                DBCollection cl = db.getCollectionSpace(csName).getCollection(clName);
                List<BSONObject> insertObjs = new ArrayList<BSONObject>();
                String strB = StringUtils.getRandomString(8);
                String strC = StringUtils.getRandomString(32);
                String strD = StringUtils.getRandomString(64);
                String strE = StringUtils.getRandomString(128);
                int insertNum1 = insertNum + 1000;
                for (int i = 0; i < 10; i++) {
                    for (int j = insertNum; j < insertNum1; j++) {
                        int recordNum = i * 1000 + j;
                        insertObjs.add((BSONObject) JSON.parse("{id: " + recordNum + ", a: '" + clName + recordNum
                                + "', b: '" + strB + "', c: '" + strC + "', d: '" + strD + "', e: '" + strE + "'}"));
                    }
                    cl.insert(insertObjs, 0);
                    insertObjs.clear();
                }
            } catch (BaseException e) {
                Assert.assertEquals(e.getErrorCode(), -321, e.getMessage());
            }
        }
    }

    private class UpdateThread {

        @ExecuteOrder(step = 1)
        private void update() throws InterruptedException {
            try (Sequoiadb db = new Sequoiadb(SdbTestBase.coordUrl, "", "")) {
                DBCollection cl = db.getCollectionSpace(csName).getCollection(clName);
                cl.update("{a: {$gte: 0, $lt: " + (insertNum / 2) + "}}", "{$set: {b: 'text'}}", null);
            } catch (BaseException e) {
                Assert.assertEquals(e.getErrorCode(), -321, e.getMessage());
            }
        }
    }

    private class DeleteThread {

        @ExecuteOrder(step = 1)
        private void delete() throws InterruptedException {
            try (Sequoiadb db = new Sequoiadb(SdbTestBase.coordUrl, "", "")) {
                DBCollection cl = db.getCollectionSpace(csName).getCollection(clName);
                cl.delete("{a: {$gte: " + (insertNum / 2) + ", $lt: " + insertNum + "}}");
            } catch (BaseException e) {
                Assert.assertEquals(e.getErrorCode(), -321, e.getMessage());
            }
        }
    }

    private class TruncateThread {

        @ExecuteOrder(step = 1)
        private void truncate() throws InterruptedException {
            try (Sequoiadb db = new Sequoiadb(SdbTestBase.coordUrl, "", "")) {
                DBCollection cl = db.getCollectionSpace(csName).getCollection(clName);
                cl.truncate();
            } catch (BaseException e) {
                if (e.getErrorCode() != -147 && e.getErrorCode() != -190) {
                    e.printStackTrace();
                    Assert.fail(e.getMessage());
                }
            }
        }
    }

    private class QueryByTextIndexThread {

        @ExecuteOrder(step = 1)
        private void queryData() throws InterruptedException {
            for (int i = 0; i < 10; i++) {
                try (Sequoiadb db = new Sequoiadb(SdbTestBase.coordUrl, "", "")) {
                    Thread.sleep(5500 + new Random().nextInt(500));
                    DBCollection cl = db.getCollectionSpace(csName).getCollection(clName);
                    DBCursor cur = cl.query("{'': {'$Text': {'query': {'match_all': {}}}}}", null, "{'a': 1}",
                            "{'': '" + indexName + "'}");
                    if (cur.hasNext()) {
                        cur.getNext();
                    }
                    cur.close();
                } catch (BaseException e) {
                    if (e.getErrorCode() != -6 && e.getErrorCode() != -52) {
                        e.printStackTrace();
                        Assert.fail(e.getMessage());
                    }
                }
            }
        }
    }

    private class TruncateLobThread {

        @ExecuteOrder(step = 1)
        private void truncateLob() throws InterruptedException {
            try (Sequoiadb db = new Sequoiadb(SdbTestBase.coordUrl, "", "")) {
                Thread.sleep(1000 + new Random().nextInt(100));
                DBCollection cl = db.getCollectionSpace(csName).getCollection(clName);
                for (ObjectId lobId : lobTruncateList) {
                    cl.truncateLob(lobId, lobSize);
                }
            } catch (BaseException e) {
                if (e.getErrorCode() != -4 && e.getErrorCode() != -321) {
                    e.printStackTrace();
                    Assert.fail(e.getMessage());
                }
            }
        }
    }

    private class PutLobThread {

        @ExecuteOrder(step = 1)
        private void putLob() throws InterruptedException {
            try (Sequoiadb db = new Sequoiadb(SdbTestBase.coordUrl, "", "")) {
                Thread.sleep(1000 + new Random().nextInt(100));
                DBCollection cl = db.getCollectionSpace(csName).getCollection(clName);
                writeLob(cl, 100);
            }
        }
    }

    private class RemoveLobThread {

        @ExecuteOrder(step = 1)
        private void removeLob() throws InterruptedException {
            try (Sequoiadb db = new Sequoiadb(SdbTestBase.coordUrl, "", "")) {
                Thread.sleep(1000 + new Random().nextInt(100));
                DBCollection cl = db.getCollectionSpace(csName).getCollection(clName);
                for (ObjectId lobId : lobRemoveList) {
                    cl.removeLob(lobId);
                }
            } catch (BaseException e) {
                if (e.getErrorCode() != -4 && e.getErrorCode() != -321) {
                    e.printStackTrace();
                    Assert.fail(e.getMessage());
                }
            }
        }
    }

    private class GetLoBThread {

        @ExecuteOrder(step = 1)
        private void getLob() throws InterruptedException {
            try (Sequoiadb db = new Sequoiadb(SdbTestBase.coordUrl, "", "")) {
                Thread.sleep(1000 + new Random().nextInt(100));
                DBCollection cl = db.getCollectionSpace(csName).getCollection(clName);
                for (ObjectId lobId : lobReadList) {
                    DBLob lob = cl.openLob(lobId);
                    byte[] data = new byte[(int) lobSize];
                    lob.read(data);
                }
            } catch (BaseException e) {
                if (e.getErrorCode() != -4 && e.getErrorCode() != -321) {
                    e.printStackTrace();
                    Assert.fail(e.getMessage());
                }
            }
        }
    }

    private List<ObjectId> writeLob(DBCollection cl, int lobNum) {

        List<ObjectId> lobIdList = new ArrayList<ObjectId>();
        byte[] data = new byte[(int) lobSize];
        new Random().nextBytes(data);

        for (int i = 0; i < lobNum; i++) {
            DBLob lob = cl.createLob();
            lob.write(data);
            lob.close();
            lobIdList.add(lob.getID());
        }

        return lobIdList;
    }

}
