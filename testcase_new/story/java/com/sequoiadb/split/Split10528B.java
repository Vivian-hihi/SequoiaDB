package com.sequoiadb.split;

import java.text.SimpleDateFormat;

import java.util.Date;
import java.util.List;
import java.util.concurrent.atomic.AtomicBoolean;

import org.bson.BSONObject;
import org.bson.util.JSON;
import org.testng.Assert;
import org.testng.SkipException;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import com.sequoiadb.base.CollectionSpace;
import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.testcommon.CommLib;
import com.sequoiadb.testcommon.SdbTestBase;
import com.sequoiadb.testcommon.SdbThreadBase;

/**
 * @FileName:SEQDB-10528 切分过程中删除CL :1、向cl中循环插入数据记录 2、执行split，设置范围切分条件
 *                       3、切分过程中执行删除CL操作，分别在如下阶段删除CL:
 *                       a、任务已下发还未开始执行（如执行split后，通过listTasks查看无任务，在此过程中删除cl）
 *                       b、迁移数据过程中（如直连目标组节点查看数据持续插入，可count查询数据量在增加，删除cl）
 *                       c、目标组更新编目信息后删除cs（如直连目标组查看数据已迁移完成，或者直连编目节点查看cl信息中存在目标组，
 *                       删除cl） 4、查看切分和删除cl操作结果 备注此用例验证B
 * @author huangqiaohui
 * @version 1.00
 *
 */

public class Split10528B extends SdbTestBase {
	private String clName = "testcaseCL_10528B";
	private String srcGroupName;
	private String destGroupName;
	private Sequoiadb commSdb = null;
	private AtomicBoolean flag = new AtomicBoolean(false);

	@BeforeClass()
	public void setUp() {

		try {
			System.out.println("the TestCase Name:" + this.getClass().getName() + ". the TestCase begin at:"
					+ new SimpleDateFormat("YYYY-MM-dd HH:mm:ss.SSS").format(new Date()));
			commSdb = new Sequoiadb(coordUrl, "", "");

			// 跳过 standAlone 和数据组不足的环境
			CommLib commlib = new CommLib();
			if (commlib.isStandAlone(commSdb)) {
				throw new SkipException("skip StandAlone");
			}
			List<String> groupsName = commlib.getDataGroupNames(commSdb);
			if (groupsName.size() < 2) {
				throw new SkipException("current environment less than tow groups ");
			}
			srcGroupName = groupsName.get(0);
			destGroupName = groupsName.get(1);
			CollectionSpace cs = commSdb.getCollectionSpace(csName);
			DBCollection cl = cs.createCollection(clName,
					(BSONObject) JSON
							.parse("{ShardingKey:{'sk':1},ReplSize:3,Partition:4096,ShardingType:'hash',Group:'"
									+ srcGroupName + "'}"));
			insertData(cl);// 写入待切分的记录（1000）
		} catch (BaseException e) {
			if (commSdb != null) {
				commSdb.disconnect();
			}
			Assert.fail(this.getClass().getName() + " setUp error, error description:" + e.getMessage());
		}
	}

	public void insertData(DBCollection cl) {
		try {
			for (int i = 0; i < 1000; i++) {
				BSONObject obj = (BSONObject) JSON.parse("{sk:" + i + "}");
				cl.insert(obj);
			}
		} catch (BaseException e) {
			throw e;
		}

	}

	@Test
	public void dropCL() {
		Sequoiadb db = null;
		Sequoiadb dataNode = null;
		Split splitThread = new Split();
		splitThread.start();
		try {
			db = new Sequoiadb(coordUrl, "", "");

			dataNode = db.getReplicaGroup(destGroupName).getMaster().connect();// 获得目标组主节点链接

			while (dataNode.isCollectionSpaceExist(csName) != true && flag.get() == false) {
				// Thread.sleep(500);
			}
			CollectionSpace cs = dataNode.getCollectionSpace(csName);
			while (cs.isCollectionExist(clName) != true && flag.get() == false) {
				// Thread.sleep(500);
			}
			DBCollection cl = dataNode.getCollectionSpace(csName).getCollection(clName);
			while (cl.getCount() == 0 && flag.get() == false) {
				// Thread.sleep(500);
			}
			db.getCollectionSpace(csName).dropCollection(clName);
			if (!splitThread.isSuccess()) {
				Assert.fail(splitThread.getErrorMsg());
			}
		} catch (BaseException e) {
			Assert.fail(e.getMessage());
		} finally {
			if (db != null) {
				db.disconnect();
			}
			splitThread.join();
		}
	}

	@AfterClass(enabled = true)
	public void tearDown() {
		try {
			if (commSdb.getCollectionSpace(csName).isCollectionExist(clName)) {
				commSdb.getCollectionSpace(csName).dropCollection(clName);
			}
		} catch (BaseException e) {
			Assert.fail(e.getMessage());
		} finally {
			if (commSdb != null) {
				commSdb.disconnect();
			}
			System.out.println("the TestCase Name:" + this.getClass().getName() + ". the TestCase end at:"
					+ new SimpleDateFormat("YYYY-MM-dd HH:mm:ss.SSS").format(new Date()));
		}
	}

	class Split extends SdbThreadBase {

		@Override
		public void exec() throws Exception {
			Sequoiadb sdb = null;
			try {
				sdb = new Sequoiadb(coordUrl, "", "");
				DBCollection cl = sdb.getCollectionSpace(csName).getCollection(clName);
				cl.split(srcGroupName, destGroupName, 90);
			} catch (BaseException e) {
				throw e;
			} finally {
				if (sdb != null) {
					sdb.disconnect();
				}
				flag.set(true);
			}
		}
	}

}
