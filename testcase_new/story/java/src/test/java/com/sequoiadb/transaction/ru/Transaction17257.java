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
 * @Description Transaction17135.java 回滚的记录与本事务中的记录重复
 * @author luweikang
 * @date 2019年1月15日
 */
@Test(groups = "ru")
public class Transaction17257 extends SdbTestBase {

    private String clName = "transCL_17135";
    private Sequoiadb sdb = null;
    private DBCollection cl = null;
    private BSONObject data = null;
    private BSONObject data2 = null;
    private DBCursor recordCur = null;
    private List<BSONObject> expDataList = null;
    private List<BSONObject> actDataList = null;

    @BeforeClass
    public void setUp() {
        sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "");
        cl = sdb.getCollectionSpace(csName).createCollection(clName);
        expDataList = new ArrayList<BSONObject>();
        data = new BasicBSONObject();
        data.put("a", 1);
        data.put("b", 1);
        data.put("c", 13700000000L);
        data.put("d", "customer transaction type data application.");
        expDataList.add(data);

        data2 = new BasicBSONObject();
        data2.put("a", 1);
        data2.put("b", 2);
        data2.put("c", 13700000000L);
        data2.put("d", "customer transaction type data application.");
        expDataList.add(data2);
        cl.insert(expDataList);

    }

    // TODO:SEQUOIADBMAINSTREAM-4118
    @Test(enabled = false)
    public void test() {
        try (Sequoiadb transDB1 = new Sequoiadb(SdbTestBase.coordUrl, "", "");) {
            transDB1.beginTransaction();
            DBCollection transCL1 = transDB1.getCollectionSpace(csName).getCollection(clName);
            transCL1.delete("{a:1}");
            try {
                transCL1.createIndex("a", "{a:1}", true, false);
                Assert.fail("create unique index should be failed");
            } catch (BaseException e) {
                Assert.assertEquals(e.getErrorCode(), -38, e.getMessage());
            }
            transDB1.rollback();
        }

        recordCur = cl.query("{'a': {'$isnull': 0}}", null, null, "{'': null}");
        actDataList = TransUtils.getReadActList(recordCur);
        Assert.assertEquals(actDataList, expDataList, "check data");
        actDataList.clear();

        recordCur = cl.query("{'a': {'$isnull': 0}}", null, null, "{'': 'a'}");
        actDataList = TransUtils.getReadActList(recordCur);
        Assert.assertEquals(actDataList, expDataList);
        actDataList.clear();

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
