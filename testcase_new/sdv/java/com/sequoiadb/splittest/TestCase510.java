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

import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.exception.BaseException;

import com.sequoiadb.testcommon.SdbTestBase;
import com.sequoiadb.util.MySdbTools;

/**
 * @FileName:SEQDB-510 1.在CS下创建cl，指定分区方式为range 2、向cl中插入大量数据，如插入1百万条记录
 *                     3、并发执行多个split，其中切分范围不相同，如一个切分范围为（0,10】，另一个切分范围为（80,100]
 *                     4、查看数据切分是否正确
 * @author huangqiaohui
 * @version 1.00
 *
 */

public class TestCase510 extends SdbTestBase {
	private String clName = "testcaseCL510";
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
			tearDown();
			Assert.fail("TestCase510 setUp error, error description:" + e.getMessage());
		} finally {
			if (sdb != null)
				sdb.disconnect();
		}
	}

	// 写入待切分的记录（1000）
	@Test
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
	@Test(enabled = true, dependsOnMethods = "beforSplitInsertData", groups = "split_510")
	public void splitCL1() {

		Sequoiadb sdb = null;
		try {
			sdb = new Sequoiadb(coordUrl, "", "");
			DBCollection cl = sdb.getCollectionSpace(csName).getCollection(clName);
			cl.split(srcGroupName, destGroupName, (BSONObject) JSON.parse("{a:0}"),
					(BSONObject) JSON.parse("{a:100}"));
		} catch (BaseException e) {
			Assert.fail(e.getMessage());
		} finally {
			if (sdb != null)
				sdb.disconnect();
		}


	}

	// 切分(200,300)
	@Test(enabled = true, dependsOnMethods = "beforSplitInsertData", groups = "split_510")
	public void splitCL2() {
		Sequoiadb sdb = null;
		try {
			sdb = new Sequoiadb(coordUrl, "", "");
			DBCollection cl = sdb.getCollectionSpace(csName).getCollection(clName);
			cl.split(srcGroupName, destGroupName, (BSONObject) JSON.parse("{a:200}"),
					(BSONObject) JSON.parse("{a:300}"));
		} catch (BaseException e) {
			Assert.fail(e.getMessage());
		} finally {
			if (sdb != null)
				sdb.disconnect();
		}


	}

	// 切分(500,600)
	@Test(enabled = true, dependsOnMethods = "beforSplitInsertData", groups = "split_510")
	public void splitCL3() {
		Sequoiadb sdb = null;
		try {
			sdb = new Sequoiadb(coordUrl, "", "");
			DBCollection cl = sdb.getCollectionSpace(csName).getCollection(clName);
			cl.split(srcGroupName, destGroupName, (BSONObject) JSON.parse("{a:500}"),
					(BSONObject) JSON.parse("{a:600}"));
		} catch (BaseException e) {
			Assert.fail(e.getMessage());
		} finally {
			if (sdb != null)
				sdb.disconnect();
		}

	}

	// 检查切分后结果，尝试写入数据
	@Test(enabled = true, dependsOnGroups = "split_510")
	public void checkReault() {
		Sequoiadb sdb = null;
		try {
			sdb = new Sequoiadb(coordUrl, "", "");
			int port = sdb.getReplicaGroup(destGroupName).getMaster().getPort();
			String destHostName = sdb.getReplicaGroup(destGroupName).getMaster().getHostName();
			Sequoiadb dataNode = new Sequoiadb(destHostName + ":" + port, "", "");
			DBCollection cl = dataNode.getCollectionSpace(csName).getCollection(clName);
			long count1 = cl.getCount("{a:{$gte:500,$lt:600}}");
			long count2 = cl.getCount("{a:{$gte:200,$lt:300}}");
			long count3 = cl.getCount("{a:{$gte:0,$lt:100}}");
			
			Assert.assertEquals(count1+count2+count3, 300);//目标组应当含有上述范围数据
			Assert.assertEquals(cl.getCount(), 300);//目标组仅含上述数据
		} catch (BaseException e) {
			Assert.fail(e.getMessage());
		}
		try {
			DBCollection cl = sdb.getCollectionSpace(csName).getCollection(clName);
			cl.insert((BSONObject) JSON.parse("{a:-2000}"));
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
