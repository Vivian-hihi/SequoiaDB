package com.sequoiadb.fulltext.largedata;

import org.bson.BSONObject;
import org.bson.types.BasicBSONList;
import org.testng.Assert;
import org.testng.annotations.Test;

import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.DBCursor;
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.fulltext.utils.FullTextDBUtils;
import com.sequoiadb.fulltext.utils.FullTextUtils;
import com.sequoiadb.testcommon.FullTestBase;

/**
 * @Description seqDB-14885: 正在查询固定集合时删除全文索引
 * @author yinzhen
 * @date 2018/11/20
 */
public class Fulltext14885 extends FullTestBase {
    private String csName14885 = "cs14885";
    private String clName = "dropCollection14885";
    private String fullIndexName = "fullIndex14885";
    private String esIndexName;
    private String cappedCLName;

    @Override
    protected void initTestProp() {
        caseProp.setProperty(IGNORESTANDALONE, "true");
        caseProp.setProperty(CSNAME, csName14885);

        caseProp.setProperty(CLNAME, clName);
    }

    @Test
    public void test() throws Exception {
        // 在集合上创建1个全文索引，并插入包含索引字段的数据
        cl.createIndex(fullIndexName,
                "{\"a\":\"text\",\"b\":\"text\",\"c\":\"text\",\"d\":\"text\",\"e\":\"text\",\"f\":\"text\"}", false,
                false);
        FullTextDBUtils.insertData(cl, FullTextUtils.INSERT_NUMS);
        Assert.assertTrue(FullTextUtils.isIndexCreated(cl, fullIndexName, FullTextUtils.INSERT_NUMS));

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
        Assert.assertTrue(FullTextUtils.isIndexDeleted(sdb, esIndexName, cappedCLName));

        // 校验是否有 context 残留
        checkContext();
    }

    @Override
    protected void caseFini() throws Exception {
        Assert.assertTrue(FullTextUtils.isIndexDeleted(sdb, esIndexName, cappedCLName));
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
