package com.sequoiadb.fulltext.parallel;

import java.util.List;

import org.bson.BSONObject;
import org.testng.Assert;
import org.testng.SkipException;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import com.sequoiadb.base.CollectionSpace;
import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.fulltext.utils.FullTextDBUtils;
import com.sequoiadb.fulltext.utils.FullTextUtils;
import com.sequoiadb.testcommon.CommLib;
import com.sequoiadb.testcommon.SdbTestBase;
import com.sequoiadb.threadexecutor.ThreadExecutor;
import com.sequoiadb.threadexecutor.annotation.ExecuteOrder;

/**
 * @FileName seqDB-12120:并发更新同一条记录
 * @Author yinzhen
 * @Date 2019-4-28
 */
public class FullText12120 extends SdbTestBase {
    private String clName = "cl12120";
    private Sequoiadb sdb;
    private DBCollection cl;
    private String fullIdxName = "idx12120";
    private String esIndexName;
    private String cappedCLName;
    private int insertNum = 20000;

    @BeforeClass
    public void setUp() throws Exception {
        sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "");
        if (CommLib.isStandAlone(sdb)) {
            throw new SkipException("STANDALONE MODE");
        }

        cl = sdb.getCollectionSpace(csName).createCollection(clName);

        // 创建全文索引
        cl.createIndex(fullIdxName, "{'a':'text','b':'text','c':'text', 'd':'text', 'e':'text', 'f':'text'}", false,
                false);
        esIndexName = FullTextDBUtils.getESIndexName(cl, fullIdxName);
        cappedCLName = FullTextDBUtils.getCappedName(cl, fullIdxName);

        FullTextDBUtils.insertData(cl, insertNum);
        cl.insert("{a:'idx12120', b:'b12120'}");
        Assert.assertTrue(FullTextUtils.isIndexCreated(cl, fullIdxName, 20001));
    }

    @Test
    public void test() throws Exception {
        ThreadExecutor thExecutor = new ThreadExecutor(600000);
        for (int i = 0; i < 10; i++) {
            thExecutor.addWorker(new UpdateData());
        }
        thExecutor.run();

        // 固定集合中新增一条操作类型未更新的记录，es中数据与原集合数据一致
        Assert.assertTrue(FullTextUtils.isIndexCreated(cl, fullIdxName, 20001));
        DBCollection cappedCL = FullTextDBUtils.getCappedCLs(cl, fullIdxName).get(0);
        List<BSONObject> records = FullTextDBUtils.getRecordsFromCL(cappedCL.query());
        Assert.assertEquals(records.get(0).get("Type"), 3);
    }

    @AfterClass
    public void tearDown() throws Exception {
        try {
            CollectionSpace cs = sdb.getCollectionSpace(csName);
            FullTextDBUtils.dropCollection(cs, clName);
            Assert.assertTrue(FullTextUtils.isIndexDeleted(sdb, esIndexName, cappedCLName));
        } finally {
            if (sdb != null) {
                sdb.close();
            }
        }
    }

    private class UpdateData {
        @ExecuteOrder(step = 1, desc = "多线程并发更新同一条包含全文索引字段的记录")
        private void updateData() {
            Sequoiadb db = null;
            try {
                db = new Sequoiadb(coordUrl, "", "");
                DBCollection cl = db.getCollectionSpace(csName).getCollection(clName);
                cl.update("{a:'idx12120', b:'b12120'}", "{$set:{a:'a12120'}}", "{'':'" + fullIdxName + "'}");
            } finally {
                if (db != null) {
                    db.close();
                }
            }
        }
    }
}