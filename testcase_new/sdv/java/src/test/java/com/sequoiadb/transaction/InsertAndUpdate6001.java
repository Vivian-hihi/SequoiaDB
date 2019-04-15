package com.sequoiadb.transaction;

import org.bson.BSONObject;
import org.bson.BasicBSONObject;
import org.testng.Assert;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.DBCursor;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.testcommon.SdbConfTestBase;
import com.sequoiadb.testcommon.SdbTestBase;
import com.sequoiadb.threadexecutor.ThreadExecutor;
import com.sequoiadb.threadexecutor.annotation.ExecuteOrder;

/**
 * test content: 事务1中更新数据为事务2中插入数据_SD.transaction.012
 * testlink-case: seqDB-6001
 * @author wangkexin
 * @Date 2019.04.08
 * @version 1.00
 */

public class InsertAndUpdate6001 extends SdbConfTestBase {
	private String clName = "cl6001";
	private Sequoiadb sdb = null;
	private Sequoiadb db1 = null;
	private Sequoiadb db2 = null;
	private DBCollection cl = null;

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
		es.addWorker(new TransInsert6001());
		es.addWorker(new TransUpdate6001());
		es.run();

		CheckResult();
	}

	@AfterClass
	private void teardown() {
		db1.close();
		db2.close();
		sdb.getCollectionSpace(SdbTestBase.csName).dropCollection(clName);
		sdb.close();
	}

	class TransInsert6001 {
		private DBCollection cl = null;

		public TransInsert6001() {
			db1 = new Sequoiadb(SdbTestBase.coordUrl, "", "");
			db1.beginTransaction();
			cl = db1.getCollectionSpace(SdbTestBase.csName).getCollection(clName);
		}

		@ExecuteOrder(step = 1, desc = "插入数据")
		public void Insert() {
			BSONObject obj = new BasicBSONObject();
			obj.put("a", 1);
			cl.insert(obj);
		}

		@ExecuteOrder(step = 3, desc = "提交事务")
		public void Commit() {
			db1.commit();
		}
	}

	class TransUpdate6001 {
		private DBCollection cl = null;

		public TransUpdate6001() {
			db2 = new Sequoiadb(SdbTestBase.coordUrl, "", "");
			db2.beginTransaction();
			cl = db2.getCollectionSpace(SdbTestBase.csName).getCollection(clName);
		}

		@ExecuteOrder(step = 2, desc = "更新数据")
		public void Update() {
			BSONObject matcher = new BasicBSONObject();
			matcher.put("a", 1);
			BSONObject modifyObj = new BasicBSONObject();
			BSONObject modifier = new BasicBSONObject();
			modifyObj.put("a", 2);
			modifier.put("$set", modifyObj);
			try {
				cl.update(matcher, modifier, null);
				Assert.fail("exp fail but found succ.");
			} catch (BaseException e) {
				Assert.assertEquals(e.getErrorCode(), -13);
			}
		}

		@ExecuteOrder(step = 4, desc = "提交事务")
		public void Commit() {
			db2.commit();
		}
	}

	private void CheckResult() {
		long expCount = 1;
		long actCount = cl.getCount();
		Assert.assertEquals(actCount, expCount);
		DBCursor cursor = cl.query();
		while (cursor.hasNext()) {
			Assert.assertEquals(cursor.getNext().get("a").toString(), "1");
		}
	}
}