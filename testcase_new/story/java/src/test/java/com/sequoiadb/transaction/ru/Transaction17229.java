package com.sequoiadb.transaction.ru;

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
import com.sequoiadb.testcommon.SdbTestBase;
import com.sequoiadb.transaction.TransUtils;

/**
 * @testcase seqDB-17229:并发读写不同的记录
 * @date 2019-1-16
 * @author yinzhen
 *
 */
@Test(groups = "ru")
public class Transaction17229 extends SdbTestBase {
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
          this.cl1Insert();

          // 事务2更新记录
          cl2.update("{a:{$lt:'1001'}}", "{$set:{a:1}}", "{'':'textIndex17229'}");

          // 事务3删除记录
          cl3.delete("{$and:[{a:{$gt:1000}},{a:{$lt:2001}}]}", "{'':'textIndex17229'}");

          // 事务4读记录走索引扫描
          DBCursor recordsCursor = cl4.query("{$and:[{a:{$gt:2000}},{a:{$lt:3001}}]}", null, "{_id:1}",
                    "{'':'textIndex17229'}");
          actList = TransUtils.getReadActList(recordsCursor);
          cl4Query();
          Assert.assertEquals(actList, expList);

          // 事务5读记录走表扫描
          recordsCursor = cl5.query("{$and:[{a:{$gt:3000}},{a:{$lt:4001}}]}", null, "{_id:1}", "{'':null}");
          cl5Query();
          actList = TransUtils.getReadActList(recordsCursor);
          Assert.assertEquals(actList, expList);

          // 提交事务
          db1.commit();
          db2.commit();
          db3.commit();
          db4.commit();
          db5.commit();
          recordsCursor.close();
     }

     private void insertData() {
          List<BSONObject> records = new ArrayList<>();
          for (int i = 1; i <= 4000; i++) {
               BSONObject record = (BSONObject) JSON.parse("{_id:" + i + ",a:" + i + ", b:" + i + "}");
               records.add(record);
          }
          cl.insert(records);
     }

     private void cl1Insert() {
          List<BSONObject> records = new ArrayList<>();
          for (int i = 4001; i <= 5000; i++) {
               BSONObject record = (BSONObject) JSON.parse("{_id:" + i + ",a:" + i + ", b:" + i + "}");
               records.add(record);
          }
          cl1.insert(records);
     }

     private void cl4Query() {
          List<BSONObject> records = new ArrayList<>();
          for (int i = 2001; i <= 3000; i++) {
               BSONObject record = (BSONObject) JSON.parse("{_id:" + i + ",a:" + i + ", b:" + i + "}");
               records.add(record);
          }
          expList.clear();
          expList.addAll(records);
     }

     private void cl5Query() {
          List<BSONObject> records = new ArrayList<>();
          for (int i = 3001; i <= 4000; i++) {
               BSONObject record = (BSONObject) JSON.parse("{_id:" + i + ",a:" + i + ", b:" + i + "}");
               records.add(record);
          }
          expList.clear();
          expList.addAll(records);
     }
}
