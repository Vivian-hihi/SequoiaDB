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
 * @FileName:SEQDB-508 1.在CS下创建cl，指定多个分区键，如设置ShardingKey：{"a"：1，"b":-1}
 *                     2、向cl中插入大量数据，如插入1千万条记录 3、并发执行多个split，设置不同的分区键值进行数据切分
 *                     4、查看数据切分是否正确
 * @author huangqiaohui
 * @version 1.00
 *
 */

public class Split508 extends SdbTestBase {
	private String clName = "testcaseCL508";
	private String srcGroupName;
	private String destGroupName;
	private AtomicInteger a = new AtomicInteger();
	private AtomicInteger b = new AtomicInteger();
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
			MySdbTools.createCL(clName, commCS, "{ShardingKey:{\"a\":1,\"b\":-1},ShardingType:\"range\"}");
			ArrayList<String> tmp = MySdbTools.getGroupName(sdb, csName, clName);
			if (tmp.size() != 2) {
				isGroupTooless = true;
				return;
			}
			srcGroupName = tmp.get(0);
			destGroupName = tmp.get(1);
		} catch (Exception e) {
			tearDown();
			Assert.fail("Split508 setUp error, error description:" + e.getMessage());
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

	// 切分
	@Test(enabled = true, dependsOnMethods = "beforSplitInsertData", groups = "split_508")
	public void splitCL1() {
		if (isStandAlone || isGroupTooless) {
			return;
		}
		Sequoiadb sdb = null;
		try {
			sdb = new Sequoiadb(coordUrl, "", "");
			DBCollection cl = sdb.getCollectionSpace(csName).getCollection(clName);
			cl.split(srcGroupName, destGroupName, (BSONObject) JSON.parse("{a:0,b:20}"),
					(BSONObject) JSON.parse("{a:20,b:0}"));
		} catch (BaseException e) {
			Assert.fail(e.getMessage());
			return;
		} finally {
			if (sdb != null) {
				sdb.disconnect();
			}
		}
	}

	// 切分
	@Test(enabled = true, dependsOnMethods = "beforSplitInsertData", groups = "split_508")
	public void splitCL2() {
		if (isStandAlone || isGroupTooless) {
			return;
		}
		Sequoiadb sdb = null;
		try {
			sdb = new Sequoiadb(coordUrl, "", "");
			DBCollection cl = sdb.getCollectionSpace(csName).getCollection(clName);
			cl.split(srcGroupName, destGroupName, (BSONObject) JSON.parse("{a:30,b:50}"),
					(BSONObject) JSON.parse("{a:50,b:30}"));
		} catch (BaseException e) {
			Assert.fail(e.getMessage());
		} finally {
			if (sdb != null) {
				sdb.disconnect();
			}
		}
	}

	// 检查切分后结果
	@Test(enabled = true, dependsOnGroups = "split_508")
	public void checkReault() {
		if (isStandAlone || isGroupTooless) {
			return;
		}
		Sequoiadb sdb = null;
		Sequoiadb destDataNode = null;
		try {
			sdb = new Sequoiadb(coordUrl, "", "");
			int destPort = sdb.getReplicaGroup(destGroupName).getMaster().getPort();
			String destHostName = sdb.getReplicaGroup(destGroupName).getMaster().getHostName();
			destDataNode = new Sequoiadb(destHostName + ":" + destPort, "", "");
			long destDataCount1 = destDataNode.getCollectionSpace(csName).getCollection(clName)
					.getCount("{a:{$gte:0,$lte:20}}");
			long destDataCount2 = destDataNode.getCollectionSpace(csName).getCollection(clName)
					.getCount("{a:{$gte:30,$lte:50}}");
			long destDataCount3 = destDataNode.getCollectionSpace(csName).getCollection(clName).getCount();

			Assert.assertEquals(destDataCount1 + destDataCount2, 42);// 目标组含有全部切分范围的数据
			Assert.assertEquals(destDataCount3, 42);// 目标组仅含有切分范围的数据
		} catch (BaseException e) {
			Assert.fail(e.getMessage());
		} finally {
			if (sdb != null) {
				sdb.disconnect();
			}
		}

	}

	public BSONObject getBasicBSON() {
		BasicBSONObject obj = null;
		obj = (BasicBSONObject) JSON.parse("{a:" + a.getAndIncrement() + ",b:" + b.getAndIncrement() + "}");
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
				if (commCS.isCollectionExist(clName)) {
					commCS.dropCollection(clName);
				}
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
