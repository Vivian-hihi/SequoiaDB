package com.sequoiadb.split;

import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Date;
import java.util.List;

import org.bson.BSONObject;
import org.bson.util.JSON;
import org.testng.Assert;
import org.testng.SkipException;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import com.sequoiadb.base.CollectionSpace;
import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.DBCursor;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.testcommon.CommLib;
import com.sequoiadb.testcommon.SdbTestBase;
import com.sequoiadb.testcommon.SdbThreadBase;

/**
 * @FileName:SEQDB-10534 切分过程中创建索引 1、向cl中插入数据记录 2、执行split，设置切分条件
 *                       3、切分过程中创建索引，分别验证如下几个场景： a、迁移数据过程中 b、清除数据过程中 创建索引覆盖如下条件：
 *                       a、索引最大数（64个索引） b、索引包含正序、逆序 4、查看切分和创建索引结果
 *                       5、带索引查询切分数据（覆盖查询切分范围边界值数据）
 * @author huangqiaohui
 * @version 1.00
 *
 */

public class Split10534 extends SdbTestBase {
	private String clName = "testcaseCL10534";
	private String srcGroupName;
	private String destGroupName;
	private Sequoiadb commSdb = null;
	private List<BSONObject> insertedData = new ArrayList<>();
	private List<BSONObject> indexes = new ArrayList<>();

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

