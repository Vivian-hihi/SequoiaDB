package com.sequoiadb.transaction.ru;
/**
 * @Description seqDB-17231:更新复合索引，同时与读并发 
 * @author Zhao Xiaoni
 * @date 2019-1-17
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
@Test(groups = "ru")
public class UpdateRead17231 extends SdbTestBase {
	private String clName = "cl_17231";
	private Sequoiadb sdb = null;
	private Sequoiadb db1 = null;
	private Sequoiadb db2 = null;
	private DBCollection cl = null;
	private DBCollection cl1 = null;
	private DBCollection cl2 = null;
	private DBCursor tableCur = null;
	private DBCursor indexCur = null;
	private List<BSONObject> expList = new ArrayList<BSONObject>();
	private List<BSONObject> actIndexList = new ArrayList<BSONObject>();
	private List<BSONObject> actTableList = new ArrayList<BSONObject>();
	
	@BeforeClass
    public void setUp(){
        sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "");
        db1 = new Sequoiadb(SdbTestBase.coordUrl, "", "");
		db2 = new Sequoiadb(SdbTestBase.coordUrl, "", "");
        cl = sdb.getCollectionSpace(csName).createCollection(clName);
        cl1 = db1.getCollectionSpace(csName).getCollection(clName);
		cl2 = db2.getCollectionSpace(csName).getCollection(clName);
        cl.createIndex("a", "{a:1, b:1}", false, false);
        
        BSONObject insertR1 = (BSONObject)JSON.parse("{_id:1, a:1, b:1}");
		cl.insert(insertR1);
    }
	
	@Test
    public void test(){
		//开启两个并发事务
		db1.beginTransaction();
		db2.beginTransaction();
		
		//事务1更新全部索引字段值
		cl1.update(null, "{$set:{a:2, b:2}}", "{'':'a'}");
		BSONObject updateR1 = (BSONObject)JSON.parse("{_id:1, a:2, b:2}");
		expList.add(updateR1);
		
		//事务1表扫描记录
		tableCur = cl1.query(null, null, "{_id:1}", "{'':null}");
		actTableList = TransUtils.getReadActList(tableCur);
		Assert.assertEquals(actTableList, expList);
		actTableList.clear();
		
		//事务1索引扫描记录
		indexCur = cl1.query(null, null, "{_id:1}", "{'':'a'}");
	    actIndexList = TransUtils.getReadActList(indexCur);
	    Assert.assertEquals(actIndexList, expList);
	    actIndexList.clear();
			    
		//事务2表扫描记录
		tableCur = cl2.query(null, null, "{_id:1}", "{'':null}");
		actTableList = TransUtils.getReadActList(tableCur);
		Assert.assertEquals(actTableList, expList);
		actTableList.clear();
		
		//事务2索引扫描记录
		indexCur = cl2.query(null, null, "{_id:1}", "{'':'a'}");
	    actIndexList = TransUtils.getReadActList(indexCur);
	    Assert.assertEquals(actIndexList, expList);
	    actIndexList.clear();
	    
	    //非事务表扫描记录
  		tableCur = cl.query(null, null, "{_id:1}", "{'':null}");
  		actTableList = TransUtils.getReadActList(tableCur);
  		Assert.assertEquals(actTableList, expList);
  		actTableList.clear();
  		
  		//非事务索引扫描记录
  		indexCur = cl.query(null, null, "{_id:1}", "{'':'a'}");
  	    actIndexList = TransUtils.getReadActList(indexCur);
  	    Assert.assertEquals(actIndexList, expList);
  	    actIndexList.clear();
  	    
  	    db1.commit();
  	    
  	    //事务2表扫描记录
	    tableCur = cl2.query(null, null, "{_id:1}", "{'':null}");
		actTableList = TransUtils.getReadActList(tableCur);
		Assert.assertEquals(actTableList, expList);
		actTableList.clear();
		
		//事务2索引扫描记录
		indexCur = cl2.query(null, null, "{_id:1}", "{'':'a'}");
	    actIndexList = TransUtils.getReadActList(indexCur);
	    Assert.assertEquals(actIndexList, expList);
	    actIndexList.clear();
	    
	    //非事务表扫描记录
  		tableCur = cl.query(null, null, "{_id:1}", "{'':null}");
  		actTableList = TransUtils.getReadActList(tableCur);
  		Assert.assertEquals(actTableList, expList);
  		actTableList.clear();
  		
  		//非事务索引扫描记录
  		indexCur = cl.query(null, null, "{_id:1}", "{'':'a'}");
  	    actIndexList = TransUtils.getReadActList(indexCur);
  	    Assert.assertEquals(actIndexList, expList);
  	    actIndexList.clear();
  	    
  	    //事务2提交
  	    db2.commit();
  	    
  	    //非事务表扫描记录
  		tableCur = cl.query(null, null, "{_id:1}", "{'':null}");
  		actTableList = TransUtils.getReadActList(tableCur);
  		Assert.assertEquals(actTableList, expList);
  		actTableList.clear();
  		
  		//非事务索引扫描记录
  		indexCur = cl.query(null, null, "{_id:1}", "{'':'a'}");
  	    actIndexList = TransUtils.getReadActList(indexCur);
  	    Assert.assertEquals(actIndexList, expList);
  	    actIndexList.clear();
  	    
  	    //重新开启两个并发事务
  	    db1.beginTransaction();
  	    db2.beginTransaction();
  	    
  	    //事务1删除部分索引字段
  	    cl1.update(null,"{$unset:{a:2}}", "{'':'a'}");
  	    BSONObject updateR2 = (BSONObject)JSON.parse("{_id:1, b:2}");
  	    expList.clear();
		expList.add(updateR2);
		
		//事务1表扫描记录
		tableCur = cl1.query(null, null, "{_id:1}", "{'':null}");
		actTableList = TransUtils.getReadActList(tableCur);
		Assert.assertEquals(actTableList, expList);
		actTableList.clear();
		
		//事务1索引扫描记录
		indexCur = cl1.query(null, null, "{_id:1}", "{'':'a'}");
	    actIndexList = TransUtils.getReadActList(indexCur);
	    Assert.assertEquals(actIndexList, expList);
	    actIndexList.clear();
		
		//事务2表扫描记录
		tableCur = cl2.query(null, null, "{_id:1}", "{'':null}");
		actTableList = TransUtils.getReadActList(tableCur);
		Assert.assertEquals(actTableList, expList);
		actTableList.clear();
		
		//事务2索引扫描记录
		indexCur = cl2.query(null, null, "{_id:1}", "{'':'a'}");
	    actIndexList = TransUtils.getReadActList(indexCur);
	    Assert.assertEquals(actIndexList, expList);
	    actIndexList.clear();
	    
	    //非事务表扫描记录
  		tableCur = cl.query(null, null, "{_id:1}", "{'':null}");
  		actTableList = TransUtils.getReadActList(tableCur);
  		Assert.assertEquals(actTableList, expList);
  		actTableList.clear();
  		
  		//非事务索引扫描记录
  		indexCur = cl.query(null, null, "{_id:1}", "{'':'a'}");
  	    actIndexList = TransUtils.getReadActList(indexCur);
  	    Assert.assertEquals(actIndexList, expList);
  	    actIndexList.clear();
  	    
  	    db1.commit();
  	    
  	    //事务2表扫描记录
	    tableCur = cl2.query(null, null, "{_id:1}", "{'':null}");
		actTableList = TransUtils.getReadActList(tableCur);
		Assert.assertEquals(actTableList, expList);
		actTableList.clear();
		
		//事务2索引扫描记录
		indexCur = cl2.query(null, null, "{_id:1}", "{'':'a'}");
	    actIndexList = TransUtils.getReadActList(indexCur);
	    Assert.assertEquals(actIndexList, expList);
	    actIndexList.clear();
	    
	    //非事务表扫描记录
  		tableCur = cl.query(null, null, "{_id:1}", "{'':null}");
  		actTableList = TransUtils.getReadActList(tableCur);
  		Assert.assertEquals(actTableList, expList);
  		actTableList.clear();
  		
  		//非事务索引扫描记录
  		indexCur = cl.query(null, null, "{_id:1}", "{'':'a'}");
  	    actIndexList = TransUtils.getReadActList(indexCur);
  	    Assert.assertEquals(actIndexList, expList);
  	    actIndexList.clear();
  	    
  	    //事务2提交
  	    db2.commit();
  	    
  	    //非事务表扫描记录
  		tableCur = cl.query(null, null, "{_id:1}", "{'':null}");
  		actTableList = TransUtils.getReadActList(tableCur);
  		Assert.assertEquals(actTableList, expList);
  		actTableList.clear();
  		
  		//非事务索引扫描记录
  		indexCur = cl.query(null, null, "{_id:1}", "{'':'a'}");
  	    actIndexList = TransUtils.getReadActList(indexCur);
  	    Assert.assertEquals(actIndexList, expList);
  	    actIndexList.clear();
  	    
  	    //重新开启两个并发事务
  	    db1.beginTransaction();
  	    db2.beginTransaction();
  	    
  	    //事务1新增部分索引字段
  	    cl1.update(null, "{$set:{a:2}}", "{'':'a'}");
  	    BSONObject updateR3 = (BSONObject)JSON.parse("{_id:1, a:2, b:2}");
  	    expList.clear();
  	    expList.add(updateR3);
  	    
  	    //事务1表扫描记录
		tableCur = cl1.query(null, null, "{_id:1}", "{'':null}");
		actTableList = TransUtils.getReadActList(tableCur);
		Assert.assertEquals(actTableList, expList);
		actTableList.clear();
		
		//事务1索引扫描记录
		indexCur = cl1.query(null, null, "{_id:1}", "{'':'a'}");
	    actIndexList = TransUtils.getReadActList(indexCur);
	    Assert.assertEquals(actIndexList, expList);
	    actIndexList.clear();
  		    
  	    //事务2表扫描记录
		tableCur = cl2.query(null, null, "{_id:1}", "{'':null}");
		actTableList = TransUtils.getReadActList(tableCur);
		Assert.assertEquals(actTableList, expList);
		actTableList.clear();
		
		//事务2索引扫描记录
		indexCur = cl2.query(null, null, "{_id:1}", "{'':'a'}");
	    actIndexList = TransUtils.getReadActList(indexCur);
	    Assert.assertEquals(actIndexList, expList);
	    actIndexList.clear();
	    
	    //非事务表扫描记录
  		tableCur = cl.query(null, null, "{_id:1}", "{'':null}");
  		actTableList = TransUtils.getReadActList(tableCur);
  		Assert.assertEquals(actTableList, expList);
  		actTableList.clear();
  		
  		//非事务索引扫描记录
  		indexCur = cl.query(null, null, "{_id:1}", "{'':'a'}");
  	    actIndexList = TransUtils.getReadActList(indexCur);
  	    Assert.assertEquals(actIndexList, expList);
  	    actIndexList.clear();
  	    
  	    db1.commit();
  	    
  	    //事务2表扫描记录
	    tableCur = cl2.query(null, null, "{_id:1}", "{'':null}");
		actTableList = TransUtils.getReadActList(tableCur);
		Assert.assertEquals(actTableList, expList);
		actTableList.clear();
		
		//事务2索引扫描记录
		indexCur = cl2.query(null, null, "{_id:1}", "{'':'a'}");
	    actIndexList = TransUtils.getReadActList(indexCur);
	    Assert.assertEquals(actIndexList, expList);
	    actIndexList.clear();
	    
	    //非事务表扫描记录
  		tableCur = cl.query(null, null, "{_id:1}", "{'':null}");
  		actTableList = TransUtils.getReadActList(tableCur);
  		Assert.assertEquals(actTableList, expList);
  		actTableList.clear();
  		
  		//非事务索引扫描记录
  		indexCur = cl.query(null, null, "{_id:1}", "{'':'a'}");
  	    actIndexList = TransUtils.getReadActList(indexCur);
  	    Assert.assertEquals(actIndexList, expList);
  	    actIndexList.clear();
  	    
  	    //事务2提交
  	    db2.commit();
  	    
  	    //非事务表扫描记录
  		tableCur = cl.query(null, null, "{_id:1}", "{'':null}");
  		actTableList = TransUtils.getReadActList(tableCur);
  		Assert.assertEquals(actTableList, expList);
  		actTableList.clear();
  		
  		//非事务索引扫描记录
  		indexCur = cl.query(null, null, "{_id:1}", "{'':'a'}");
  	    actIndexList = TransUtils.getReadActList(indexCur);
  	    Assert.assertEquals(actIndexList, expList);
  	    actIndexList.clear();
  	    
  	    //重新开启两个并发事务
  	    db1.beginTransaction();
  	    db2.beginTransaction();
  	    
  	    //事务1更新部分字段的值
  	    cl1.update(null, "{$set:{a:3}}", "{'':'a'}");
  	    BSONObject updateR4 = (BSONObject)JSON.parse("{_id:1, a:3, b:2}");
  	    expList.clear();
  	    expList.add(updateR4);
  	    
  	    //事务1表扫描记录
		tableCur = cl1.query(null, null, "{_id:1}", "{'':null}");
		actTableList = TransUtils.getReadActList(tableCur);
		Assert.assertEquals(actTableList, expList);
		actTableList.clear();
		
		//事务1索引扫描记录
		indexCur = cl1.query(null, null, "{_id:1}", "{'':'a'}");
	    actIndexList = TransUtils.getReadActList(indexCur);
	    Assert.assertEquals(actIndexList, expList);
	    actIndexList.clear();
  	    
  	    //事务2表扫描记录
		tableCur = cl2.query(null, null, "{_id:1}", "{'':null}");
		actTableList = TransUtils.getReadActList(tableCur);
		Assert.assertEquals(actTableList, expList);
		actTableList.clear();
		
		//事务2索引扫描记录
		indexCur = cl2.query(null, null, "{_id:1}", "{'':'a'}");
	    actIndexList = TransUtils.getReadActList(indexCur);
	    Assert.assertEquals(actIndexList, expList);
	    actIndexList.clear();
	    
	    //非事务表扫描记录
  		tableCur = cl.query(null, null, "{_id:1}", "{'':null}");
  		actTableList = TransUtils.getReadActList(tableCur);
  		Assert.assertEquals(actTableList, expList);
  		actTableList.clear();
  		
  		//非事务索引扫描记录
  		indexCur = cl.query(null, null, "{_id:1}", "{'':'a'}");
  	    actIndexList = TransUtils.getReadActList(indexCur);
  	    Assert.assertEquals(actIndexList, expList);
  	    actIndexList.clear();
  	    
  	    db1.commit();
  	    
  	    //事务2表扫描记录
	    tableCur = cl2.query(null, null, "{_id:1}", "{'':null}");
		actTableList = TransUtils.getReadActList(tableCur);
		Assert.assertEquals(actTableList, expList);
		actTableList.clear();
		
		//事务2索引扫描记录
		indexCur = cl2.query(null, null, "{_id:1}", "{'':'a'}");
	    actIndexList = TransUtils.getReadActList(indexCur);
	    Assert.assertEquals(actIndexList, expList);
	    actIndexList.clear();
	    
	    //非事务表扫描记录
  		tableCur = cl.query(null, null, "{_id:1}", "{'':null}");
  		actTableList = TransUtils.getReadActList(tableCur);
  		Assert.assertEquals(actTableList, expList);
  		actTableList.clear();
  		
  		//非事务索引扫描记录
  		indexCur = cl.query(null, null, "{_id:1}", "{'':'a'}");
  	    actIndexList = TransUtils.getReadActList(indexCur);
  	    Assert.assertEquals(actIndexList, expList);
  	    actIndexList.clear();
  	    
  	    //事务2提交
  	    db2.commit();
  	    
  	    //非事务表扫描记录
  		tableCur = cl.query(null, null, "{_id:1}", "{'':null}");
  		actTableList = TransUtils.getReadActList(tableCur);
  		Assert.assertEquals(actTableList, expList);
  		actTableList.clear();
  		
  		//非事务索引扫描记录
  		indexCur = cl.query(null, null, "{_id:1}", "{'':'a'}");
  	    actIndexList = TransUtils.getReadActList(indexCur);
  	    Assert.assertEquals(actIndexList, expList);
    }
	
	@AfterClass
    public void tearDown(){
		indexCur.close();
		tableCur.close();
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
