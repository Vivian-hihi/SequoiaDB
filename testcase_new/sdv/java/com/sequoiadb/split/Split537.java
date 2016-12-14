package com.sequoiadb.split;

import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Date;

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
 * @FileName:SEQDB-536 1.在cl下指定百分比数进行数据切分 2、执行split操作，其中设置的源分区组中不存在数据
 *                     3、查看数据切分是否成功
 * @author huangqiaohui
 * @version 1.00
 *
 */
public class Split537 extends SdbTestBase {
	private Sequoiadb sdb;
	private CollectionSpace commCS;
	private DBCollection currentCL;
	private String clName = "testcaseCL537";
	private boolean isStandAlone;

	@BeforeTest(enabled = true)
	public void setUp() {
		try {
			System.out.println("the TestCase Name:" + this.getClass().getName() + ". the TestCase begin at:"
					+ new SimpleDateFormat("YYYY-MM-dd HH:mm:ss.SSS").format(new Date()));
			sdb = new Sequoiadb(coordUrl, "", "");
			CommLib commlib = new CommLib();
			isStandAlone = commlib.isStandAlone(sdb);
			if (isStandAlone) {
				return;
			}
			commCS = sdb.getCollectionSpace(csName);
			currentCL = MySdbTools.createCL(clName, commCS, "{ShardingKey:{\"a\":1},ShardingType:\"range\"}");
		} catch (Exception e) {
			tearDown();
			Assert.fail("Split537 setUp error, error description:" + e.getMessage());
		}
	}

	@Test(enabled = true)
	public void test() {
		if (isStandAlone) {
			return;
		}
		try {
			ArrayList<String> groups = MySdbTools.getGroupName(sdb, csName, clName);
			if (groups.size() < 2) {
				return;
			}
			currentCL.split(groups.get(0), groups.get(1), 50);
		} catch (BaseException e) {
			Assert.assertEquals(e.getErrorCode(), -296, e.getMessage());
			return;
		} catch (Exception e) {
			Assert.fail(e.getMessage());
		}
		Assert.fail("collection(ShardingType:range) is empty,split success");
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
