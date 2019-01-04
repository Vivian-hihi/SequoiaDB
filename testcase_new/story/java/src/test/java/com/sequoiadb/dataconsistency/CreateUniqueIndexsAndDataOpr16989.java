package com.sequoiadb.dataconsistency;

import java.util.ArrayList;

import org.bson.BSONObject;
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
 * @FileName CreateUniqueIndexsAndDataOpr16989.java
 * @content create unique Indexes, concurrent execution of insert and update
 *          operations, than check dataConsistency.
 * @testlink seqDB-16989
 * @author wuyan
 * @Date 2018.12.28
 * @version 1.00
 */
public class CreateUniqueIndexsAndDataOpr16989 extends SdbTestBase {

	private String clName = "dataConsistency16989";
	private Sequoiadb sdb = null;
	private String groupName = "";
	private CollectionSpace cs = null;
	private DBCollection dbcl = null;
	private int insertNums = 30000;
	private ArrayList<BSONObject> insertRecords = null;

	@BeforeClass
	public void setUp() {
		sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "");
		if (CommLib.isStandAlone(sdb)) {
			throw new SkipException("standAlone skip testcase");
		}

		groupName = DataConsistencyUtil.getGroupName(sdb);
		String options = "{ShardingKey:{no:1},ReplSize:1,Compressed:true," + "CompressionType:'lzw',Group:'" + groupName
				+ "'}";
		cs = sdb.getCollectionSpace(SdbTestBase.csName);
		dbcl = DataConsistencyUtil.createCL(cs, clName, options);

		dbcl.createIndex("testa", "{no:1}", true, false);
		insertRecords = DataConsistencyUtil.insertDatas(dbcl, insertNums);
	}

	@Test
	public void test() {
		InsertThread insertThread = new InsertThread();
		UpdateThread updateThread = new UpdateThread();
		insertThread.start();
		updateThread.start();
		Assert.assertTrue(insertThread.isSuccess(), insertThread.getErrorMsg());
		Assert.assertTrue(updateThread.isSuccess(), updateThread.getErrorMsg());

		updateExpDatas();
		DataConsistencyUtil.checkDataContent(dbcl, insertRecords);
		DataConsistencyUtil.checkDataConsistency(sdb, groupName, SdbTestBase.csName, clName, insertRecords);

	}

	@AfterClass
	public void tearDown() {
		try {
			if (cs.isCollectionExist(clName)) {
				cs.dropCollection(clName);
			}

		} finally {
			if (sdb != null)
				sdb.close();
		}
	}

	public class UpdateThread extends SdbThreadBase {
		@Override
		public void exec() throws BaseException {
			try (Sequoiadb db = new Sequoiadb(SdbTestBase.coordUrl, "", "")) {
				DBCollection dbcl = db.getCollectionSpace(SdbTestBase.csName).getCollection(clName);
				String modifier = "{ $set: { 'str': 'testdataconsistency16989' } }";
				dbcl.update("{no:{'$lt':" + insertNums + "}}", modifier, "");
			}
		}
	}

	private void updateExpDatas() {
		for (BSONObject object : insertRecords) {
			int value = (int) object.get("no");
			if (value < insertNums) {
				object.put("str", "testdataconsistency16989");
			} else {
				break;
			}
		}
	}

	public class InsertThread extends SdbThreadBase {
		@Override
		public void exec() throws Exception {

			try (Sequoiadb db = new Sequoiadb(SdbTestBase.coordUrl, "", "")) {
				DBCollection dbcl = db.getCollectionSpace(SdbTestBase.csName).getCollection(clName);
				// insert 3W records again from 3W.
				ArrayList<BSONObject> curInsertRecords = DataConsistencyUtil.insertDatas(dbcl, 30000, 30000);
				insertRecords.addAll(curInsertRecords);
			}
		}
	}

}
