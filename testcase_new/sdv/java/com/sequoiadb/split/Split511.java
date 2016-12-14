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
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.testcommon.CommLib;
import com.sequoiadb.testcommon.SdbTestBase;
import com.sequoiadb.util.MySdbTools;

/**
 * @FileName:SEQDB-511 1.在cl下指定分区键进行数据切分 2、切分过程中向cl中插入大量数据，如插入1百万条记录 3、查看数据切分结果
 *                     4、再次插入数据，查看写数据情况
 * @author huangqiaohui
 * @version 1.00
 *
 */

public class Split511 extends SdbTestBase {
	private String clName = "testcaseCL511";
	private String srcGroupName;
	private String destGroupName;
	private AtomicInteger a = new AtomicInteger();
	private boolean isStandAlone;
	private boolean isGroupTooless;
	
	@BeforeTest(enabled = true)
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
			Assert.fail("Split511 setUp error, error description:" + e.getMessage());
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
	@Test(enabled = true, dependsOnMethods = "beforSplitInsertData")
	public void getGroupName() {
		if (isStandAlone || isGroupTooless) {
			return;
		}
		Sequoiadb db = null;
		a.set(0);
		try {
			db = new Sequoiadb(coordUrl, "", "");
			ArrayList<String> tmp = MySdbTools.getGroupName(db, csName, clName);
			srcGroupName = tmp.get(0);
			destGroupName = tmp.get(1);
		} catch (Exception e) {
			Assert.fail(e.getMessage());
		} finally {
			if (db != null) {
				db.disconnect();
			}
		}
	}

	// 切分过程中写入记录(1000)
	@Test(dependsOnMethods = "getGroupName", groups = "splitAndInsert_511")
	public void SplittingInsertData() {
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

	// 切分
	@Test(enabled = true, dependsOnMethods = "getGroupName", groups = "splitAndInsert_511")
	public void splitCL() {
		if (isStandAlone || isGroupTooless) {
			return;
		}
		Sequoiadb sdb = null;
		try {
			sdb = new Sequoiadb(coordUrl, "", "");
			DBCollection cl = sdb.getCollectionSpace(csName).getCollection(clName);
			cl.split(srcGroupName, destGroupName, (BSONObject) JSON.parse("{a:0}"), (BSONObject) JSON.parse("{a:500}"));

		} catch (BaseException e) {
			Assert.fail(e.getMessage());
		} finally {
			if (sdb != null)
				sdb.disconnect();
		}

	}

	// 检查切分后结果，尝试写入数据
	@Test(enabled = true, dependsOnGroups = "splitAndInsert_511", timeOut = 120000)
	public void checkReault() {
		if (isStandAlone || isGroupTooless) {
			return;
		}
		Sequoiadb sdb = null;
		try {
			sdb = new Sequoiadb(coordUrl, "", "");
			int port = sdb.getReplicaGroup(destGroupName).getMaster().getPort();
			String destHostName = sdb.getReplicaGroup(destGroupName).getMaster().getHostName();
			Sequoiadb dataNode = new Sequoiadb(destHostName + ":" + port, "", "");
			DBCollection destCL = dataNode.getCollectionSpace(csName).getCollection(clName);
			long count = destCL.getCount("{a:{$gte:0,$lt:500}}");
			Assert.assertEquals(count, 1000);// 目标组数据应当含有上述查询数据
			Assert.assertEquals(destCL.getCount(), 1000);

			// 插入数据
			DBCollection cl = sdb.getCollectionSpace(csName).getCollection(clName);
			cl.insert((BSONObject) JSON.parse("{a:-2000,b:1}"));// 期望此数据落入源数据组
			cl.insert((BSONObject) JSON.parse("{a:200,b:1}"));// 期望此数据落入目标组

			// 检查落入情况
			long count1 = dataNode.getCollectionSpace(csName).getCollection(clName).getCount("{a:200,b:1}");
			Assert.assertEquals(count1, 1);// 新插入的数据正确落入目标组

			int srcport = sdb.getReplicaGroup(srcGroupName).getMaster().getPort();
			String srcHostName = sdb.getReplicaGroup(srcGroupName).getMaster().getHostName();
			Sequoiadb srcdataNode = new Sequoiadb(srcHostName + ":" + srcport, "", "");
			long count2 = srcdataNode.getCollectionSpace(csName).getCollection(clName).getCount("{a:-2000,b:1}");
			Assert.assertEquals(count2, 1);// 新插入的数据正确落入源数据组

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
