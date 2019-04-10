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
import com.sequoiadb.testcommon.SdbTestBase;
import com.sequoiadb.transaction.TransUtils;

/**
 * @Description Transaction18221.java query.update事务回滚功能验证
 * @author luweikang
 * @date 2019年1月15日
 */
@Test(groups = "rc")
public class Transaction18221 extends SdbTestBase {

    private String clName = "transCL_18221";
    private Sequoiadb sdb = null;
    private Sequoiadb sdb2 = null;
    private DBCollection cl = null;
    private DBCollection cl2 = null;
    private BSONObject data = null;
    private BSONObject data2 = null;
    private BSONObject modifier = null;
    private DBCursor recordCur = null;
    private List<BSONObject> expDataList = null;
    private List<BSONObject> actDataList = null;

    @BeforeClass
    public void setUp() {
        sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "");
        cl = sdb.getCollectionSpace(csName).createCollection(clName);
        cl.createIndex("a", "{a:1}", false, false);
        expDataList = new ArrayList<BSONObject>();

        data = new BasicBSONObject();
        data.put("_id", "insertID18221");
        data.put("a", 1);
        data.put("b", 1);
        data.put("c", 13700000000L);
        data.put("d", "customer transaction type data application.");
        cl.insert(data);

        modifier = new BasicBSONObject();
        data2 = new BasicBSONObject();
        data2.put("_id", "updateID18221");
        data2.put("a", 2);
        data2.put("b", "update2");
        data2.put("c", 13700000000L);
        data2.put("d", "customer transaction type data application.");
        modifier.put("$set", data2);
    }

    @Test
    public void test() {
        sdb2 = new Sequoiadb(SdbTestBase.coordUrl, "", "");
        cl2 = sdb2.getCollectionSpace(csName).getCollection(clName);

        sdb2.beginTransaction();

        // query.update R1 to R2
        DBCursor tbCur = cl2.queryAndUpdate(new BasicBSONObject("a", 1), null, null, null, modifier, 0, -1, 0, true);
        try {
            BSONObject actData = tbCur.getNext();
            Assert.assertEquals(actData, data2);
        } finally {
            if (tbCur != null) {
                tbCur.close();
            }
        }

        // commit trans
        sdb2.rollback();

        // no trans query
        expDataList.clear();
        expDataList.add(data);
        recordCur = cl.query(null, null, "{a:1}", "{'': null}");
        actDataList = TransUtils.getReadActList(recordCur);
        Assert.assertEquals(actDataList, expDataList);
        actDataList.clear();

        recordCur = cl.query(null, null, "{a:1}", "{'': 'a'}");
        actDataList = TransUtils.getReadActList(recordCur);
        Assert.assertEquals(actDataList, expDataList);
        actDataList.clear();

    }

    @AfterClass
    public void tearDown() {
        sdb.getCollectionSpace(csName).dropCollection(clName);
        if (recordCur != null) {
            recordCur.close();
        }
        if (sdb != null) {
            sdb.close();
        }
        if (sdb2 != null) {
            sdb2.close();
        }
    }

}
