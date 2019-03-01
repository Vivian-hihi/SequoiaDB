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
import com.sequoiadb.testcommon.SdbTestBase;
import com.sequoiadb.transaction.TransUtils;

/**
 * @Description Transaction17137.java 创建/删除索引与事务操作并发
 * @author luweikang
 * @date 2019年1月15日
 */
//@Test(groups = "rc")
public class Transaction17137 extends SdbTestBase {

    private String clName = "transCL_17137";
    private Sequoiadb sdb = null;
    private Sequoiadb sdb2 = null;
    private Sequoiadb sdb3 = null;
    private DBCollection cl = null;
    private DBCollection cl2 = null;
    private DBCollection cl3 = null;
    private int recordNum = 10000;
    private DBCursor recordCur = null;
    private List<BSONObject> dataList1 = null;
    private List<BSONObject> dataList2 = null;
    private List<BSONObject> actDataList = null;
    private List<BSONObject> expDataList = null;

    @BeforeClass
    public void setUp() {
        sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "");
        cl = sdb.getCollectionSpace(csName).createCollection(clName);
        
        List<BSONObject> prepareDataList = prepareData( recordNum );
        dataList1 = prepareDataList.subList(0, recordNum/2 );
        dataList2 = prepareDataList.subList(recordNum/2, recordNum );
        cl.insert(dataList1);
        
    }

    @Test
    public void test() {
        sdb2 = new Sequoiadb(SdbTestBase.coordUrl, "", "");
        sdb3 = new Sequoiadb(SdbTestBase.coordUrl, "", "");
        cl2 = sdb2.getCollectionSpace(csName).getCollection(clName);
        cl3 = sdb3.getCollectionSpace(csName).getCollection(clName);

        sdb.beginTransaction();
        sdb3.beginTransaction();
        
        //1 trans1 insert/delete/update return R2s
        cl.insert(dataList2);

        cl.update("{'a':{'$gte':0, '$lt': " + recordNum / 2 + "}}", "{'$set':{ 'a': 1024, 'b': 'test_update_1024'}}", null);
        cl.delete("{'a':{'$gte':" + recordNum / 2 + ", '$lt': " + recordNum + "}}");
        //2 sdb2 create and drop index
        for(int i=0; i<10; i++){
            cl2.createIndex("a", "{a:1, b:-1}", false, false);
            cl2.dropIndex("a");
        }
        cl2.createIndex("a", "{a:1, b:-1}", false, false);

        //3 trans2 query
        expDataList = dataList1;
        recordCur = cl3.query("{'a': {'$isnull': 0}}", null, null, "{'': null}");
        actDataList = TransUtils.getReadActList(recordCur);
        Assert.assertEquals(actDataList, expDataList);
        actDataList.clear();

        recordCur = cl3.query("{'a': {'$isnull': 0}}", null, null, "{'': 'a'}");
        actDataList = TransUtils.getReadActList(recordCur);
        Assert.assertEquals(actDataList, expDataList);
        actDataList.clear();

        //4 commit the trans1
        sdb.commit();
        expDataList.clear();
        expDataList = expData();

        //5 trans2 query
        recordCur = cl3.query("{'a': {'$isnull': 0}}", "{'_id': {'$include': 0}}", null, "{'': null}");
        actDataList = TransUtils.getReadActList(recordCur);
        Assert.assertEquals(actDataList, expDataList);
        actDataList.clear();

        recordCur = cl3.query("{'a': {'$isnull': 0}}", "{'_id': {'$include': 0}}", null, "{'': 'a'}");
        actDataList = TransUtils.getReadActList(recordCur);
        Assert.assertEquals(actDataList, expDataList);
        actDataList.clear();

        //6 commit all trans
        sdb3.commit();
        Assert.assertTrue(cl.isIndexExist("a"));
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
        if( sdb2 != null ){
            sdb2.close();
        }
        if( sdb3 != null ){
            sdb3.close();
        }
    }

    private List<BSONObject> prepareData(int recordNum) {
        List<BSONObject> dataList = new ArrayList<BSONObject>();
        for (int i = 0; i < recordNum; i++) {
            BSONObject data = new BasicBSONObject();
            data.put("a", i);
            data.put("b", "testTrans_17137_" + i);
            data.put("c", 13700000000L);
            data.put("d", "customer transaction type data application.");
            dataList.add(data);
        }
        return dataList;
    }

    private List<BSONObject> expData() {
        List<BSONObject> dataList = new ArrayList<BSONObject>();
        BSONObject data = null;
        for (int i = 0; i < recordNum / 2; i++) {
            data = new BasicBSONObject();
            data.put("a", 1024);
            data.put("b", "test_update_1024");
            data.put("c", 13700000000L);
            data.put("d", "customer transaction type data application.");
            dataList.add(data);
        }
        return dataList;
    }

}
