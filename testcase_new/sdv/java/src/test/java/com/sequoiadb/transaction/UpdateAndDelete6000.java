package com.sequoiadb.transaction;

import org.bson.BSONObject;
import org.bson.BasicBSONObject;
import org.testng.Assert;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.testcommon.SdbConfTestBase;
import com.sequoiadb.testcommon.SdbTestBase;
import com.sequoiadb.testcommon.SdbThreadBase;

/**
 * test content: 多个事务并发，同时更新/删除cl中不同记录并提交事务_SD.transaction.010
 * testlink-case: seqDB-6000
 * @author wangkexin
 * @Date 2019.03.15
 * @version 1.00
 */
public class UpdateAndDelete6000 extends SdbConfTestBase {
	private String clName = "cl6000";
	private Sequoiadb sdb = null;
	private Sequoiadb db1 = null;
	private Sequoiadb db2 = null;
	private DBCollection cl = null;
	private DBCollection cl1 = null;
	private DBCollection cl2 = null;

	@Override
	protected void setNodeConf() {
		dataConf.put("transactionon", true);
	}

	@BeforeClass
	public void setup() {
		final int START = 0;
		final int RECSUM = 100;
		final int STRLENGTH = 10;
		sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "");
		cl = sdb.getCollectionSpace(SdbTestBase.csName).createCollection(clName);

		// insertData(DBCollection cl,int start, int recSum, int strLength)
		TransactionUtils.insertData(cl, START, RECSUM, STRLENGTH);
	}

	@Test
	private void test() {
		db1 = new Sequoiadb(SdbTestBase.coordUrl, "", "");
		db2 = new Sequoiadb(SdbTestBase.coordUrl, "", "");

		db1.beginTransaction();
		db2.beginTransaction();

		cl1 = db1.getCollectionSpace(SdbTestBase.csName).getCollection(clName);
		cl2 = db2.getCollectionSpace(SdbTestBase.csName).getCollection(clName);

		UpdateThread updateThread = new UpdateThread();
		DeleteThread deleteThread = new DeleteThread();
		updateThread.start();
		deleteThread.start();

		if (updateThread.isSuccess() && !deleteThread.isSuccess()) {
			BaseException e = (BaseException) (deleteThread.getExceptions().get(0));
			if (e.getErrorCode() != -13) {
				Assert.fail("delete thread fail:" + deleteThread.getErrorMsg() + "  e:" + e.getErrorCode());
			}
			checkUpdateResult();
		} else if (!updateThread.isSuccess() && deleteThread.isSuccess()) {
			BaseException e = (BaseException) (updateThread.getExceptions().get(0));
			if (e.getErrorCode() != -13) {
				Assert.fail("update thread fail:" + updateThread.getErrorMsg() + "  e:" + e.getErrorCode());
			}
			checkDeleteResult();
		} else {
			Assert.fail("Unexpected results! updateThreadError:" + updateThread.getErrorMsg() + "deleteThreadError:"
					+ deleteThread.getErrorMsg());
		}

		db1.commit();
		db2.commit();
	}

	@AfterClass
	public void teardown() {
		try{
			sdb.getCollectionSpace(csName).dropCollection(clName);
		}finally{
			db1.close();
			db2.close();
			sdb.close();
		}
	}

	private class UpdateThread extends SdbThreadBase {
		@Override
		public void exec() throws BaseException {
			BSONObject matcher = new BasicBSONObject();
			BSONObject modifyObj = new BasicBSONObject();
			BSONObject modifier = new BasicBSONObject();
			matcher.put("a", 50);
			modifyObj.put("a", 6000);
			modifier.put("$set", modifyObj);
			cl1.update(matcher, modifier, null);
		}
	}

	private class DeleteThread extends SdbThreadBase {
		@Override
		public void exec() throws BaseException {
			cl2.delete("");
		}
	}

	private void checkUpdateResult() {
		BSONObject matcher = new BasicBSONObject();
		matcher.put("a", 6000);
		long actCount = cl.getCount(matcher);
		Assert.assertEquals(actCount, 1, "Update data does not exist!");
	}

	private void checkDeleteResult() {
		long actCount = cl.getCount();
		Assert.assertEquals(actCount, 0, "there still have data");
	}
}
