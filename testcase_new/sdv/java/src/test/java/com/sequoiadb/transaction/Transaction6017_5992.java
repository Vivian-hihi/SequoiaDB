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
//TODO：其他检视意见同 5999 用例
/**
 * test content: 多个会话同时提交事务_SD.transaction.028、多个会话同时开启事务_SD.transaction.003 
 * testlink-case: seqDB-6017、seqDB-5992
 * @author wangkexin
 * @Date 2019.03.27
 * @version 1.00
 */
//TODO: 5990 用例没有提交？
public class Transaction6017_5992 extends SdbConfTestBase {
	private String clName = "cl6017";
	private Sequoiadb sdb = null;
	private DBCollection cl = null;
	private int threadNum = 100;
	private int insertNum = 1000;

	@Override
	protected void setNodeConf() {
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
		for (int i = 0; i < threadNum; i++) {
			es.addWorker(new Trans6017());
		}
		es.run();
		checkResult(threadNum * insertNum);
	}

	@AfterClass
	private void teardown() {
		try{
			sdb.getCollectionSpace(csName).dropCollection(clName);
		}finally{
			sdb.close();
		}
	}

	private void checkResult(int expNum) {
		int actCount = (int) cl.getCount();
		int count = 0;
		BSONObject orderBy = new BasicBSONObject();
		orderBy.put("a", 1);
		DBCursor cur = cl.query(null, null, orderBy, null);
		while (cur.hasNext()) {
			for (int j = 0; j < threadNum; j++) {
				BSONObject obj = cur.getNext();
				Assert.assertEquals((int) obj.get("a"), count, "insert data is wrong");
			}
			count++;
		}
		Assert.assertEquals(actCount, expNum);
	}

	class Trans6017 {//TODO:测试点覆盖了6017/5992用例，类名要改下，测试点都在同一个类时类名可以不用加用例ID
		private Sequoiadb db = null;

		@ExecuteOrder(step = 1, desc = "连接数据库，开启事务")
		private void beginTrans() {
			db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
			db.beginTransaction();
		}

		@ExecuteOrder(step = 2, desc = "插入数据")
		private void insertData() {
			DBCollection cl = db.getCollectionSpace(SdbTestBase.csName).getCollection(clName);
			insertData(cl);
		}

		@ExecuteOrder(step = 3, desc = "提交事务")
		private void commit() {
			try {
				db.commit();
			} finally {
				db.close();
			}
		}

		private void insertData(DBCollection cl) {
			List<BSONObject> insertor = new ArrayList<>();
			for (int i = 0; i < insertNum; i++) {
				BSONObject rec = new BasicBSONObject();
				rec.put("a", i);
				insertor.add(rec);
			}
			cl.insert(insertor);
		}
	}
}