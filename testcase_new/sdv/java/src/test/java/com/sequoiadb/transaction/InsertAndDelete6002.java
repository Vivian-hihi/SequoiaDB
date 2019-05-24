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
//TODO：其他检视意见同  5999、6001 用例
/**
 * test content: 事务1中删除数据为事务2中插入数据_SD.transaction.013
 * testlink-case: seqDB-6002
 * @author wangkexin
 * @Date 2019.04.08
 * @version 1.00
 */
//TODO:文本用例预期结果有问题，插入跟删除事务并发，插入不会失败，删除会超时，文本用例写得不够明确，请修改文本用例预期结果
public class InsertAndDelete6002 extends SdbConfTestBase {
	private String clName = "cl6002";
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
	public void test() throws Exception {//TODO:并发在事务做数据操作，数据量要大一些，不然并发可能撞不上，如删除记录完成后才开始插入记录，应该没什么影响
		ThreadExecutor es = new ThreadExecutor();
		es.addWorker(new TransInsert6002());
		es.addWorker(new TransDelete6002());
		es.run();

		CheckResult();//TODO：需要分情况校验结果，任何一个
	}

	@AfterClass
	private void teardown() {
		try{
			sdb.getCollectionSpace(SdbTestBase.csName).dropCollection(clName);
		}finally{
			db1.close();
			db2.close();
			sdb.close();
		}
	}

	class TransInsert6002 {
		private DBCollection cl = null;

		public TransInsert6002() {
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

	class TransDelete6002 {
		private DBCollection cl = null;

		public TransDelete6002() {
			db2 = new Sequoiadb(SdbTestBase.coordUrl, "", "");
			db2.beginTransaction();
			cl = db2.getCollectionSpace(SdbTestBase.csName).getCollection(clName);
		}

		@ExecuteOrder(step = 2, desc = "删除数据")
		public void Delete() {
			BSONObject matcher = new BasicBSONObject();
			matcher.put("a", 1);
			try {
				cl.delete(matcher);
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