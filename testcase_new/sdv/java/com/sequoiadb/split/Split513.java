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
import com.sequoiadb.base.DBLob;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.testcommon.CommLib;
import com.sequoiadb.testcommon.SdbTestBase;
import com.sequoiadb.util.MySdbTools;

/**
 * @FileName:SEQDB-513 1.在cl下指定分区键进行数据切分
 *                     2、切分过程中向cl中插入大量数据，其中包括普通记录和lob对象，如插入1百万条记录 3、查看数据切分结果
 *                     4、再次插入数据，查看写数据情况
 * @author huangqiaohui
 * @version 1.00
 *
 */

public class Split513 extends SdbTestBase {
	private String clName = "testcaseCL513";
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
			MySdbTools.createCL(clName, commCS, "{ShardingKey:{\"a\":1},ShardingType:\"hash\"}");
			ArrayList<String> tmp = MySdbTools.getGroupName(sdb, csName, clName);
			if (tmp.size() != 2) {
				isGroupTooless = true;
				return;
			}
			srcGroupName = tmp.get(0);
			destGroupName = tmp.get(1);
		} catch (Exception e) {
			tearDown();// 清理可能创建的CL
			Assert.fail("TestCase513 setUp error, error description:" + e.getMessage());
		} finally {
			if (sdb != null) {
				sdb.disconnect();
			}
		}
	}

	// 写入待切分的记录（1000）
	@Test(enabled = true)
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

	// 切分50%
	@Test(enabled = true, dependsOnMethods = "beforSplitInsertData", groups = "splitAndInsert_513")
	public void splitCL() {
		if (isStandAlone || isGroupTooless) {
			return;
		}
		Sequoiadb sdb = null;
		try {
			sdb = new Sequoiadb(coordUrl, "", "");
			DBCollection cl = sdb.getCollectionSpace(csName).getCollection(clName);
			cl.split(srcGroupName, destGroupName, 50);
		} catch (BaseException e) {
			Assert.fail(e.getMessage());
		} finally {
			if (sdb != null) {
				sdb.disconnect();
			}
		}
	}

	// 切分时，插入LOB和普通记录
	@Test(enabled = true, dependsOnMethods = "beforSplitInsertData", groups = "splitAndInsert_513")
	public void insertLobAndDoc() {
		if (isStandAlone || isGroupTooless) {
			return;
		}
		Sequoiadb sdb = null;
		try {
			sdb = new Sequoiadb(coordUrl, "", "");
			DBCollection cl = sdb.getCollectionSpace(csName).getCollection(clName);
			for (int i = 0; i < 100; i++) {
				DBLob blob = cl.createLob();
				blob.write(clName.getBytes());
				blob.close();
			}
			for (int j = 0; j < 100; j++) {
				cl.insert(getBasicBSON());
			}
		} catch (BaseException e) {
			Assert.fail(e.getMessage());
		} finally {
			if (sdb != null) {
				sdb.disconnect();
			}
		}

	}

	// 检查切分后结果，尝试写入数据
	@Test(enabled = true, dependsOnGroups = "splitAndInsert_513")
	public void checkReault() {
		if (isStandAlone || isGroupTooless) {
			return;
		}
		Sequoiadb sdb = null;
		Sequoiadb destDataNode = null;
		long destDataCount = 0;
		try {
			sdb = new Sequoiadb(coordUrl, "", "");
			int destPort = sdb.getReplicaGroup(destGroupName).getMaster().getPort();
			String destHostName = sdb.getReplicaGroup(destGroupName).getMaster().getHostName();
			destDataNode = new Sequoiadb(destHostName + ":" + destPort, "", "");
			destDataCount = destDataNode.getCollectionSpace(csName).getCollection(clName).getCount();
			DBCursor dbc = destDataNode.getCollectionSpace(csName).getCollection(clName).listLobs();
			int destLobCount = 0;
			while (dbc.hasNext()) {
				destLobCount++;
				dbc.getNext();
			}
			dbc.close();
			if (destDataCount < 550 - (550 * 0.3) || destDataCount > 550 + (550 * 0.3)) {
				Assert.fail("split count unexpeted");// 对目标组的普通记录数量做校验
			}
			if (destLobCount < 50 - (50 * 0.3) || destLobCount > 50 + (50 * 0.3)) {
				Assert.fail("split lob count unexpeted");// 对目标组的lob记录数量做校验
			}
		} catch (BaseException e) {
			Assert.fail(e.getMessage());
		}

		// 插入数据，检查落入情况
		try {
			DBCollection cl = sdb.getCollectionSpace(csName).getCollection(clName);
			DBCursor dbc = destDataNode.getCollectionSpace(csName).getCollection(clName).query("", null, null, null, 0,
					-1);
			BSONObject bobj = null;
			if (dbc.hasNext()) {
				bobj = dbc.getNext();
			} else {
				dbc.close();
				Assert.fail("query error");
			}
			dbc.close();
			bobj.put("b", -10);
			bobj.removeField("_id");
			cl.insert(bobj);// 期望此数据落入目标组
			long destDataCount_1 = destDataNode.getCollectionSpace(csName).getCollection(clName).getCount(bobj);
			Assert.assertEquals(destDataCount_1, 1);// 检查是否落入目标组

			int srcport = sdb.getReplicaGroup(srcGroupName).getMaster().getPort();
			String srcHostName = sdb.getReplicaGroup(srcGroupName).getMaster().getHostName();
			Sequoiadb srcdataNode = new Sequoiadb(srcHostName + ":" + srcport, "", "");
			dbc = srcdataNode.getCollectionSpace(csName).getCollection(clName).query("", null, null, null, 0, -1);
			BSONObject bobj2 = null;
			if (dbc.hasNext()) {
				bobj2 = dbc.getNext();
			} else {
				dbc.close();
				Assert.fail("query error");
			}
			dbc.close();
			bobj2.put("b", -10);
			bobj2.removeField("_id");
			cl.insert(bobj2);// 期望此数据落入源数据组

			long count2 = srcdataNode.getCollectionSpace(csName).getCollection(clName).getCount(bobj2);
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
