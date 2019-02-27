package com.sequoiadb.transaction.rc;

import java.util.ArrayList;
import java.util.List;

import org.bson.BSONObject;
import org.bson.BasicBSONObject;
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
 * @testcase seqDB-17142:固定集合中增删改记录，与读并发
 * @date 2019-1-24
 * @author yinzhen
 */
@Test(groups = "rc")
public class Transaction17142 extends SdbTestBase {
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
     private BSONObject object2 = null;

     @BeforeClass
     public void setUp() {
          sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "");
          cappedCL = sdb.createCollectionSpace(csName, (BSONObject) JSON.parse("{Capped:true}"))
                    .createCollection(clName, (BSONObject) JSON.parse("{Capped:true, Size:1024}"));
          //TODO:定义一个全局变量就可以了吧，不同的记录赋不同值，最好是能用JSON.parse强转，其他地方一并修改
          BSONObject object1 = new BasicBSONObject();
          long oid = 0L;
          object1.put("_id", oid);
          object1.put("a",1);
          object1.put("b",1);
          cappedCL.insert(object1);
          expList.add(object1);
          object2 = new BasicBSONObject();
          oid = 64L;
          object2.put("_id", oid);
          object2.put("a",2);
          object2.put("b",2);
          cappedCL.insert(object2);
          expList.add(object2);
     }

     @AfterClass
     public void tearDown() {
          if (!db1.isClosed()) {
               db1.close();
          }
          if (!db2.isClosed()) {
               db2.close();
          }
          //TODO:直接删除cs
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
          long oid = 128L;
          BSONObject object = new BasicBSONObject();
          object.put("_id", oid);
          object.put("a",3);
          object.put("b",3);
          cl1.insert(object);
          expList.add(object);
          DBCursor recordsCursor = cl1.query(null, null, "{a:1}", "{'':null}");
          actList = TransUtils.getReadActList(recordsCursor);
          Assert.assertEquals(actList, expList);

          // 事务2读记录走表扫描
          recordsCursor = cl2.query(null, null, "{a:1}", "{'':null}");
          actList = TransUtils.getReadActList(recordsCursor);
          Assert.assertEquals(actList, expList);

          // 事务1执行pop操作
          recordsCursor = cl1.query(null, null, "{a:1}", "{'':null}");
          actList = TransUtils.getReadActList(recordsCursor);
          //TODO:oid已使用变量定义，此处最好使用变量执行pop操作
          cl1.pop((BSONObject) JSON.parse("{LogicalID:0, Direction:1}"));
          cl1.pop((BSONObject) JSON.parse("{LogicalID:128, Direction:-1}"));
          expList.clear();
          expList.add(object2);

          // 事务1读记录走表扫描
          recordsCursor = cl1.query(null, null, "{a:1}", "{'':null}");
          actList = TransUtils.getReadActList(recordsCursor);
          Assert.assertEquals(actList, expList);

          // 事务2读记录走表扫描
          recordsCursor = cl2.query(null, null, "{a:1}", "{'':null}");
          actList = TransUtils.getReadActList(recordsCursor);
          Assert.assertEquals(actList, expList);

          // 回滚事务1，并读记录走表扫描
          db1.rollback();
          recordsCursor = cl1.query(null, null, "{a:1}", "{'':null}");
          actList = TransUtils.getReadActList(recordsCursor);
          Assert.assertEquals(actList, expList);

          // 事务2读记录走表扫描
          recordsCursor = cl2.query(null, null, "{a:1}", "{'':null}");
          actList = TransUtils.getReadActList(recordsCursor);
          Assert.assertEquals(actList, expList);
          
          // 事务2回滚
          db2.rollback();
          recordsCursor.close();
     }
}
