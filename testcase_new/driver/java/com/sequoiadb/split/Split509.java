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
 * @FileName:SEQDB-509 1.在CS下创建cl，指定分区方式为range 2、向cl中插入大量数据，如插入1百万条记录
 *                     3、并发执行多个split，其中切分范围存在交集部分，如一个切分范围为（10,30】，另一个切分范围为（20,
 *                     100] 4、查看数据切分是否正确
 * @author huangqiaohui
 * @version 1.00
 *
 */

public class Split509 extends SdbTestBase {
	private String clName = "testcaseCL509";
	private String srcGroupName;
	private String destGroupName;
	private AtomicInteger a = new AtomicInteger();
	private ArrayList<Integer> successRange = new ArrayList<>();
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
			srcGroupName = tmp.get(0);
			destGroupName = tmp.get(1);
		} catch (Exception e) {
			tearDown();
			Assert.fail("Split509 setUp error, error description:" + e.getMessage());
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

	// 切分(0,100)
	@Test(enabled = true, dependsOnMethods = "beforSplitInsertData", groups = "split_509")
	public void splitCL1() {
		if (isStandAlone || isGroupTooless) {
			return;
		}
		Sequoiadb sdb = null;
		try {
			sdb = new Sequoiadb(coordUrl, "", "");
			DBCollection cl = sdb.getCollectionSpace(csName).getCollection(clName);
			cl.split(srcGroupName, destGroupName, (BSONObject) JSON.parse("{a:0}"), (BSONObject) JSON.parse("{a:100}"));
		} catch (BaseException e) {
			Assert.assertEquals(e.getErrorCode(), -175);
			return;
		} finally {
			if (sdb != null) {
				sdb.disconnect();
			}
		}
		successRange.add(0);
		successRange.add(100);

	}

	// 切分(50,150),重叠上一个切分
	@Test(enabled = true, dependsOnMethods = "beforSplitInsertData", groups = "split_509")
	public void splitCL2() {
		if (isStandAlone || isGroupTooless) {
			return;
		}
		Sequoiadb sdb = null;
		try {
			sdb = new Sequoiadb(coordUrl, "", "");
			DBCollection cl = sdb.getCollectionSpace(csName).getCollection(clName);
			cl.split(srcGroupName, destGroupName, (BSONObject) JSON.parse("{a:50}"),
					(BSONObject) JSON.parse("{a:150}"));
		} catch (BaseException e) {
			Assert.assertEquals(e.getErrorCode(), -175);
			return;
		} finally {
			if (sdb != null) {
				sdb.disconnect();
			}
		}
		successRange.add(50);
		successRange.add(150);
	}

	// 检查切分后结果，尝试写入数据
	@Test(enabled = true, dependsOnGroups = "split_509")
	public void checkReault() {
		if (isStandAlone || isGroupTooless) {
			return;
		}
		if (successRange.size() != 2) {
			Assert.fail("parallel execute split range(0,100) and range(50,150),successRange.size() = "
					+ successRange.size());
		}
		Sequoiadb sdb = null;
		Sequoiadb destDataNode = null;
		long destDataCount;
		long destDataCount_1;
		// 检查切分结果
		try {
			sdb = new Sequoiadb(coordUrl, "", "");
			int destPort = sdb.getReplicaGroup(destGroupName).getMaster().getPort();
			String destHostName = sdb.getReplicaGroup(destGroupName).getMaster().getHostName();
			destDataNode = new Sequoiadb(destHostName + ":" + destPort, "", "");

			destDataCount = destDataNode.getCollectionSpace(csName).getCollection(clName)
					.getCount("{$and:[{a:{$gte:" + successRange.get(0) + "}},{a:{$lt:" + successRange.get(1) + "}}]}");
			Assert.assertEquals(destDataCount, successRange.get(1) - successRange.get(0));// 检查指定范围数据是否均已落入目标组
			Assert.assertEquals(destDataNode.getCollectionSpace(csName).getCollection(clName).getCount(),
					successRange.get(1) - successRange.get(0));// 检查目标组是否仅含指定范围数据
		} catch (BaseException e) {
			Assert.fail(e.getMessage());
		}

		// 插入数据，检查落入情况
		try {
			DBCollection cl = sdb.getCollectionSpace(csName).getCollection(clName);
			cl.insert((BSONObject) JSON.parse("{b:1,a:" + successRange.get(0) + "}"));// 期望此数据落入目标组
			cl.insert((BSONObject) JSON.parse("{b:-1,a:" + (successRange.get(0) - 1) + "}"));// 期望此数据落入源数据组
			destDataCount_1 = destDataNode.getCollectionSpace(csName).getCollection(clName).getCount("{b:1}");
			Assert.assertEquals(destDataCount_1, 1);// 新插入的数据正确落入目标数据组

			int srcport = sdb.getReplicaGroup(srcGroupName).getMaster().getPort();
			String srcHostName = sdb.getReplicaGroup(srcGroupName).getMaster().getHostName();
			Sequoiadb srcdataNode = new Sequoiadb(srcHostName + ":" + srcport, "", "");
			long count2 = srcdataNode.getCollectionSpace(csName).getCollection(clName).getCount("{b:-1}");
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
