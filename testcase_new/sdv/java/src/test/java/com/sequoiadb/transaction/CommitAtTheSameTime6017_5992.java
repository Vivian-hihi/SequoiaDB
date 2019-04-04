package com.sequoiadb.transaction;

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
import com.sequoiadb.testcommon.SdbConfTestBase;
import com.sequoiadb.testcommon.SdbTestBase;
import com.sequoiadb.threadexecutor.ThreadExecutor;
import com.sequoiadb.threadexecutor.annotation.ExecuteOrder;

/**
 * test content: 多个会话同时提交事务_SD.transaction.028、多个会话同时开启事务_SD.transaction.003 
 * testlink-case: seqDB-6017、seqDB-5992
 * @author wangkexin
 * @Date 2019.03.27
 * @version 1.00
 */

public class CommitAtTheSameTime6017_5992 extends SdbConfTestBase{
	private String clName = "cl6017";
	private Sequoiadb sdb = null;
	private DBCollection cl = null;
	private int threadNum = 100;
	private int insertNum = 1000;
	
	@Override
    protected void setNodeConf(){
        dataConf.put("transactionon", true);
    }
	
	@BeforeClass
	private void setup() {
		sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "");
		cl = sdb.getCollectionSpace(SdbTestBase.csName).createCollection(clName);
	}
	
	@Test
	public void test() throws Exception {
		ThreadExecutor es = new ThreadExecutor();
		for(int i = 0 ; i < threadNum; i++){
			es.addWorker(new Trans6017(clName, insertNum));
		}
        es.run();
        checkResult( threadNum*insertNum );
	}
	
	@AfterClass
	private void teardown() {
		sdb.getCollectionSpace(csName).dropCollection(clName);
		sdb.close();
	}
	
	private void checkResult(int expNum){
		int i = 0;
		BSONObject orderBy = new BasicBSONObject();
		orderBy.put("a", 1);
		BSONObject sel = new BasicBSONObject();
		BSONObject subSel = new BasicBSONObject();
		subSel.put("$include", 0);
		sel.put("_id",subSel);
		DBCursor cur = cl.query(null, sel, orderBy, null);
		while (cur.hasNext()) {
			for(int j = 0; j < threadNum; j++){
				BSONObject obj = cur.getNext();
				Assert.assertEquals(obj.get("a").toString(), String.valueOf(i), "insert data is wrong");
			}
            i++;
        }
		cur.close();
		Assert.assertEquals(i*threadNum, expNum);
	}
	
	class Trans6017 {
		private Sequoiadb db = null;
		private String clName;
		private int insertNum = 0;
		
		public Trans6017(String clName, int insertNum) {
			this.clName = clName;
			this.insertNum = insertNum;
		}
		
		@ExecuteOrder(step = 1, desc = "连接数据库，开启事务")
	    public void beginTrans() {
	    	db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
	    	db.beginTransaction();
	    }
		
		@ExecuteOrder(step = 2, desc = "插入数据")
		public void insertData() {
			DBCollection cl = db.getCollectionSpace(SdbTestBase.csName).getCollection(clName);
	    	insertData(cl, insertNum);
		}

	    @ExecuteOrder(step = 3, desc = "提交事务")
	    public void commit() {
	        db.commit();
	        db.close();
	    }

	    private void insertData(DBCollection cl, int recNum ) {
	    	List<BSONObject> insertor = new ArrayList<>();
	    	for(int i = 0 ; i < recNum ; i++){
	    		BSONObject rec = new BasicBSONObject();
	    		rec.put("a", i);
	    		insertor.add(rec);
	    	}
	    	cl.insert(insertor);
	    }
	}
}