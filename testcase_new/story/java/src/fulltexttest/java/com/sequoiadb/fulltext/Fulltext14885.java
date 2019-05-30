package com.sequoiadb.fulltext;

import org.bson.BSONObject;
import org.bson.types.BasicBSONList;
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
import com.sequoiadb.utils.FullTextDBUtils;
import com.sequoiadb.utils.FullTextESUtils;
import com.sequoiadb.utils.FullTextUtils;

/**
 * @Description seqDB-14885: 正在查询固定集合时删除全文索引
 * @author yinzhen
 * @date 2018/11/20
 */
public class Fulltext14885 extends SdbTestBase {
    private Sequoiadb sdb;
    private CollectionSpace cs;
    private DBCollection cl;
    private String csName14885 = "cs14885";
    private String clName = "dropCollection14885";
    private String fullIndexName = "fullIndex14885";
    private Client esClient = null;
    private String esIndexName;
    private String cappedCLName;

    @BeforeClass
    public void setUp() {
        sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "");
        if (CommLib.isStandAlone(sdb)) {
            throw new SkipException("StandAlone environment!");
        }
        if (sdb.isCollectionSpaceExist(csName14885)) {
            sdb.dropCollectionSpace(csName14885);
        }

        cs = sdb.createCollectionSpace(csName14885);
        cl = cs.createCollection(clName);
        esClient = FullTextESUtils.createTransportClient(SdbTestBase.esHostName,
                Integer.parseInt(SdbTestBase.esServiceName));
    }

    @Test
    public void test() throws Exception {
        // 在集合上创建1个全文索引，并插入包含索引字段的数据
        cl.createIndex(fullIndexName,
                "{\"a\":\"text\",\"b\":\"text\",\"c\":\"text\",\"d\":\"text\",\"e\":\"text\",\"f\":\"text\"}", false,
                false);
        FullTextDBUtils.insertData(cl, FullTextUtils.INSERT_NUMS);
        Assert.assertTrue(FullTextUtils.isIndexCreated(esClient, cl, fullIndexName, FullTextUtils.INSERT_NUMS));

        // 使用游标的方式获取对应的固定集合中的一条记录
        DBCollection cappedCL = FullTextDBUtils.getCappedCLs(cl, fullIndexName).get(0);
        DBCursor cursor = cappedCL.query();
        cursor.getNext();

        // 多次执行删除全文索引的操作，检查结果
        for (int i = 0; i < 10; i++) {
            try {
                cl.dropIndex(fullIndexName);
                Assert.fail("drop textIndex need to return -147!");
            } catch (BaseException e) {
                if (e.getErrorCode() != -147 && e.getErrorCode() != -190) {
                    throw e;
                }
            }
        }

        // 关闭打开的游标
        if (cursor != null) {
            cursor.close();
        }

        // 关闭步骤2中的游标，再次删除全文索引
        esIndexName = FullTextDBUtils.getESIndexName(cl, fullIndexName);
        cappedCLName = FullTextDBUtils.getCappedName(cl, fullIndexName);
        System.out.println("cappedCSName : " + cappedCLName + " esIndexNames " + esIndexName);

        FullTextDBUtils.dropFullTextIndex(cl, fullIndexName);
        Assert.assertTrue(FullTextUtils.isIndexDeleted(sdb, esClient, esIndexName, cappedCLName));

        // 校验是否有 context 残留
        checkContext();
    }

    @AfterClass
    public void tearDown() throws Exception {
        try {
            FullTextDBUtils.dropCollectionSpace(sdb, csName14885);
            Assert.assertTrue(FullTextUtils.isIndexDeleted(sdb, esClient, esIndexName, cappedCLName));
        } finally {
            if (sdb != null) {
                sdb.close();
            }
            if (esClient != null) {
                esClient.close();
            }
        }
    }

    private void checkContext() throws InterruptedException {
        int count = 0;
        out: while (count++ < 500) {
            Thread.sleep(100);
            DBCursor cursor2 = sdb.getSnapshot(0, "{}", "{}", "{}");
            while (cursor2.hasNext()) {
                BSONObject object = cursor2.getNext();
                BasicBSONList list = (BasicBSONList) object.get("Contexts");
                for (int i = 0; i < list.size(); i++) {
                    BSONObject object2 = (BSONObject) list.get(i);
                    String desc = (String) object2.get("Description");
                    if (desc.indexOf(cappedCLName) != -1) {
                        Assert.assertNotEquals(count, 500);
                        continue out;
                    }
                }
            }
            break;
        }
    }
}
