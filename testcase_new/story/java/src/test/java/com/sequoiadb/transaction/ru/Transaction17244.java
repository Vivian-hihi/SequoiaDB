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
 * @Description Transaction17244.java 插入记录与本事务中更新的记录重复
 * @author luweikang
 * @date 2019年1月15日
 */
@Test(groups = "ru")
public class Transaction17244 extends SdbTestBase {

    private String clName = "transCL_17244";
    private Sequoiadb sdb = null;
    private DBCollection cl = null;
    private BSONObject data = null;
    private BSONObject UpdateData = null;
    private DBCursor recordCur = null;
    private List<BSONObject> expDataList = null;
    private List<BSONObject> actDataList = null;

    @BeforeClass
    public void setUp() {
        sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "");
        cl = sdb.getCollectionSpace(csName).createCollection(clName);
        cl.createIndex("a", "{a: 1}", true, false);
        data = new BasicBSONObject();
        data.put("a", 1);
        data.put("b", "testTrans_17244");
        data.put("c", 13700000000L);
        data.put("d", "customer transaction type data application.");
        cl.insert(data);

        UpdateData = new BasicBSONObject();
        UpdateData.put("_id", "id17244");
        UpdateData.put("a", 17244);
        UpdateData.put("b", "testTrans_17244");
        UpdateData.put("c", 13700017243L);
        UpdateData.put("d", "customer transaction type data application. :17244");

        expDataList = new ArrayList<BSONObject>();
        expDataList.add(data);
    }

    @Test
    public void test() {

        try (Sequoiadb transDB = new Sequoiadb(SdbTestBase.coordUrl, "", "")) {
            transDB.beginTransaction();
            DBCollection transCL = transDB.getCollectionSpace(csName).getCollection(clName);
            BSONObject modifier = new BasicBSONObject("$set", UpdateData);
            transCL.update(new BasicBSONObject("a", data.get("a")), modifier, null);
            // insert the same record as the update
            transCL.insert(UpdateData);
            Assert.fail("insert an existing record with an index,should be failed");
        } catch (BaseException e) {
            Assert.assertEquals(e.getErrorCode(), -38, e.getMessage());
        }

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
