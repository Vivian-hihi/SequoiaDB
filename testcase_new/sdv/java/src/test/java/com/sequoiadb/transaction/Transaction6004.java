package com.sequoiadb.transaction;

import java.util.List;

import org.bson.BSONObject;
import org.bson.BasicBSONObject;
import org.testng.Assert;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import com.sequoiadb.base.DBCursor;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.testcommon.CommLib;
import com.sequoiadb.testcommon.SdbConfTestBase;
import com.sequoiadb.testcommon.SdbTestBase;

/**
 * test content: 配置事务锁超时时间值非法校验_SD.transaction.015(设置事务锁等待超时时间值为-1,a01,3601)
 * testlink-case: seqDB-6004
 * @author wangkexin
 * @Date 2019.04.08
 * @version 1.00
 */

public class Transaction6004 extends SdbConfTestBase {
	private Sequoiadb sdb = null;
	private int expTransTimeout1 = 60;
	private int expTransTimeout2 = 3600;
	private BSONObject options = new BasicBSONObject();

	@Override
	protected void setNodeConf() {
		dataConf.put("transactionon", true);
	}

	@BeforeClass
	private void setup() {
		sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "");
	}

	@Test
	public void test() throws Exception {
		List<String> dataGroupNames = CommLib.getDataGroupNames(sdb);
		List<String> nodeList = CommLib.getNodeAddress(sdb, dataGroupNames.get(0));
		String NodeName = nodeList.get(0);

		// test transactiontimeout is -1
		BSONObject configs1 = new BasicBSONObject();
		configs1.put("transactiontimeout", -1);
		options.put("NodeName", NodeName);
		try {
			sdb.updateConfig(configs1, options);
			Assert.fail("exp failed but succ.");
		} catch (BaseException e) {
			Assert.assertEquals(e.getErrorCode(), -264);
		}

		BSONObject selector = new BasicBSONObject();
		selector.put("transactiontimeout", 1);

		DBCursor cursor1 = sdb.getSnapshot(Sequoiadb.SDB_SNAP_CONFIGS, options, selector, null);
		while (cursor1.hasNext()) {
			int actValue = (int) cursor1.getNext().get("transactiontimeout");
			Assert.assertEquals(actValue, expTransTimeout1);
		}

		// test transactiontimeout is a01
		BSONObject configs2 = new BasicBSONObject();
		configs2.put("transactiontimeout", "a01");

		try {
			sdb.updateConfig(configs2, options);
			Assert.fail("exp failed but succ.");
		} catch (BaseException e) {
			Assert.assertEquals(e.getErrorCode(), -264);
		}

		DBCursor cursor2 = sdb.getSnapshot(Sequoiadb.SDB_SNAP_CONFIGS, options, selector, null);
		while (cursor2.hasNext()) {
			int actValue = (int) cursor2.getNext().get("transactiontimeout");
			Assert.assertEquals(actValue, expTransTimeout1);
		}

		// test transactiontimeout is 3601
		BSONObject configs3 = new BasicBSONObject();
		configs3.put("transactiontimeout", 3601);
		sdb.updateConfig(configs3, options);

		DBCursor cursor3 = sdb.getSnapshot(Sequoiadb.SDB_SNAP_CONFIGS, options, selector, null);
		while (cursor2.hasNext()) {
			int actValue = (int) cursor3.getNext().get("transactiontimeout");
			Assert.assertEquals(actValue, expTransTimeout2);
		}
	}

	@AfterClass
	private void teardown() {
		try{
			// 恢复环境
			BSONObject configs4 = new BasicBSONObject();
			configs4.put("transactiontimeout", 60);
			sdb.updateConfig(configs4, options);
		}finally{
			sdb.close();
		}
	}
}