package com.sequoiadb.fulltext.parallel;

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
 * @FileName seqDB-15830:创建全文索引与删除集合并发
 * @Author yinzhen
 * @Date 2019-4-30
 */
public class FullText15830 extends SdbTestBase {
    private String clName = "cl15830";
    private Sequoiadb sdb;
    private DBCollection cl;
    private String fullIdxName = "idx15830";
    private String cappedCLName;
    private String esIndexName;
    private int insertNum = 20000;

    @BeforeClass
    public void setUp() throws Exception {
        sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "");
        if (CommLib.isStandAlone(sdb)) {
            throw new SkipException("STANDALONE MODE");
        }

        // 创建全文索引
        cl = sdb.getCollectionSpace(csName).createCollection(clName);
        FullTextDBUtils.insertData(cl, insertNum);
        cl.createIndex(fullIdxName, "{'a':'text','b':'text','c':'text', 'd':'text', 'e':'text', 'f':'text'}", false,
                false);
        Assert.assertTrue(FullTextUtils.isIndexCreated(cl, fullIdxName, insertNum));

        // 获取固定集合和ES索引名
        esIndexName = FullTextDBUtils.getESIndexName(cl, fullIdxName);
        cappedCLName = FullTextDBUtils.getCappedName(cl, fullIdxName);
        cl.dropIndex(fullIdxName);
        Assert.assertTrue(FullTextUtils.isIndexDeleted(sdb, esIndexName, cappedCLName));
    }

    @Test
    public void test() throws Exception {
        ThreadExecutor thExecutor = new ThreadExecutor(600000);
        thExecutor.addWorker(new CreateFullIdx());
        thExecutor.addWorker(new DropCL());

        thExecutor.run();

        // 原始集合及固定集合均被删除成功，ES上全文索引删除成功，主备节点数据一致，无数据文件残留
        Assert.assertFalse(sdb.getCollectionSpace(csName).isCollectionExist(clName));
        Assert.assertTrue(FullTextUtils.isIndexDeleted(sdb, esIndexName, cappedCLName));
    }

    @AfterClass
    public void tearDown() throws Exception {
        try {
            CollectionSpace cs = sdb.getCollectionSpace(csName);
            if (cs.isCollectionExist(clName)) {
                FullTextDBUtils.dropCollection(cs, clName);
            }
            Assert.assertTrue(FullTextUtils.isIndexDeleted(sdb, esIndexName, cappedCLName));
        } finally {
            if (sdb != null) {
                sdb.close();
            }
        }
    }

    private class CreateFullIdx {
        @ExecuteOrder(step = 1, desc = "创建全文索引")
        private void createFullIdx() {
            Sequoiadb db = null;
            try {
                db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
                DBCollection cl = db.getCollectionSpace(csName).getCollection(clName);
                cl.createIndex(fullIdxName, "{'a':'text','b':'text','c':'text', 'd':'text', 'e':'text', 'f':'text'}",
                        false, false);
            } catch (BaseException e) {
                if (e.getErrorCode() != -23) {
                    throw e;
                }
            } finally {
                if (db != null) {
                    db.close();
                }
            }
        }
    }

    private class DropCL {
        @ExecuteOrder(step = 1, desc = "删除集合")
        private void dropCL() {
            Sequoiadb db = null;
            try {
                db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
                db.getCollectionSpace(csName).dropCollection(clName);
            } finally {
                if (db != null) {
                    db.close();
                }
            }
        }
    }
}