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
 * @Description Transaction17256.java 回滚的记录与已提交记录唯一索引重复
 * @author luweikang
 * @date 2019年1月15日
 */
@Test(groups = "ru")
public class Transaction17256 extends SdbTestBase {

    private String clName = "transCL_17256";
    private Sequoiadb sdb = null;
    private DBCollection cl = null;
    private BSONObject data = null;
    private BSONObject data2 = null;
    private BSONObject modifier = null;
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
        cl.insert(data);

        data2 = new BasicBSONObject();
        data2.put("a", 1);
        data2.put("b", 1);
        data2.put("c", 13700000000L);
        data2.put("d", "customer transaction type data application.");
        cl.insert(data2);

        modifier = new BasicBSONObject();
        BSONObject data3 = new BasicBSONObject();
        data3.put("a", 3);
        data3.put("b", 3);
        data3.put("c", 13700000000L);
        data3.put("d", "customer transaction type data application.");
        modifier.put("$set", data3);
    }
    
    //TODO:SEQUOIADBMAINSTREAM-4118
    @Test(enabled=false)
    public void test(){
        //1 update record R1 to R2 not the same
        sdb.beginTransaction();
        cl.update(new BasicBSONObject("b", 1), modifier, null);
        
        try(Sequoiadb db = new Sequoiadb(SdbTestBase.coordUrl, "", "")) {
            //2 no trans create unique index
            DBCollection cl2 = db.getCollectionSpace(csName).getCollection(clName); 
            cl2.createIndex("a", "{a: 1}", true, false);
            Assert.fail("create unique index should be failed");
        } catch (BaseException e) {
            Assert.assertEquals(e.getErrorCode(), -38, e.getMessage());
        }

        try {
            //2 trans create unique index
            cl.createIndex("b", "{b: 1}", true, false);
            Assert.fail("create unique index should be failed");
        } catch (BaseException e) {
            Assert.assertEquals(e.getErrorCode(), -38, e.getMessage());
        }
        
        expDataList.add(data);
        expDataList.add(data2);
        recordCur = cl.query("{'a': {'$isnull': 0}}", null, "{'a': 1}", "{'': null}");
        actDataList = TransUtils.getReadActList(recordCur);
        Assert.assertEquals(actDataList, expDataList, "check data");
        actDataList.clear();

        recordCur = cl.query("{'a': {'$isnull': 0}}", null, "{'a': 1}", "{'': 'a'}");
        actDataList = TransUtils.getReadActList(recordCur);
        Assert.assertEquals(actDataList, expDataList);
        actDataList.clear();

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