			CollectionSpace customCS = commSdb.getCollectionSpace(csName);
			DBCollection cl = customCS.createCollection(clName, (BSONObject) JSON
					.parse("{ShardingKey:{'sk':1},ShardingType:'range',Group:'" + srcGroupName + "'}"));
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
			for (int i = 0; i < 500; i++) {
				BSONObject obj = (BSONObject) JSON.parse("{sk:" + i + ",index1:" + i + "}");
				cl.insert(obj);
				insertedData.add(obj);
			}
		} catch (BaseException e) {
			throw e;
		}
	}

	@Test
	public void createIndex() {
		Sequoiadb db = null;
		Split splitThread = new Split();
		splitThread.start();
		try {
			db = new Sequoiadb(coordUrl, "", "");
			DBCollection cl = db.getCollectionSpace(csName).getCollection(clName);
			// 建立62个普通索引
			for (int i = 0; i < 62; i++) {
				String indexName = "index" + i;
				int oder = i % 2 == 0 ? 1 : -1;
				cl.createIndex(indexName, "{" + indexName + ":" + oder + "}", false, false);
				BSONObject indexObj = (BSONObject) JSON.parse("{name:'" + indexName + "',key: {" + indexName + ": "
						+ oder + "},v: 0,unique: false,dropDups: false,enforced: false}");
				indexes.add(indexObj);
			}
			// 索引集合加入默认的id索引和shard索引
			BSONObject idIndex = (BSONObject) JSON
					.parse("{name: \"$id\",key: {_id: 1},v: 0,unique: true,dropDups: false,enforced: true}");
			BSONObject shardingKeyIndex = (BSONObject) JSON
					.parse("{name: \"$shard\",key: {sk: 1},v: 0,unique: false,dropDups: false,enforced: false}");
			indexes.add(idIndex);
			indexes.add(shardingKeyIndex);

			// 等待切分结束
			if (!splitThread.isSuccess()) {
				Assert.fail(splitThread.getErrorMsg());
			}
			
			// 期望有250条符合{sk:{$gte:0,$lt:250}}的记录，并且源组中只有250条记录
			checkDestGroup(db, 250, "{sk:{$gte:0,$lt:250}}", 250, srcGroupName);
			// 期望有250条符合条件的记录，并且目标组中只有250条记录
			checkDestGroup(db, 250, "{sk:{$gte:250,$lt:500}}", 250, destGroupName);

			// 检查源和目标组的索引
			checkIndexExist(db, srcGroupName);
			checkIndexExist(db, destGroupName);

			// 指定索引信息查询数据（在cl中匹配{index1:34}，期望结果{sk:34,index1:34}，且为ixscan）
			queryByIndexAndCheckExplain(cl, "{index1:34}", "{sk:34,index1:34}", "ixscan");
		} catch (BaseException e) {
			Assert.fail(e.getMessage());
		} finally {
			if (db != null) {
				db.disconnect();
			}
		}
	}

	@AfterClass(enabled = true)
	public void tearDown() {
		try {
			CollectionSpace cs = commSdb.getCollectionSpace(csName);
			cs.dropCollection(clName);
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

	// 按macher查询，结果与expectedRecord比对，检查访问计划的扫描方式是否为expectScanType
	private void queryByIndexAndCheckExplain(DBCollection cl, String macher, String expectedRecord,
			String expectScanType) {
		DBCursor dbc1 = null;
		DBCursor dbc2 = null;
		BSONObject expected = (BSONObject) JSON.parse(expectedRecord);
		try {
			// 查询，检查结果的正确性
			dbc1 = cl.query(macher, null, null, null);
			ArrayList<BSONObject> queryReaults = new ArrayList<>();
			while (dbc1.hasNext()) {
				queryReaults.add(dbc1.getNext());
			}
			if (queryReaults.size() == 1) {
				BSONObject expect = (BSONObject) queryReaults.get(0);
				expect.removeField("_id");
				Assert.assertEquals(expected.equals(expect), true,
						"expected:" + expected.toString() + " actual:" + expect);
			} else {
				Assert.fail("query resault not correct,the array:" + queryReaults);
			}

			// 检查扫描方式
			dbc2 = cl.explain((BSONObject) JSON.parse(macher), null, null, null, 0, -1, 0, null);
			if (dbc2.hasNext()) {
				String scanType = (String) dbc2.getNext().get("ScanType");
				Assert.assertEquals(scanType.equals(expectScanType), true, "scanType not " + expectScanType);
			} else {
				Assert.fail("mainCL explain wrong");
			}
		} catch (BaseException e) {
			throw e;
		} finally {
			if (dbc1 != null) {
				dbc1.close();
			}
			if (dbc2 != null) {
				dbc2.close();
			}
		}
	}

	// 检查是否存在test方法中设置的索引
	public void checkIndexExist(Sequoiadb db, String groupName) {
		DBCursor dbc = null;
		List<BSONObject> indexesCopy = new ArrayList<>(indexes);
		Sequoiadb dataNode = null;
		try {
			dataNode = db.getReplicaGroup(groupName).getMaster().connect();// 获得目标组主节点链接
			DBCollection cl = dataNode.getCollectionSpace(csName).getCollection(clName);
			dbc = cl.getIndexes();
			while (dbc.hasNext()) {
				BSONObject actual = (BSONObject) dbc.getNext().get("IndexDef");
				actual.removeField("_id");
				if (indexesCopy.contains(actual)) {
					indexesCopy.remove(actual);
				} else {
					Assert.fail("should not have this index:" + actual);
				}
			}
			Assert.assertEquals(indexesCopy.size() == 0, true, "miss some indexes:" + indexesCopy);
		} catch (BaseException e) {
			throw e;
		} finally {
			if (dbc != null) {
				dbc.close();
			}
			if (dataNode != null) {
				dataNode.disconnect();
			}
		}
	}

	private void checkDestGroup(Sequoiadb db, int expectedCount, String macher, int expectTotalCount,
			String groupName) {
		Sequoiadb destDataNode = null;
		try {
			destDataNode = db.getReplicaGroup(groupName).getMaster().connect();// 获得目标组主节点链接
			DBCollection destCL = destDataNode.getCollectionSpace(csName).getCollection(clName);
			long count = destCL.getCount(macher);
			Assert.assertEquals(count, expectedCount);// 目标组应当含有上述查询数据
			Assert.assertEquals(destCL.getCount(), expectTotalCount); // 目标组应当含有的数据量
		} catch (BaseException e) {
			e.printStackTrace();
			Assert.fail(e.getMessage());
		} finally {
			if (destDataNode != null) {
				destDataNode.disconnect();
			}
		}
	}

	class Split extends SdbThreadBase {

		@Override
		public void exec() throws Exception {
			Sequoiadb sdb = null;
			try {
				sdb = new Sequoiadb(coordUrl, "", "");
				CollectionSpace cs = sdb.getCollectionSpace(csName);
				DBCollection cl = cs.getCollection(clName);
		
				cl.split(srcGroupName, destGroupName, 50);
			} catch (BaseException e) {
				throw e;
			} finally {
				if (sdb != null) {
					sdb.disconnect();
				}
			}
		}
	}

}
