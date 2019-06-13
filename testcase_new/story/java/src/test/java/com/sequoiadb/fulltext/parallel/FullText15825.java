package com.sequoiadb.fulltext.parallel;

import java.util.concurrent.atomic.AtomicInteger;

import org.bson.BSONObject;
import org.bson.util.JSON;
import org.testng.Assert;
import org.testng.SkipException;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import com.sequoiadb.base.CollectionSpace;
import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.fulltext.utils.FullTextDBUtils;
import com.sequoiadb.fulltext.utils.FullTextUtils;
import com.sequoiadb.testcommon.CommLib;
import com.sequoiadb.testcommon.SdbTestBase;
import com.sequoiadb.threadexecutor.ThreadExecutor;
import com.sequoiadb.threadexecutor.annotation.ExecuteOrder;

/**
 * @FileName seqDB-15825:同一集合并发删除相同的全文索引
 * @Author yinzhen
 * @Date 2019-4-30
 */
public class FullText15825 extends SdbTestBase {
    private String clName = "cl15825";
    private Sequoiadb sdb;
    private DBCollection cl;
    private String fullIdxName = "idx15825";
    private String groupName;
    private String cappedCLName;
    private String esIndexName;
    private AtomicInteger atoint = new AtomicInteger(0);
    private int insertNum = 20000;

    @BeforeClass
    public void setUp() throws Exception {
        sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "");
        if (CommLib.isStandAlone(sdb)) {
            throw new SkipException("STANDALONE MODE");
        }

        groupName = CommLib.getDataGroupNames(sdb).get(0);
        cl = sdb.getCollectionSpace(csName).createCollection(clName,
                (BSONObject) JSON.parse("{Group:'" + groupName + "'}"));
        FullTextDBUtils.insertData(cl, insertNum);
        cl.createIndex(fullIdxName, "{'a':'text','b':'text','c':'text', 'd':'text', 'e':'text', 'f':'text'}", false,
                false);
        Assert.assertTrue(FullTextUtils.isIndexCreated(cl, fullIdxName, insertNum));
        esIndexName = FullTextDBUtils.getESIndexName(cl, fullIdxName);
        cappedCLName = FullTextDBUtils.getCappedName(cl, fullIdxName);
    }

    @Test
    public void test() throws Exception {
        ThreadExecutor thExecutor = new ThreadExecutor(600000);
        for (int i = 0; i < 10; i++) {
            thExecutor.addWorker(new DropFullIdx());
        }
        thExecutor.run();
        Assert.assertEquals(atoint.get(), 1);

        // 索引信息被删除，同时固定集合删除成功，主备节点数据一致，ES端最终无索引数据
        Assert.assertTrue(FullTextUtils.isIndexDeleted(sdb, esIndexName, cappedCLName));
        Assert.assertTrue(FullTextUtils.isCLDataConsistency(cl));
        FullTextUtils.isCLDataConsistency(cl);
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

    private class DropFullIdx {
        @ExecuteOrder(step = 1, desc = "多线程创建删除同一个全文索引")
        private void dropFullIdx() {
            Sequoiadb db = null;
            try {
                db = new Sequoiadb(coordUrl, "", "");
                DBCollection cl = db.getCollectionSpace(csName).getCollection(clName);
                cl.dropIndex(fullIdxName);
                atoint.incrementAndGet();
            } catch (BaseException e) {
                if (e.getErrorCode() != -47) {
                    throw e;
                }
            } finally {
                if (db != null) {
                    db.close();
                }
            }
        }
    }
}