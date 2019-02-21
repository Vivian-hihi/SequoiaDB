package com.sequoiadb.transaction.rcwaitlock;
/**
 * @Description seqDB-17167: 事务中批量更新与读并发 
 * @author xiaoni Zhao
 * @date 2019-1-22
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
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.testcommon.SdbTestBase;
import com.sequoiadb.testcommon.SdbThreadBase;
import com.sequoiadb.transaction.TransUtils;
@Test(groups = "rcwaitlock")
public class UpdateRead17167 extends SdbTestBase {
	private String clName = "cl_17167";
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
        cl.createIndex("a", "{a:1}", false, false);
        
        for(int i=0; i<50000; i++)
        {
        	BSONObject insertR = (BSONObject)JSON.parse("{_id:"+i+", a:1, b:1}");
			cl.insert(insertR);
        }
    }

	@Test
    public void test(){
		//开启两个并发事务
		db1.beginTransaction();
		db2.beginTransaction();
		
		//事务1执行批量更新
		cl1.update("{a:1}","{$set:{a:2}}", null);
		for(int i=0; i<50000; i++)
        {
        	BSONObject updateR = (BSONObject)JSON.parse("{_id:"+i+", a:2, b:1}");
			expList.add(updateR);
        }
		
		//事务2表扫描记录
		Read read1 = new Read("{'':null}");
		read1.start();
		Assert.assertTrue(read1.matchBlockingMethod(DBCursor.class.getName(), "hasNext"));
		
		//事务2索引扫描记录
		Read read2 = new Read("{'':'a'}");
		read2.start();
		Assert.assertTrue(read2.matchBlockingMethod(DBCursor.class.getName(), "hasNext"));
	    
		//非事务表扫描记录
	    cursor = cl.query(null, null, "{_id:1}", "{'':null}");
	    actList = TransUtils.getReadActList(cursor);
  		Assert.assertEquals(actList, expList);
  		actList.clear();
  		
  		//非事务索引扫描记录
  		cursor = cl.query(null, null, "{_id:1}", "{'':'a'}");
  		actList = TransUtils.getReadActList(cursor);
  	    Assert.assertEquals(actList, expList);
  	    actList.clear();
  	    
  	    db1.commit();
  	    
  	    //校验被阻塞线程返回的记录
  	    if(!read1.isSuccess() || !read2.isSuccess()){
			Assert.fail(read1.getErrorMsg()+read2.getErrorMsg());
		}
		try{
			List<BSONObject> actList = new ArrayList<BSONObject>();
			actList = (List<BSONObject>)read1.getExecResult();
			Assert.assertEquals(actList, expList);
			
			actList = (List<BSONObject>)read2.getExecResult();
			Assert.assertEquals(actList, expList);
		}catch(Exception e){
			Assert.fail(e.getMessage());
		}
  	    
  	    //事务2表扫描记录
  	    cursor = cl2.query(null, null, "{_id:1}", "{'':null}");
  	    actList = TransUtils.getReadActList(cursor);
		Assert.assertEquals(actList, expList);
		actList.clear();
		
		//事务2索引扫描记录
		cursor = cl2.query(null, null, "{_id:1}", "{'':'a'}");
		actList = TransUtils.getReadActList(cursor);
	    Assert.assertEquals(actList, expList);
	    actList.clear();
	    
	    //非事务表扫描记录
	    cursor = cl.query(null, null, "{_id:1}", "{'':null}");
	    actList = TransUtils.getReadActList(cursor);
  		Assert.assertEquals(actList, expList);
  		actList.clear();
  		
  		//非事务索引扫描记录
  		cursor = cl.query(null, null, "{_id:1}", "{'':'a'}");
  		actList = TransUtils.getReadActList(cursor);
  	    Assert.assertEquals(actList, expList);
  	    actList.clear();
  	    
  	    db2.commit();
  	    cursor.close();
	}
	
	private class Read extends SdbThreadBase{
        private String hint = null;
        private Sequoiadb db = null ;
        private DBCollection cl = null ;
        
		public Read(String hint) {
			// TODO Auto-generated constructor stub
			this.hint = hint ;
			db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
		    db.beginTransaction(); 
		    cl = db.getCollectionSpace(csName).getCollection(clName);
		}
		
		@Override
		public void exec() throws Exception {
			// TODO Auto-generated method stub
			try{
				DBCursor cursor = cl.query(null, null, "{_id:1}", hint);
				List<BSONObject> records = TransUtils.getReadActList(cursor);
				setExecResult(records);
			    db.commit(); 
			}catch(BaseException e){
			    db.rollback();
				db.close();
			}
		}
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
