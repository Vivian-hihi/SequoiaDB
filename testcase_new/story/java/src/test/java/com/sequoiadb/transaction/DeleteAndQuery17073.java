package com.sequoiadb.transaction;

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

/**
 * @testcase seqDB-17073:同一事务下，删除记录后读记录
 * @date 2019-1-18
 * @author yinzhen
 *
 */
@Test(groups = "rc")
public class DeleteAndQuery17073 extends SdbTestBase {
     private Sequoiadb sdb = null;
     private String clName = "cl17073";
     private DBCollection cl = null;
     private List<BSONObject> expList = new ArrayList<BSONObject>();
     private List<BSONObject> actList = new ArrayList<BSONObject>();

     @BeforeClass
     public void setUp() {
          sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "");
          cl = sdb.getCollectionSpace(csName).createCollection(clName);
          cl.createIndex("textIndex17073", "{a:1}", false, false);
          BSONObject record = (BSONObject) JSON.parse("{_id:1, a:1, b:1}");
          cl.insert(record);
     }

     @AfterClass
     public void tearDown() {
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
          // 开启事务
          sdb.beginTransaction();

          // 删除记录R1
          cl.delete("", "{'':'textIndex17073'}");

          // 读记录走表扫描
          DBCursor recordsCursor = cl.query(null, null, null, "{'':null}");
          DBCursor explainCursor = cl.explain(null, null, null, (BSONObject) JSON.parse("{'':null}"), 0, 1, 0, null);
          while (explainCursor.hasNext()) {
               String scanType = (String) explainCursor.getNext().get("ScanType");
               Assert.assertEquals(scanType, "tbscan");
          }
          actList = TransUtils.getReadActList(recordsCursor);
          expList.clear();
          Assert.assertEquals(actList, expList);

          // 读记录走索引扫描
          recordsCursor = cl.query("{a:{$exists:1}}", null, null, "{'':'textIndex17073'}");
          explainCursor = cl.explain((BSONObject) JSON.parse("{a:{$exists:1}}"), null, null,
                    (BSONObject) JSON.parse("{'':'textIndex17073'}"), 0, 1, 0, null);
          while (explainCursor.hasNext()) {
               String scanType = (String) explainCursor.getNext().get("ScanType");
               Assert.assertEquals(scanType, "ixscan");
          }
          actList = TransUtils.getReadActList(recordsCursor);
          Assert.assertEquals(actList, expList);

          // 事务提交
          sdb.commit();

          // 读记录走表扫描
          recordsCursor = cl.query(null, null, null, "{'':null}");
          actList = TransUtils.getReadActList(recordsCursor);
          Assert.assertEquals(actList, expList);

          // 读记录走索引扫描
          recordsCursor = cl.query("{a:{$exists:1}}", null, null, "{'':'textIndex17073'}");
          actList = TransUtils.getReadActList(recordsCursor);
          Assert.assertEquals(actList, expList);
          recordsCursor.close();
          explainCursor.close();

     }
}
