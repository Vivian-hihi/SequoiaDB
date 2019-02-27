package com.sequoiadb.transaction.ru;

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
import com.sequoiadb.transaction.TransUtils;

/**
 * @Description seqDB-17268 : 插入记录与其他事务中更新前的记录重复
 * @author luweikang
 * @date 2019年1月15日
 */
@Test(groups = "ru")
public class Transaction17268 extends SdbTestBase {

    private String clName = "transCL_17268";
    private Sequoiadb sdb = null;
    private Sequoiadb sdb2 = null;
    private DBCollection cl = null;
    private BSONObject data = null;
    private BSONObject data2 = null;
    private BSONObject updateData = null;
    private BSONObject matcher = null;
    private BSONObject modifier = null;
    private DBCursor recordCur = null;
    private List<BSONObject> expDataList = null;
    private List<BSONObject> actDataList = null;

    @BeforeClass
    public void setUp() {
        sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "");
        sdb2 = new Sequoiadb(SdbTestBase.coordUrl, "", "");
        cl = sdb.getCollectionSpace(csName).createCollection(clName);
        data = new BasicBSONObject();
        data.put("a", 1);
        data.put("b", "testTrans_17268");
        data.put("c", 13700000000L);
        data.put("d", "customer transaction type data application.");
        cl.insert(data);
        cl.createIndex("a", "{a:1}", true, false);
        expDataList = new ArrayList<BSONObject>();
        expDataList.add(data);

        modifier = new BasicBSONObject();
        updateData = new BasicBSONObject();
        updateData.put("_id", "updateId17268");
        updateData.put("a", 17268);
        updateData.put("b", "testTrans_17268Update");
        updateData.put("c", 13700017268L);
        updateData.put("d", "customer transaction type data application.");
        modifier.put("$set", updateData);

        data2 = new BasicBSONObject();
        data2.put("_id", data.get("_id"));
        data2.put("a", "testTrans_17268Insert");
        data2.put("b", 1);
        data2.put("c", 13700000000L);
        data2.put("d", "customer transaction type data application.");

        matcher = new BasicBSONObject("a", new BasicBSONObject("$isnull", 0));
    }

    @Test
    public void test1() {
        sdb.beginTransaction();
        sdb2.beginTransaction();
        try {
            DBCollection transCL1 = sdb.getCollectionSpace(csName).getCollection(clName);
            DBCollection transCL2 = sdb2.getCollectionSpace(csName).getCollection(clName);
            // 1 trans1 update R1 to R2
            transCL1.update(matcher, modifier, null);
            // 2 trans2 insert record R3 same as the R1
            transCL2.insert(data2);
            Assert.fail("insert an existing record with an index,should be failed");
        } catch (BaseException e) {
            Assert.assertEquals(e.getErrorCode(), -38, e.getMessage());
        }
        sdb.rollback();

        recordCur = cl.query("{'a': {'$isnull': 0}}", null, null, "{'': null}");
        actDataList = TransUtils.getReadActList(recordCur);
        Assert.assertEquals(actDataList, expDataList);
        actDataList.clear();

        recordCur = cl.query("{'a': {'$isnull': 0}}", null, null, "{'': 'a'}");
        actDataList = TransUtils.getReadActList(recordCur);
        Assert.assertEquals(actDataList, expDataList);
        actDataList.clear();

    }

    @Test
    public void test2() {
        sdb.beginTransaction();
        sdb2.beginTransaction();
        try {
            DBCollection transCL1 = sdb.getCollectionSpace(csName).getCollection(clName);
            DBCollection transCL2 = sdb2.getCollectionSpace(csName).getCollection(clName);
            // 1 trans1 update R1 to R2
            transCL1.update(matcher, modifier, null);
            // 2 trans2 insert record R3 same as the R1
            transCL2.insert(data2);
            Assert.fail("insert an existing record with an index,should be failed");
        } catch (BaseException e) {
            Assert.assertEquals(e.getErrorCode(), -38, e.getMessage());
        }
        sdb.commit();
        expDataList.clear();
        expDataList.add(updateData);

        recordCur = cl.query("{'a': {'$isnull': 0}}", null, null, "{'': null}");
        actDataList = TransUtils.getReadActList(recordCur);
        Assert.assertEquals(actDataList, expDataList);
        actDataList.clear();

        recordCur = cl.query("{'a': {'$isnull': 0}}", null, null, "{'': 'a'}");
        actDataList = TransUtils.getReadActList(recordCur);
        Assert.assertEquals(actDataList, expDataList);
        actDataList.clear();

        cl.delete("{'a': {'$isnull' :0}}");
        Assert.assertEquals(cl.getCount(), 0);
    }

    @AfterClass
    public void tearDown() {
        try {
            sdb.getCollectionSpace(csName).dropCollection(clName);
        } finally {
            if (recordCur != null) {
                recordCur.close();
            }
            if (sdb != null) {
                sdb.close();
            }
        }
    }

}
