package com.sequoiadb.transaction;

import java.util.ArrayList;
import java.util.List;

import org.bson.BSON;
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
import com.sequoiadb.testcommon.SdbTestBase;

/**
 * @testcase seqDB-17229:并发读写不同的记录
 * @date 2019-1-16
 * @author yinzhen
 *
 */
@Test(groups = "ru")
public class QueryAndWrite17229 extends SdbTestBase {
     private Sequoiadb sdb = null;
     private String clName = "cl17229";
     private DBCollection cl = null;
     private List<BSONObject> expList = new ArrayList<BSONObject>();
     private List<BSONObject> actList = new ArrayList<BSONObject>();
     private Sequoiadb db1 = null;
     private Sequoiadb db2 = null;
     private Sequoiadb db3 = null;
     private Sequoiadb db4 = null;
     private Sequoiadb db5 = null;
     private DBCollection cl1 = null;
     private DBCollection cl2 = null;
     private DBCollection cl3 = null;
     private DBCollection cl4 = null;
     private DBCollection cl5 = null;

     @BeforeClass
     public void setUp() {
          sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "");
          cl = sdb.getCollectionSpace(csName).createCollection(clName);
          cl.createIndex("textIndex17229", "{a:1}", false, false);
          insertData();
     }

     @AfterClass
     public void tearDown() {
          if (!db1.isClosed()) {
               db1.close();
          }
          if (!db2.isClosed()) {
               db2.close();
          }
          if (!db3.isClosed()) {
               db3.close();
          }
          if (!db4.isClosed()) {
               db4.close();
          }
          if (!db5.isClosed()) {
               db5.close();
          }
          CollectionSpace cs = sdb.getCollectionSpace(csName);
          if (cs.isCollectionExist(clName)) {
               cs.dropCollection(clName);
          }
          if (!sdb.isClosed()) {
               sdb.close();
          }
     }

     @Test
     public void test() {
          // 开启并发事务
          db1 = new Sequoiadb(SdbTestBase.coordUrl, "", "");
          db2 = new Sequoiadb(SdbTestBase.coordUrl, "", "");
          db3 = new Sequoiadb(SdbTestBase.coordUrl, "", "");
          db4 = new Sequoiadb(SdbTestBase.coordUrl, "", "");
          db5 = new Sequoiadb(SdbTestBase.coordUrl, "", "");
          cl1 = db1.getCollectionSpace(csName).getCollection(clName);
          cl2 = db2.getCollectionSpace(csName).getCollection(clName);
          cl3 = db3.getCollectionSpace(csName).getCollection(clName);
          cl4 = db4.getCollectionSpace(csName).getCollection(clName);
          cl5 = db5.getCollectionSpace(csName).getCollection(clName);
          db1.beginTransaction();
          db2.beginTransaction();
          db3.beginTransaction();
          db4.beginTransaction();
          db5.beginTransaction();

          // 事务1插入记录
          BSONObject record = (BSONObject) JSON.parse("{_id:200, a:200, b:200}");
          cl1.insert(record);

          // 事务2更新记录
          cl2.update("{a:1}", "{$set:{a:100}}", "{'':'textIndex17229'}");

          // 事务3删除记录
          cl3.delete("{a:0}", "{'':'textIndex17229'}");

          // 事务4读记录走索引扫描
          DBCursor recordsCursor = cl4.query("{a:2}", null, "{_id:1}", "{'':'textIndex17229'}");
          DBCursor explainCursor = cl4.explain((BSONObject) JSON.parse("{a:2}"), null,
                    (BSONObject) JSON.parse("{_id:1}"), (BSONObject) JSON.parse("{'':'textIndex17229'}"), 0, 1, 0,
                    null);
          while (explainCursor.hasNext()) {
               String scanType = (String) explainCursor.getNext().get("ScanType");
               Assert.assertEquals(scanType, "ixscan");
          }
          record = (BSONObject) JSON.parse("{_id:2, a:2, b:2}");
          expList.add(record);
          actList = TransUtils.getReadActList(recordsCursor);
          Assert.assertEquals(actList, expList);

          // 事务5读记录走表扫描
          recordsCursor = cl5.query("{a:3}", null, "{_id:1}", "{'':null}");
          explainCursor = cl5.explain((BSONObject) JSON.parse("{a:3}"), null, (BSONObject) JSON.parse("{_id:1}"),
                    (BSONObject) JSON.parse("{'':null}"), 0, 1, 0, null);
          while (explainCursor.hasNext()) {
               String scanType = (String) explainCursor.getNext().get("ScanType");
               Assert.assertEquals(scanType, "tbscan");
          }
          record = (BSONObject) JSON.parse("{_id:3, a:3, b:3}");
          expList.clear();
          expList.add(record);
          actList = TransUtils.getReadActList(recordsCursor);
          Assert.assertEquals(actList, expList);

          // 提交事务
          db1.commit();
          db2.commit();
          db3.commit();
          db4.commit();
          db5.commit();
          recordsCursor.close();
          explainCursor.close();
     }

     private void insertData() {
          List<BSONObject> records = new ArrayList<>();
          for (int i = 0; i < 100; i++) {
               BSONObject record = (BSONObject) JSON.parse("{_id:" + i + ",a:" + i + ", b:" + i + "}");
               records.add(record);
          }
          cl.insert(records);
     }
}
