package com.sequoiadb.fulltext.largedata;

import java.util.List;

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
import com.sequoiadb.fulltext.utils.FullTextDBUtils;
import com.sequoiadb.fulltext.utils.FullTextUtils;
import com.sequoiadb.testcommon.CommLib;
import com.sequoiadb.testcommon.SdbTestBase;

/**
 * @Description seqDB-12066: 集合上存在全文索引，删除集合
 * @author yinzhen
 * @date 2018/11/20
 */
public class Fulltext12066 extends SdbTestBase {
    private Sequoiadb sdb;
    private CollectionSpace cs;
    private DBCollection cl;
    private String clName = "cl12066";
    private String fullIndexName = "fullIndex12066";
    private String cappedName;
    private String esIndexName;

    @BeforeClass
    public void setUp() {
        sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "");
        if (CommLib.isStandAlone(sdb)) {
            throw new SkipException("StandAlone environment!");
        }
        cs = sdb.getCollectionSpace(csName);
        cl = cs.createCollection(clName);
    }

    @Test
    public void test() throws Exception {
        // 在集合上创建1个全文索引，并插入大量包含索引字段的数据
        cl.createIndex(fullIndexName,
                "{\"a\":\"text\",\"b\":\"text\",\"c\":\"text\",\"d\":\"text\",\"e\":\"text\",\"f\":\"text\"}", false,
                false);
        FullTextDBUtils.insertData(cl, FullTextUtils.INSERT_NUMS);
        Assert.assertTrue(FullTextUtils.isIndexCreated(cl, fullIndexName, FullTextUtils.INSERT_NUMS));

        // 直连主数据节点使用游标的方式获取固定集合中的一条记录
        List<DBCollection> cappedCLs = FullTextDBUtils.getCappedCLs(cl, fullIndexName);
        DBCollection cappedCL = cappedCLs.get(0);
        DBCursor cursor = cappedCL.query();
        cursor.getNext();

        // 多次执行删除集合的操作
        if (cappedCL.getCount() > 2) {
            for (int i = 0; i < 3; i++) {
                try {
                    cs.dropCollection(clName);
                    Assert.fail("drop collection need to return -147!");
                } catch (BaseException e) {
                    Assert.assertEquals(e.getErrorCode(), -147, e.getMessage());
                }
            }
        }

        // 关闭打开的游标
        if (cursor != null) {
            cursor.close();
        }

        // 关闭步骤2中的游标，再次删除集合
        cappedName = FullTextDBUtils.getCappedName(cl, fullIndexName);
        esIndexName = FullTextDBUtils.getESIndexName(cl, fullIndexName);
        FullTextDBUtils.dropCollection(cs, clName);
        Assert.assertTrue(FullTextUtils.isIndexDeleted(sdb, esIndexName, cappedName));
    }

    @AfterClass
    public void tearDown() throws Exception {
        try {
            FullTextDBUtils.dropCollection(cs, clName);
            Assert.assertTrue(FullTextUtils.isIndexDeleted(sdb, esIndexName, cappedName));
        } finally {
            if (sdb != null) {
                sdb.close();
            }
        }
    }
}
