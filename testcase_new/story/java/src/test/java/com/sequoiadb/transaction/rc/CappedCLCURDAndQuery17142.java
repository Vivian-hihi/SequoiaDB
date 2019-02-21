package com.sequoiadb.transaction.rc;

import java.util.ArrayList;
import java.util.List;

import org.bson.BSONObject;
import org.bson.util.JSON;
import org.testng.Assert;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import com.sequoiadb.base.CollectionSpace;
import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.DBCursor;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.cappedcl.CappedCLUtils;
import com.sequoiadb.testcommon.SdbTestBase;
import com.sequoiadb.transaction.TransUtils;

/**
 * @testcase seqDB-17142:固定集合中增删改记录，与读并发
 * @date 2019-1-24
 * @author yinzhen
 */
@Test(groups = "rc")
public class CappedCLCURDAndQuery17142 extends SdbTestBase {
     private Sequoiadb sdb = null;
     private String csName = "cappedCS17142";
     private String clName = "cappedCL17142";
     private DBCollection cappedCL = null;
     private List<BSONObject> expList = new ArrayList<BSONObject>();
     private List<BSONObject> actList = new ArrayList<BSONObject>();
     private Sequoiadb db1 = null;
     private Sequoiadb db2 = null;
     private DBCollection cl1 = null;
     private DBCollection cl2 = null;

     @BeforeClass
     public void setUp() {
          sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "");
          cappedCL = CappedCLUtils.createCL(sdb, csName, clName, true);
          BSONObject record = (BSONObject) JSON.parse("{a:1, b:1}");
          cappedCL.insert(record);
          expList.add(record);
          record = (BSONObject) JSON.parse("{a:2, b:2}");
          cappedCL.insert(record);
          expList.add(record);
     }

     @AfterClass
     public void tearDown() {
          if (!db1.isClosed()) {
               db1.close();
          }
          if (!db2.isClosed()) {
               db2.close();
          }
          CollectionSpace cs = sdb.getCollectionSpace(csName);
          if (cs.isCollectionExist(clName)) {
               cs.dropCollection(clName);
          }
          if (!sdb.isClosed()) {
               sdb.dropCollectionSpace(csName);
               sdb.close();
          }
     }

     @Test
     public void test() {
          // 开启并发事务
          db1 = new Sequoiadb(SdbTestBase.coordUrl, "", "");
          db2 = new Sequoiadb(SdbTestBase.coordUrl, "", "");
          cl1 = db1.getCollectionSpace(csName).getCollection(clName);
          cl2 = db2.getCollectionSpace(csName).getCollection(clName);
          db1.beginTransaction();
          db2.beginTransaction();

          // 事务1插入记录，并读记录走表扫描
          BSONObject record = (BSONObject) JSON.parse("{a:3, b:3}");
          cl1.insert(record);
          expList.add(record);
          //TODO:多条记录查询，最好是使用排序，避免用例随机失败
          DBCursor recordsCursor = cl1.query(null, null, null, "{'':null}");
          actList = TransUtils.getReadActList(recordsCursor);
          actList = getNoOidRecords(actList);
          expList = getNoOidRecords(expList);
          Assert.assertEquals(actList, expList);

          // 事务2读记录走表扫描
          recordsCursor = cl2.query(null, null, null, "{'':null}");
          actList = TransUtils.getReadActList(recordsCursor);
          //TODO:记录是固定的，比较的时候不需要剔除id字段
          actList = getNoOidRecords(actList);
          expList = getNoOidRecords(expList);
          Assert.assertEquals(actList, expList);

          // 事务1执行pop操作
          recordsCursor = cl1.query(null, null, "{a:1}", "{'':null}");
          actList = TransUtils.getReadActList(recordsCursor);
          //TODO:封装成本地方法，此处调用
          for (int i = 0; i < actList.size(); i++) {
               BSONObject object = actList.get(i);
               int a = (int) object.get("a");
               if (a == 1) {
                    long oid = (long) object.get("_id");
                    BSONObject poObject = (BSONObject) JSON.parse("{LogicalID:" + oid + ", Direction:1}");
                    cl1.pop(poObject);
               }
               if (a == 3) {
                    long oid = (long) object.get("_id");
                    BSONObject poObject = (BSONObject) JSON.parse("{LogicalID:" + oid + ", Direction:-1}");
                    cl1.pop(poObject);
               }
          }
          record = (BSONObject) JSON.parse("{_id:2, a:2, b:2}");
          expList.clear();
          expList.add(record);

          // 事务1读记录走表扫描
          recordsCursor = cl1.query(null, null, "{a:1}", "{'':null}");
          actList = TransUtils.getReadActList(recordsCursor);
          actList = getNoOidRecords(actList);
          expList = getNoOidRecords(expList);
          Assert.assertEquals(actList, expList);

          // 事务2读记录走表扫描
          recordsCursor = cl2.query(null, null, "{a:1}", "{'':null}");
          actList = TransUtils.getReadActList(recordsCursor);
          actList = getNoOidRecords(actList);
          Assert.assertEquals(actList, expList);

          // 回滚事务1，并读记录走表扫描
          db1.rollback();
          recordsCursor = cl1.query(null, null, "{a:1}", "{'':null}");
          actList = TransUtils.getReadActList(recordsCursor);
          actList = getNoOidRecords(actList);
          Assert.assertEquals(actList, expList);

          // 事务2读记录走表扫描
          recordsCursor = cl2.query(null, null, "{a:1}", "{'':null}");
          actList = TransUtils.getReadActList(recordsCursor);
          actList = getNoOidRecords(actList);
          Assert.assertEquals(actList, expList);
          
          // 事务2回滚
          db2.rollback();
          recordsCursor.close();
     }

     private List<BSONObject> getNoOidRecords(List<BSONObject> records) {
          List<BSONObject> expRecords = new ArrayList<>();
          for (int i = 0; i < records.size(); i++) {
               BSONObject record = records.get(i);
               record.removeField("_id");
               expRecords.add(record);
          }
          return expRecords;
     }
}
