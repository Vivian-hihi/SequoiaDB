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
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.testcommon.SdbTestBase;
import com.sequoiadb.testcommon.SdbThreadBase;
import com.sequoiadb.transaction.TransUtils;

/**
 * @Description seqDB-17358.java
 *              插入与更新并发，更新的记录同时匹配已提交记录及其他事务插入的记录，更新走索引，事务提交，过程中读
 * @author luweikang
 * @date 2019年1月15日
 */
@Test(groups = "rc")
public class Transaction17358 extends SdbTestBase {

    private String clName = "transCL_17358";
    private Sequoiadb sdb = null;
    private Sequoiadb sdb1 = null;
    private Sequoiadb sdb2 = null;
    private Sequoiadb sdb3 = null;
    private DBCollection cl = null;
    // TODO:集合名变量名定义不合规范，其他用例类似，均需修改
    private DBCollection CLTrans1 = null;
    private DBCollection CLTrans2 = null;
    private DBCollection CLTrans3 = null;
    private BSONObject data = null;
    private BSONObject data2 = null;
    private BSONObject data3 = null;
    private BSONObject data4 = null;
    private DBCursor recordCur = null;
    private List<BSONObject> expDataList = null;
    private List<BSONObject> actDataList = null;

    @BeforeClass
    public void setUp() {
        sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "");
        // TODO:为什么要在这里new？new完也不用，建议在定义的地方new，data、data2、data3等也类似，其他测试用例也需修改
        // 这里我有个疑问，为什么要在这里赋值，又不在这里使用；为了测试用例的清晰刻度，我觉得：这个变量是个全局变量，应该在定义的时候new，在使用的时候赋值
        // 另外：这里定义数据的时候，是不是使用字符串强转为BSONObject，可读性更强
        sdb1 = new Sequoiadb(SdbTestBase.coordUrl, "", "");
        sdb2 = new Sequoiadb(SdbTestBase.coordUrl, "", "");
        sdb3 = new Sequoiadb(SdbTestBase.coordUrl, "", "");
        cl = sdb.getCollectionSpace(csName).createCollection(clName);
        cl.createIndex("a", "{a:1}", false, false);
        expDataList = new ArrayList<BSONObject>();
        data = new BasicBSONObject();
        data.put("_id", "insertID17358_1");
        data.put("a", 1);
        data.put("b", 1);
        data.put("c", 13700000000L);
        data.put("d", "customer transaction type data application.");
        // TODO：插入记录，放到test中更合理，这个是个操作步骤，不算是一个预置条件
        cl.insert(data);

        data2 = new BasicBSONObject();
        data2.put("_id", "insertID17358_2");
        data2.put("a", 2);
        data2.put("b", 2);
        data2.put("c", 13700000000L);
        data2.put("d", "customer transaction type data application.");

        data3 = new BasicBSONObject();
        data3.put("_id", "insertID17358_1");
        data3.put("a", 3);
        data3.put("b", 3);
        data3.put("c", 13700000000L);
        data3.put("d", "customer transaction type data application.");

        data4 = new BasicBSONObject();
        data4.put("_id", "insertID17358_2");
        data4.put("a", 4);
        data4.put("b", 4);
        data4.put("c", 13700000000L);
        data4.put("d", "customer transaction type data application.");

