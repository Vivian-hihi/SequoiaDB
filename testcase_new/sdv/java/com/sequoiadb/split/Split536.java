package com.sequoiadb.split;

import java.text.SimpleDateFormat;
import java.util.Date;
import org.bson.BSONObject;
import org.bson.types.BasicBSONList;
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
 * @FileName:SEQDB-536 1.在cl下指定范围条件进行数据切分 2、执行split操作，其中设置的目标分区组不存在 3、查看数据切分是否成功
 * @author huangqiaohui
 * @version 1.00
 *
 */

public class Split536 extends SdbTestBase {
	private Sequoiadb sdb;
	private CollectionSpace commCS;
	private DBCollection currentCL;
	private String clName = "testcaseCL536";
	private boolean isStandAlone;

	@BeforeTest(enabled = true)
	public void setUp() {
		try {
			System.out.println("the TestCase Name:" + this.getClass().getName() + ". the TestCase begin at:"
					+ new SimpleDateFormat("YYYY-MM-dd HH:mm:ss.SSS").format(new Date()));
			this.sdb = new Sequoiadb(coordUrl, "", "");
			CommLib commlib = new CommLib();
			isStandAlone = commlib.isStandAlone(sdb);
			if (isStandAlone) {
				return;
			}
			this.commCS = sdb.getCollectionSpace(csName);
			this.currentCL = MySdbTools.createCL(clName, commCS, "{ShardingKey:{\"a\":1}}");
		} catch (Exception e) {
			tearDown();
			Assert.fail("TestCase536 setUp error, error description:" + e.getMessage());
		}
	}

	@Test(enabled = true)
	public void test() {
		if (isStandAlone) {
			return;
		}
		DBCursor dbc = null;
		try {
			dbc = sdb.getSnapshot(Sequoiadb.SDB_SNAP_CATALOG, "{Name:\"" + currentCL.getFullName() + "\"}", null, null);
			BasicBSONList list = (BasicBSONList) dbc.getNext().get("CataInfo");
			String srcGroupName = (String) ((BSONObject) list.get(0)).get("GroupName");// 获取源数据组名
			String destGroupName = "thisDataGroupShouldNotExists_536";// 定义一个不存在组名
			Sequoiadb db = new Sequoiadb(coordUrl, "", "");
			DBCollection cl = db.getCollectionSpace(csName).getCollection(clName);
			for (int i = 0; i < 10; i++) {
				cl.insert((BSONObject) JSON.parse("{a:" + i + "}"));
			}
			cl.split(srcGroupName, destGroupName, (BSONObject) JSON.parse("{a:1}"), (BSONObject) JSON.parse("{a:5}"));
		} catch (BaseException e) {
			Assert.assertEquals(e.getErrorCode(), -154, e.getMessage());
			return;
		} finally {
			if (dbc != null) {
				dbc.close();
			}
		}
		Assert.fail();
	}

	@AfterTest(enabled = true)
	public void tearDown() {
		System.out.println("the TestCase Name:" + this.getClass().getName() + ". the TestCase end at:"
				+ new SimpleDateFormat("YYYY-MM-dd HH:mm:ss.SSS").format(new Date()));
		try {
			if (commCS != null) {
				if (commCS.isCollectionExist(clName))

					commCS.dropCollection(clName);
			}
		} catch (BaseException e) {
			// do something??
		}
		if (sdb != null) {
			sdb.disconnect();
		}
	}

}
