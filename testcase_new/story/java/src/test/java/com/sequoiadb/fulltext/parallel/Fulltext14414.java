package com.sequoiadb.fulltext.parallel;

import java.util.concurrent.atomic.AtomicInteger;

import org.testng.Assert;
import org.testng.annotations.Test;

import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.fulltext.utils.FullTextDBUtils;
import com.sequoiadb.fulltext.utils.FullTextUtils;
import com.sequoiadb.testcommon.FullTestBase;
import com.sequoiadb.threadexecutor.ThreadExecutor;
import com.sequoiadb.threadexecutor.annotation.ExecuteOrder;

/**
 * @FileName seqDB-14414:同一集合并发创建删除相同的全文索引
 * @Author yinzhen
 * @Date 2019-4-28
 */
public class Fulltext14414 extends FullTestBase {
    private String clName = "cl14414";
    private String fullIdxName = "idx14414";
    private String cappedCLName;
    private String esIndexName;
    private AtomicInteger atoint = new AtomicInteger(0);
    private int insertNum = 20000;

    @Override
    protected void initTestProp() {
        caseProp.setProperty(IGNORESTANDALONE, "true");
        caseProp.setProperty(CLNAME, clName);
    }

    @Override
    protected void caseInit() throws Exception {
        FullTextDBUtils.insertData(cl, insertNum);
    }

    @Test
    public void test() throws Exception {
        ThreadExecutor thExecutor = new ThreadExecutor(600000);
        for (int i = 0; i < 10; i++) {
            thExecutor.addWorker(new CreateAndDropIdx());
        }
        thExecutor.run();
        Assert.assertEquals(atoint.get(), 1);

        // 主备节点上索引信息一致，固定集合、索引信息、ES端数据一致
        Assert.assertTrue(FullTextUtils.isIndexDeleted(sdb, esIndexName, cappedCLName));
        Assert.assertTrue(FullTextUtils.isCLDataConsistency(cl));
    }

    @Override
    protected void caseFini() throws Exception {
        Assert.assertTrue(FullTextUtils.isIndexDeleted(sdb, esIndexName, cappedCLName));
    }

    private class CreateAndDropIdx {
        @ExecuteOrder(step = 1, desc = "多线程创建删除同一个全文索引")
        private void createFullIdx() {
            Sequoiadb db = null;
            try {
                db = new Sequoiadb(coordUrl, "", "");
                DBCollection cl = db.getCollectionSpace(csName).getCollection(clName);
                cl.createIndex(fullIdxName, "{'a':'text','b':'text','c':'text', 'd':'text', 'e':'text', 'f':'text'}",
                        false, false);

                cappedCLName = FullTextDBUtils.getCappedName(cl, fullIdxName);
                esIndexName = FullTextDBUtils.getESIndexName(cl, fullIdxName);
                cl.dropIndex(fullIdxName);
                atoint.incrementAndGet();
            } catch (BaseException e) {
                Assert.assertEquals(e.getErrorCode(), -42);
            } finally {
                if (db != null) {
                    db.close();
                }
            }
        }
    }
}