        // TODO:在这里开启事务，对应的应该在teardown里面关闭事务，建议这个步骤也放到test里面
        sdb1.beginTransaction();
        sdb2.beginTransaction();
        sdb3.beginTransaction();
        CLTrans1 = sdb1.getCollectionSpace(csName).getCollection(clName);
        CLTrans2 = sdb2.getCollectionSpace(csName).getCollection(clName);
        CLTrans3 = sdb3.getCollectionSpace(csName).getCollection(clName);
    }

    @Test
    public void test() {

        // step2: trans1 insert record R2, R2 > R1
        CLTrans1.insert(data2);

        // step3: trans2 update record R1 and R2 to R3 and R4
        UpdateThread updateThread = new UpdateThread();
        updateThread.start();
        Assert.assertTrue(updateThread.matchBlockingMethod(CLTrans2.getClass().getName(), "update"));

        expDataList.add(data);
        expDataList.add(data2);
        // step4: trans1 read TODO:注释和代码行之间，统一一下，都空格吧
        recordCur = CLTrans1.query("{'a': {'$isnull': 0}}", null, "{a:1}", "{'': null}");
        actDataList = TransUtils.getReadActList(recordCur);
        Assert.assertEquals(actDataList, expDataList);
        actDataList.clear();

        recordCur = CLTrans1.query("{'a': {'$isnull': 0}}", null, "{a:1}", "{'': 'a'}");
        actDataList = TransUtils.getReadActList(recordCur);
        Assert.assertEquals(actDataList, expDataList);
        actDataList.clear();

        // TODO:这个变量的赋值，作为事务读的预期结果，应该放到注释之下，其他地方类似，变量在哪里使用，再哪里赋值
        expDataList.clear();
        expDataList.add(data);
        // step5: trans3 read
        recordCur = CLTrans3.query("{'a': {'$isnull': 0}}", null, "{a:1}", "{'': null}");
        actDataList = TransUtils.getReadActList(recordCur);
        Assert.assertEquals(actDataList, expDataList);
        actDataList.clear();

        recordCur = CLTrans3.query("{'a': {'$isnull': 0}}", null, "{a:1}", "{'': 'a'}");
        actDataList = TransUtils.getReadActList(recordCur);
        Assert.assertEquals(actDataList, expDataList);
        actDataList.clear();

        expDataList.clear();
        expDataList.add(data2);
        expDataList.add(data3);
        // step6: no trans read
        recordCur = cl.query("{'a': {'$isnull': 0}}", null, "{a:1}", "{'': null}");
        actDataList = TransUtils.getReadActList(recordCur);
        Assert.assertEquals(actDataList, expDataList);
        actDataList.clear();

        recordCur = cl.query("{'a': {'$isnull': 0}}", null, "{a:1}", "{'': 'a'}");
        actDataList = TransUtils.getReadActList(recordCur);
        Assert.assertEquals(actDataList, expDataList);
        actDataList.clear();

        // step7: read after trans1 commit
        // TODO:事务2的更新操作，返回成功了，就是不阻塞了，不需要重复判断
        sdb1.commit();
        Assert.assertTrue(updateThread.isSuccess(), updateThread.getErrorMsg());
        Assert.assertFalse(updateThread.matchBlockingMethod(CLTrans2.getClass().getName(), "update"));

        expDataList.clear();
        expDataList.add(data3);
        expDataList.add(data4);
        recordCur = cl.query("{'a': {'$isnull': 0}}", null, "{a:1}", "{'': null}");
        actDataList = TransUtils.getReadActList(recordCur);
        Assert.assertEquals(actDataList, expDataList);
        actDataList.clear();

        recordCur = cl.query("{'a': {'$isnull': 0}}", null, "{a:1}", "{'': 'a'}");
        actDataList = TransUtils.getReadActList(recordCur);
        Assert.assertEquals(actDataList, expDataList);
        actDataList.clear();

        // step8: trans2 read
        recordCur = CLTrans2.query("{'a': {'$isnull': 0}}", null, "{a:1}", "{'': null}");
        actDataList = TransUtils.getReadActList(recordCur);
        Assert.assertEquals(actDataList, expDataList);
        actDataList.clear();

        recordCur = CLTrans2.query("{'a': {'$isnull': 0}}", null, "{a:1}", "{'': 'a'}");
        actDataList = TransUtils.getReadActList(recordCur);
        Assert.assertEquals(actDataList, expDataList);
        actDataList.clear();

        expDataList.clear();
        expDataList.add(data);
        expDataList.add(data2);
        // step9: trans3 read
        recordCur = CLTrans3.query("{'a': {'$isnull': 0}}", null, "{a:1}", "{'': null}");
        actDataList = TransUtils.getReadActList(recordCur);
        Assert.assertEquals(actDataList, expDataList);
        actDataList.clear();

        recordCur = CLTrans3.query("{'a': {'$isnull': 0}}", null, "{a:1}", "{'': 'a'}");
        actDataList = TransUtils.getReadActList(recordCur);
        Assert.assertEquals(actDataList, expDataList);
        actDataList.clear();

        expDataList.clear();
        expDataList.add(data3);
        expDataList.add(data4);
        // step10: read after trans2 commit
        sdb2.commit();
        recordCur = cl.query("{'a': {'$isnull': 0}}", null, "{a:1}", "{'': null}");
        actDataList = TransUtils.getReadActList(recordCur);
        Assert.assertEquals(actDataList, expDataList);
        actDataList.clear();

        recordCur = cl.query("{'a': {'$isnull': 0}}", null, "{a:1}", "{'': 'a'}");
        actDataList = TransUtils.getReadActList(recordCur);
        Assert.assertEquals(actDataList, expDataList);
        actDataList.clear();

        // step11: trans3 read
        recordCur = CLTrans3.query("{'a': {'$isnull': 0}}", null, "{a:1}", "{'': null}");
        actDataList = TransUtils.getReadActList(recordCur);
        Assert.assertEquals(actDataList, expDataList);
        actDataList.clear();

        recordCur = CLTrans3.query("{'a': {'$isnull': 0}}", null, "{a:1}", "{'': 'a'}");
        actDataList = TransUtils.getReadActList(recordCur);
        Assert.assertEquals(actDataList, expDataList);
        actDataList.clear();

        sdb3.commit();

    }

    @AfterClass
    public void tearDown() {

        // TODO:这里没有必要使用try
        try {
            sdb.getCollectionSpace(csName).dropCollection(clName);
        } finally {
            if (recordCur != null) {
                recordCur.close();
            }
            if (sdb != null) {
                sdb.close();
            }
            if (sdb1 != null) {
                sdb1.close();
            }
            if (sdb2 != null) {
                sdb2.close();
            }
            if (sdb3 != null) {
                sdb2.close();
            }
        }
    }

    private class UpdateThread extends SdbThreadBase {

        @Override
        public void exec() throws BaseException {
            // TODO:建议风格统一，跟query接口一样，都使用字符串，增加测试代码可读性
            BSONObject modifier = new BasicBSONObject();
            modifier.put("a", 2);
            modifier.put("b", 2);
            CLTrans2.update(null, new BasicBSONObject("$inc", modifier), new BasicBSONObject("", "a"));
        }
    }

}
