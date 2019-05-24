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

/**  TODO:前2行用例名放到 @description seqDB-5999 多个事务并发.....。按规范来，不建议有各种不一样的风格
 * test content: 多个事务并发，同时更新/删除cl中不同记录并提交事务_SD.transaction.010
 * testlink-case: seqDB-5999
 * @author wangkexin
 * @Date 2019.03.15
 * @version 1.00
 */
public class UpdateAndDelete5999 extends SdbConfTestBase {
	private String clName = "cl5999";
	private Sequoiadb sdb = null;
	private Sequoiadb db1 = null;
	private Sequoiadb db2 = null;
	private DBCollection cl = null;
	private DBCollection cl1 = null;
	private DBCollection cl2 = null;
	private BSONObject del_matcher = new BasicBSONObject();

	@Override
	protected void setNodeConf() {
		dataConf.put("transactionon", true);
	}

	@BeforeClass
	public void setup() {//TODO:如下定义的常量命名，多个单词用“_”分开，不然全大写还简称字面上不好理解
		final int START = 0;
		final int RECSUM = 100;
		final int STRLENGTH = 10;
		sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "");
		cl = sdb.getCollectionSpace(SdbTestBase.csName).createCollection(clName);
		TransactionUtils.insertData(cl, START, RECSUM, STRLENGTH);//TODO：公共方法就 5999 、 6000用例在用，删掉公共方法类吧
	}

	@Test
	private void test() {
		db1 = new Sequoiadb(SdbTestBase.coordUrl, "", "");//TODO：线程里面用的连接都放到线程内部吧，不建议放外面
		db2 = new Sequoiadb(SdbTestBase.coordUrl, "", "");//TODO:线程里面建连接、开启事务、事务操作、提交事务、关闭连接，分多个步骤进行
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
		} else if (updateThread.isSuccess() && deleteThread.isSuccess()) {
			checkUpdateResult();
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
			matcher.put("b", 50);
			modifyObj.put("b", 5999);
			modifier.put("$set", modifyObj);
			cl1.update(matcher, modifier, null);
		}
	}

	private class DeleteThread extends SdbThreadBase {
		@Override
		public void exec() throws BaseException {
			del_matcher.put("a", 10);
			cl2.delete(del_matcher);
		}
	}

	private void checkUpdateResult() {
		BSONObject matcher = new BasicBSONObject();
		matcher.put("b", 5999);
		long actCount = cl.getCount(matcher);
		Assert.assertEquals(actCount, 1, "Update data does not exist!");
	}

	private void checkDeleteResult() {
		long count = cl.getCount(del_matcher);
		Assert.assertEquals(count, 0, "Deleted data still exists!");
	}
}
