package com.sequoiadb.splittest;

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
import com.sequoiadb.testcommon.SdbTestBase;
import com.sequoiadb.util.MySdbTools;

/**
 * @FileName:SEQDB-514 1.在cl下指定分区键进行数据切分 2、切分过程中执行findandremove组合skip/limit
 *                     3、查看数据切分结果 4、再次插入数据，查看写数据情况
 * @author huangqiaohui
 * @version 1.00
 *
 */

public class TestCase514 extends SdbTestBase {
	private String clName = "testcaseCL514";
	private String srcGroupName;
	private String destGroupName;
	private AtomicInteger a = new AtomicInteger();

	@BeforeTest(enabled = true)
	public void setUp() {
		Sequoiadb sdb = null;
		try {
			System.out.println("the TestCase Name:" + this.getClass().getName() + ". the TestCase begin at:"
					+ new SimpleDateFormat("YYYY-MM-dd HH:mm:ss.SSS").format(new Date()));
			sdb = new Sequoiadb(coordUrl, "", "");
			CollectionSpace commCS = sdb.getCollectionSpace(csName);
			MySdbTools.createCL(clName, commCS, "{ShardingKey:{\"a\":1},ShardingType:\"range\"}");
			ArrayList<String> tmp = MySdbTools.getGroupName(sdb, csName, clName);
			srcGroupName = tmp.get(0);
			destGroupName = tmp.get(1);
		} catch (Exception e) {
			tearDown();// 清理可能创建的CL
			Assert.fail("TestCase514 setUp error, error description:" + e.getMessage());
		} finally {
			if (sdb != null) {
				sdb.disconnect();
			}
		}
	}

	// 写入待切分的记录（1000）
	@Test(enabled = true)
	public void beforSplitInsertData() {
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
	@Test(enabled = true, dependsOnMethods = "beforSplitInsertData", groups = "splitAndFindRemove_514")
	public void splitCL() {
		Sequoiadb sdb = null;
		try {
			sdb = new Sequoiadb(coordUrl, "", "");
			DBCollection cl = sdb.getCollectionSpace(csName).getCollection(clName);
			cl.split(srcGroupName, destGroupName, (BSONObject) JSON.parse("{a:0}"), (BSONObject) JSON.parse("{a:100}"));
		} catch (BaseException e) {
			Assert.fail(e.getMessage());
		} finally {
			if (sdb != null) {
				sdb.disconnect();
			}
		}
	}

	// 切分同时查询删除
	@Test(enabled = true, dependsOnMethods = "beforSplitInsertData",timeOut=60000,groups = "splitAndFindRemove_514")
	public void findAndRemove() {
		Sequoiadb sdb = null;
		DBCursor dbc = null;
		try {
			sdb = new Sequoiadb(coordUrl, "", "");
			DBCollection cl = sdb.getCollectionSpace(csName).getCollection(clName);
			
			//删除a:0 - a:50 的记录
			dbc = cl.query("{a:{$gte:0,$lt:50}}", null, null, null, 0, 50);
			while (dbc.hasNext()) {
				cl.delete(dbc.getNext());
			}
		} catch (BaseException e) {
			Assert.fail(e.getMessage());
		} finally {
			if (dbc != null) {
				dbc.close();
			}
			if (sdb != null) {
				sdb.disconnect();
			}
		}

	}

	// 检查切分后结果，尝试写入数据
	@Test(enabled = true, dependsOnGroups = "splitAndFindRemove_514")
	public void checkReault() {
		Sequoiadb sdb = null;
		Sequoiadb destDataNode = null;
		long destDataCount = 0;
		try {
			sdb = new Sequoiadb(coordUrl, "", "");
			int destPort = sdb.getReplicaGroup(destGroupName).getMaster().getPort();
			String destHostName = sdb.getReplicaGroup(destGroupName).getMaster().getHostName();
			destDataNode = new Sequoiadb(destHostName + ":" + destPort, "", "");
			destDataCount = destDataNode.getCollectionSpace(csName).getCollection(clName)
					.getCount("{a:{$gte:50,$lt:100}}");
			Assert.assertEquals(destDataCount, 50);// 目标组应当含有上述范围数据
			Assert.assertEquals(destDataNode.getCollectionSpace(csName).getCollection(clName).getCount(), 50);// 目标组应当仅含有上述范围数据
		} catch (BaseException e) {
			Assert.fail(e.getMessage());
		}

		// 插入数据，检查落入情况
		try {
			DBCollection cl = sdb.getCollectionSpace(csName).getCollection(clName);
			cl.insert((BSONObject) JSON.parse("{a:10,b:-1}"));// 期望此数据落入目标组
			cl.insert((BSONObject) JSON.parse("{a:500,b:-2}"));// 期望此数据落入源数据组
			long destDataCount_1 = destDataNode.getCollectionSpace(csName).getCollection(clName)
					.getCount("{a:10,b:-1}");
			Assert.assertEquals(destDataCount_1, 1);//检查是否落入目标组
			
			int srcport = sdb.getReplicaGroup(srcGroupName).getMaster().getPort();
			String srcHostName = sdb.getReplicaGroup(srcGroupName).getMaster().getHostName();
			Sequoiadb srcdataNode = new Sequoiadb(srcHostName + ":" + srcport, "", "");
			long count2 = srcdataNode.getCollectionSpace(csName).getCollection(clName).getCount("{a:500,b:-2}");
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
