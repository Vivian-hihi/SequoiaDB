package com.sequoiadb.transaction.rc;

import java.util.ArrayList;
import java.util.List;

import org.bson.BSONObject;
import org.bson.BasicBSONObject;
import org.testng.Assert;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.DBCursor;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.testcommon.SdbTestBase;
import com.sequoiadb.testcommon.SdbThreadBase;
import com.sequoiadb.transaction.TransUtils;

/**
 * @Description Transaction17137.java 创建/删除索引与事务操作并发
 * @author luweikang
 * @date 2019年1月15日
 */
@Test(groups = "rc")
public class Transaction17137A extends SdbTestBase {

    private String clName = "transCL_17137A";
    private Sequoiadb sdb = null;
    private Sequoiadb sdb1 = null;
    private Sequoiadb sdb2 = null;
    private DBCollection cl = null;
    private int recordNum = 20000;
    private DBCursor recordCur = null;
    private List<BSONObject> rs1 = null;
    private List<BSONObject> rs2 = null;
    private List<BSONObject> expDataList = null;
    private List<BSONObject> actDataList = null;

    @BeforeClass
    public void setUp() {
        sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "");
        cl = sdb.getCollectionSpace(csName).createCollection(clName);
        expDataList = prepareData(recordNum);
        rs1 = expDataList.subList(0, recordNum/2);
        rs2 = expDataList.subList(recordNum/2, recordNum);
        cl.insert(rs1);

    }

    @Test
    public void test() {
        sdb1 = new Sequoiadb(SdbTestBase.coordUrl, "", "");
        sdb2 = new Sequoiadb(SdbTestBase.coordUrl, "", "");

        sdb1.beginTransaction();
        sdb2.beginTransaction();
        
        CRUDThread crudThread = new CRUDThread();
        crudThread.start();

        IndexThread indexThread = new IndexThread();
        indexThread.start();

        QueryThread queryThread = new QueryThread();
        queryThread.start();

        Assert.assertTrue(crudThread.isSuccess(), crudThread.getErrorMsg());
        Assert.assertTrue(indexThread.isSuccess(), indexThread.getErrorMsg());
        Assert.assertTrue(queryThread.isSuccess(), queryThread.getErrorMsg());

        sdb1.commit();
        Assert.assertTrue(cl.isIndexExist("a"));

        expDataList.clear();
        expDataList = expData();

        recordCur = cl.query(null, "{'_id': {'$include': 0}}", null, "{'': null}");
        actDataList = TransUtils.getReadActList(recordCur);
        Assert.assertEquals(actDataList, expDataList);
        actDataList.clear();

        recordCur = cl.query(null, "{'_id': {'$include': 0}}", null, "{'': 'a'}");
        actDataList = TransUtils.getReadActList(recordCur);
        Assert.assertEquals(actDataList, expDataList);
        actDataList.clear();

        sdb2.commit();
        
        recordCur = cl.query(null, "{'_id': {'$include': 0}}", null, "{'': null}");
        actDataList = TransUtils.getReadActList(recordCur);
        Assert.assertEquals(actDataList, expDataList);
        actDataList.clear();

        recordCur = cl.query(null, "{'_id': {'$include': 0}}", null, "{'': 'a'}");
        actDataList = TransUtils.getReadActList(recordCur);
        Assert.assertEquals(actDataList, expDataList);
        actDataList.clear();
        
    }

    @AfterClass
    public void tearDown() {
        if (sdb1 != null) {
            sdb1.close();
        }
        if (sdb2 != null) {
            sdb2.close();
        }
        if (recordCur != null) {
            recordCur.close();
        }
        sdb.getCollectionSpace(csName).dropCollection(clName);
        if (sdb != null) {
            sdb.close();
        }
    }

    private List<BSONObject> prepareData(int recordNum) {
        List<BSONObject> dataList = new ArrayList<BSONObject>();
        for (int i = 0; i < recordNum; i++) {
            BSONObject data = new BasicBSONObject();
            data.put("a", i);
            data.put("b", "testTrans_17137_" + i);
            data.put("c", 13700000000L);
            data.put("d", "customer transaction type data application.");
            dataList.add(data);
        }
        return dataList;
    }

    private List<BSONObject> expData() {
        List<BSONObject> dataList = new ArrayList<BSONObject>();
        BSONObject data = null;
        for (int i = 0; i < recordNum / 2; i++) {
            data = new BasicBSONObject();
            data.put("a", 1024);
            data.put("b", "test_update_1024");
            data.put("c", 13700000000L);
            data.put("d", "customer transaction type data application.");
            dataList.add(data);
        }
        return dataList;
    }

    private class CRUDThread extends SdbThreadBase {

        public void exec() {
            DBCollection cl1 = sdb1.getCollectionSpace(csName).getCollection(clName);
            cl1.insert(rs2);

            String modifier = "{'$set':{ 'a': 1024, 'b': 'test_update_1024'}}";
            cl1.update("{'a':{'$gte':0, '$lt': " + recordNum / 2 + "}}", modifier, "{'': null}");

            cl1.delete("{'a':{'$gte':" + recordNum / 2 + ", '$lt': " + recordNum + "}}", "{'': null}");
        }
    }

    private class IndexThread extends SdbThreadBase {

        public void exec() {
            try (Sequoiadb db = new Sequoiadb(SdbTestBase.coordUrl, "", "")) {
                DBCollection dbcl = db.getCollectionSpace(csName).getCollection(clName);
                for (int i = 0; i < 100; i++) {
                    dbcl.createIndex("a", "{a:1, b:-1}", false, false);
                    dbcl.dropIndex("a");
                }
                dbcl.createIndex("a", "{a:1, b:-1}", false, false);
            }
        }
    }

    private class QueryThread extends SdbThreadBase {

        public void exec() {
            DBCursor cur = null;
            try {
                for (int i = 0; i < 10; i++) {
                    DBCollection dbcl = sdb2.getCollectionSpace(csName).getCollection(clName);
                    cur = dbcl.query(null, null, "{a :1}", "{'': null}");
                    List<BSONObject> actList = TransUtils.getReadActList(cur);
                    Assert.assertEquals(actList, rs1, "select times: " +i);
                    actList.clear();
                    
                    try {
                        cur = dbcl.query(null, null, "{a :1}", "{'': 'a'}");
                        actList = TransUtils.getReadActList(cur);
                        Assert.assertEquals(actList, rs1, "select times: " +i);
                    } catch (BaseException e) {
                        int actErrCode = e.getErrorCode();
                        if(actErrCode != -48 && actErrCode != -52 && actErrCode != -10){
                            e.printStackTrace();
                            throw e;
                        }
                    }
                }
            } finally {
                if (cur != null) {
                    cur.close();
                }
            }
        }
    }

}