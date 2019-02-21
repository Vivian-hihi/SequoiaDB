package com.sequoiadb.transaction.rc;
/**
 * @Description seqDB-17077: 更新记录与读记录并发，事务回滚
 * @author xiaoni Zhao
 * @date 2019-1-21
 */
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
@Test(groups = "rc")
public class UpdateRead17077_3 extends SdbTestBase {
	private String clName = "cl_17077_3";
	private Sequoiadb sdb = null;
	private Sequoiadb db1 = null;
	private Sequoiadb db2 = null;
	private DBCollection cl = null;
	private DBCollection cl1 = null;
	private DBCollection cl2 = null;
	private DBCursor cursor = null;
	private List<BSONObject> expList = new ArrayList<BSONObject>();
	private List<BSONObject> actList = new ArrayList<BSONObject>();
	
	@BeforeClass
    public void setUp(){
        sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "");
		db1 = new Sequoiadb(SdbTestBase.coordUrl, "", "");
		db2 = new Sequoiadb(SdbTestBase.coordUrl, "", "");
        cl = sdb.getCollectionSpace(csName).createCollection(clName);
		cl1 = db1.getCollectionSpace(csName).getCollection(clName);
		cl2 = db2.getCollectionSpace(csName).getCollection(clName);
        cl.createIndex("a", "{a:1, c:1}", false, false);
    }
	
	@Test
    public void test(){
		//集合中插入带索引的记录
		BSONObject insertR1 = (BSONObject)JSON.parse("{_id:1, a:1, b:1}");
        cl.insert(insertR1);
        expList.add(insertR1);
		
		db1.beginTransaction();
		db2.beginTransaction();
		
		//更新索引字段的值
		BSONObject updateR1 = (BSONObject)JSON.parse("{_id:1, a:2, b:1}");
		cl1.update("{a:1}", "{$set:{a:2}}", "{'':null}");
		
		//事务2表扫描记录
		cursor = cl2.query(null, null, null, "{'':null}");
		actList = TransUtils.getReadActList(cursor);
		Assert.assertEquals(actList, expList);
		actList.clear();
		
		//事务2索引扫描记录
		cursor = cl2.query(null, null, null, "{'':'a'}");
	    actList = TransUtils.getReadActList(cursor);
	    Assert.assertEquals(actList, expList);
	    actList.clear();
	    
	    //非事务表扫描记录
	    cursor = cl.query(null, null, null, "{'':null}");
  		actList = TransUtils.getReadActList(cursor);
  		expList.clear();
		expList.add(updateR1);
  		Assert.assertEquals(actList, expList);
  		actList.clear();
  		
  	    //非事务索引扫描记录
  		cursor = cl.query(null, null, null, "{'':'a'}");
  	    actList = TransUtils.getReadActList(cursor);
  	    Assert.assertEquals(actList, expList);
  	    actList.clear();
  	    
  	    db1.rollback();
  	    
  	    //事务2表扫描记录
  	    cursor = cl2.query(null, null, null, "{'':null}");
		actList = TransUtils.getReadActList(cursor);
		expList.clear();
		expList.add(insertR1);
		Assert.assertEquals(actList, expList);
		actList.clear();
		
  	    //事务2索引扫描记录
	    cursor = cl2.query(null, null, null, "{'':'a'}");
	    actList = TransUtils.getReadActList(cursor);
	    Assert.assertEquals(actList, expList);
	    actList.clear();
	    
  	    //非事务表扫描记录
	    cursor = cl.query(null, null, null, "{'':null}");
  		actList = TransUtils.getReadActList(cursor);
  		Assert.assertEquals(actList, expList);
  		actList.clear();
  		
  	    //非事务索引扫描记录
  		cursor = cl.query(null, null, null, "{'':'a'}");
  	    actList = TransUtils.getReadActList(cursor);
  	    Assert.assertEquals(actList, expList);
  	    actList.clear();
  	    
  	    //事务2提交
  	    db2.rollback();
  	    cursor.close();
	}
	
	@AfterClass
    public void tearDown(){
		if(!db1.isClosed()){
			db1.close();
		}
		if(!db2.isClosed()){
			db2.close();
		}
		CollectionSpace cs = sdb.getCollectionSpace(csName);
		if(cs.isCollectionExist(clName)){
			cs.dropCollection(clName);
		}
		if(!sdb.isClosed()){
			sdb.close();
		}
	}
}
