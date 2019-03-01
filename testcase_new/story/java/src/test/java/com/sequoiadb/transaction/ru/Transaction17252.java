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
import com.sequoiadb.testcommon.SdbTestBase;
import com.sequoiadb.transaction.TransUtils;

/**
 * @Description seqDB-17252 : 更新已提交记录与本事务中删除的记录唯一索引重复
 * @author luweikang
 * @date 2019年1月15日
 */
@Test(groups = "ru")
public class Transaction17252 extends SdbTestBase {

    private String clName = "transCL_17252";
    private Sequoiadb sdb = null;
    private DBCollection cl = null;
    private BSONObject data = null;
    private BSONObject data2 = null;
    private BSONObject data3 = null;
    private BSONObject modifier = null;
    private DBCursor recordCur = null;
    private List<BSONObject> expDataList = null;
    private List<BSONObject> actDataList = null;

    @BeforeClass
    public void setUp() {
        sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "");
        cl = sdb.getCollectionSpace(csName).createCollection(clName);
        cl.createIndex("a", "{a:1}", true, false);
        expDataList = new ArrayList<BSONObject>();
        
        data = new BasicBSONObject();
        data.put("a", 1);
        data.put("b", "testTrans_17252");
        data.put("c", 13700000000L);
        data.put("d", "customer transaction type data application.");

        data2 = new BasicBSONObject();
        data2.put("_id", "id17252");
        data2.put("a", 2);
        data2.put("b", 1024);
        data2.put("c", 13700000000L);
        data2.put("d", "customer transaction type data application.");

        expDataList.add(data);
        expDataList.add(data2);
        cl.insert(expDataList);

        data3 = new BasicBSONObject();
        data3.put("_id", "id17252");
        data3.put("a", 2);
        data3.put("b", "1024_update");
        data3.put("c", 13700000000L);
        data3.put("d", "customer transaction type data application.");

        modifier = new BasicBSONObject();
        modifier.put("$set", data3);

    }

    @Test
    public void test1(){

        //delete R2
        sdb.beginTransaction();
        cl.delete(new BasicBSONObject("a", 2));
        
        //update R1 to R3 same as the R2
        cl.update(new BasicBSONObject("a", 1), modifier, null);
        sdb.rollback();
        
        expDataList.clear();
        expDataList.add(data);
        expDataList.add(data2);

        recordCur = cl.query("{'a': {'$isnull': 0}}", null, null, "{'': null}");
        actDataList = TransUtils.getReadActList(recordCur);
        Assert.assertEquals(actDataList, expDataList);
        actDataList.clear();

        recordCur = cl.query("{'a': {'$isnull': 0}}", null, null, "{'': 'a'}");
        actDataList = TransUtils.getReadActList(recordCur);
        Assert.assertEquals(actDataList, expDataList);
        actDataList.clear();

    }

    // TODO:SEQUOIADBMAINSTREAM-4182
    @Test(enabled = false)
    public void test2(){

        //delete R2
        sdb.beginTransaction();
        cl.delete(new BasicBSONObject("a", 2));
        
        //update R1 to R3 same as the R2
        cl.update(new BasicBSONObject("a", 1), modifier, null);
        sdb.commit();
        
        expDataList.clear();
        expDataList.add(data3);

        recordCur = cl.query("{'a': {'$isnull': 0}}", null, null, "{'': null}");
        actDataList = TransUtils.getReadActList(recordCur);
        Assert.assertEquals(actDataList, expDataList);
        actDataList.clear();

        recordCur = cl.query("{'a': {'$isnull': 0}}", null, null, "{'': 'a'}");
        actDataList = TransUtils.getReadActList(recordCur);
        Assert.assertEquals(actDataList, expDataList);
        actDataList.clear();

        cl.delete("{'a': {'$isnull' :0}}");
        Assert.assertEquals(cl.getCount(), 0);

    }

    @AfterClass
    public void tearDown() {
        sdb.getCollectionSpace(csName).dropCollection(clName);
        if(recordCur != null){
            recordCur.close();
        }
        if( sdb != null ){
            sdb.close();
        }
    }

}
