package com.sequoiadb.split;

import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Date;
import java.util.concurrent.atomic.AtomicInteger;

import org.bson.BSONObject;
import org.bson.BasicBSONObject;
import org.bson.util.JSON;
import org.testng.Assert;
import org.testng.annotations.AfterTest;
import org.testng.annotations.BeforeTest;
import org.testng.annotations.Test;

import com.sequoiadb.base.CollectionSpace;
import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.DBCursor;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.testcommon.CommLib;
import com.sequoiadb.testcommon.SdbTestBase;
import com.sequoiadb.util.MySdbTools;

/**
 * @FileName:SEQDB-8583 1、在cl下指定范围条件进行数据切分 2、分别验证如下两种场景：
 *                      a、执行split操作，任务已下发开始执行切分（可通过listTasks查看任务已下发，
 *                      直连目标节点查看数据已开始迁移），同时删除cl b、执行split操作，当切分任务还没开始执行时，删除cl
 *                      3、查看数据切分是否成功，删除cl是否成功
 * @author huangqiaohui
 * @version 1.00
 *
 */

public class Split8583b extends SdbTestBase {
	private String clName = "testcaseCL8583b";
	private String srcGroupName;
	private String destGroupName;
	private AtomicInteger a = new AtomicInteger();
	private boolean isStandAlone;
	private boolean isGroupTooless;

	@BeforeTest()
	public void setUp() {
		Sequoiadb sdb = null;
		try {
			System.out.println("the TestCase Name:" + this.getClass().getName() + ". the TestCase begin at:"
					+ new SimpleDateFormat("YYYY-MM-dd HH:mm:ss.SSS").format(new Date()));
			sdb = new Sequoiadb(coordUrl, "", "");
			CommLib commlib = new CommLib();
			isStandAlone = commlib.isStandAlone(sdb);
			if (isStandAlone) {
				return;
			}
			CollectionSpace commCS = sdb.getCollectionSpace(csName);
			MySdbTools.createCL(clName, commCS, "{ShardingKey:{\"a\":1},ShardingType:\"range\"}");
			ArrayList<String> tmp = MySdbTools.getGroupName(sdb, csName, clName);
			if (tmp.size() != 2) {
				isGroupTooless = true;
				return;
			}
		} catch (Exception e) {
			tearDown();
			Assert.fail("TestCase8583b setUp error, error description:" + e.getMessage());
		} finally {
			if (sdb != null)
				sdb.disconnect();
		}
	}

	// 写入待切分的记录（1000）
	@Test(groups = "insertdata")
	public void beforSplitInsertData() {
		if (isStandAlone || isGroupTooless) {
			return;
		}
		Sequoiadb db = null;
		try {
			db = new Sequoiadb(coordUrl, "", "");
			DBCollection cl = db.getCollectionSpace(csName).getCollection(clName);
			ArrayList<BSONObject> arr = new ArrayList<>();
			for (int i = 0; i < 1000; i++) {
				arr.add(getBasicBSON());
			}
			cl.bulkInsert(arr, MySdbTools.FLG_INSERT_CONTONDUP);
		} catch (BaseException e) {
			Assert.fail(e.getMessage());
		} finally {
			if (db != null)
				db.disconnect();
		}

	}

	// 获取集合所在组名，和切分目标组名
	@Test(dependsOnMethods = "beforSplitInsertData")
	public void getGroupName() {
		if (isStandAlone || isGroupTooless) {
			return;
		}
		Sequoiadb sdb = null;
		a.set(0);
		try {
			sdb = new Sequoiadb(coordUrl, "", "");
			ArrayList<String> tmp = MySdbTools.getGroupName(sdb, csName, clName);
			srcGroupName = tmp.get(0);
			destGroupName = tmp.get(1);

		} catch (BaseException e) {
			Assert.fail(e.getMessage());
		} catch (Exception e) {
			Assert.fail(e.getMessage());
			e.printStackTrace();
		} finally {
			if (sdb != null)
				sdb.disconnect();
		}
	}

	// 切分删除CL
	@Test(dependsOnMethods = "getGroupName", groups = "splitAndDelete_8583b", timeOut = 60000, enabled = true)
	public void dropCL() {
		if (isStandAlone || isGroupTooless) {
			return;
		}
		Sequoiadb sdb = null;
		try {
			sdb = new Sequoiadb(coordUrl, "", "");
			sdb.getCollectionSpace(csName).dropCollection(clName);
		} catch (BaseException e) {
			Assert.fail(e.getMessage());
		} finally {
			if (sdb != null)
				sdb.disconnect();
		}
	}

	// 切分
	@Test(dependsOnMethods = "getGroupName", groups = "splitAndDelete_8583b")
	public synchronized void splitCL() {
		if (isStandAlone || isGroupTooless) {
			return;
		}
		Sequoiadb sdb = null;
		try {
			sdb = new Sequoiadb(coordUrl, "", "");
			DBCollection cl = sdb.getCollectionSpace(csName).getCollection(clName);
			if (cl == null) {
				return;
			}
			cl.splitAsync(srcGroupName, destGroupName, (BSONObject) JSON.parse("{a:0}"),
					(BSONObject) JSON.parse("{a:100}"));
		} catch (BaseException e) {
			Assert.assertEquals(e.getErrorCode() == -23 || e.getErrorCode() == -147, true, e.getMessage());
		} catch (Exception e) {
			Assert.fail(e.getMessage());
		} finally {
			if (sdb != null)
				sdb.disconnect();
		}
	}

	// 检查切分后结果
	@Test(dependsOnGroups = "splitAndDelete_8583b")
	public void checkReault() {
		if (isStandAlone || isGroupTooless) {
			return;
		}
		Sequoiadb sdb = null;
		try {
			sdb = new Sequoiadb(coordUrl, "", "");
			if (sdb.getCollectionSpace(csName).isCollectionExist(clName)) {
				Assert.fail("CL delete fail");
			}
			DBCursor dbc = sdb.listTasks((BSONObject) JSON.parse("{Name:\"" + csName + "." + clName + "\"}"), null,
					null, null);
			if (dbc.hasNext()) {
				Assert.fail("split task dose not cancel");
			}
		} catch (BaseException e) {
			Assert.fail(e.getMessage());
		} finally {
			if (sdb != null)
				sdb.disconnect();
		}
	}

	public BSONObject getBasicBSON() {
		BasicBSONObject obj = null;
		obj = (BasicBSONObject) JSON.parse("{a:" + a.getAndIncrement() + "}");
		return obj;
	}

	@AfterTest(enabled = true)
	public void tearDown() {
		System.out.println("the TestCase Name:" + this.getClass().getName() + ". the TestCase end at:"
				+ new SimpleDateFormat("YYYY-MM-dd HH:mm:ss.SSS").format(new Date()));
		Sequoiadb sdb = new Sequoiadb(coordUrl, "", "");
		try {
			CollectionSpace commCS = sdb.getCollectionSpace(csName);
			if (commCS != null) {
				if (commCS.isCollectionExist(clName))
					commCS.dropCollection(clName);
			}
		} catch (BaseException e) {
			// do something??
		} finally {
			if (sdb != null) {
				sdb.disconnect();
			}
		}
	}

}
