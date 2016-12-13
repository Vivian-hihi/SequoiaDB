package com.sequoiadb.attachtest;

import java.text.SimpleDateFormat;
import java.util.Date;

import org.bson.BSONObject;
import org.bson.util.JSON;
import org.testng.Assert;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import com.sequoiadb.base.CollectionSpace;
import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.testcommon.SdbTestBase;
import com.sequoiadb.util.MySdbTools;

/**
 * @FileName:SEQDB-43 1.创建主表，子表； 2.attach子表使用下限$minkey和上限$maxkey；[需要在信息中心查找一下]
 * @author huangqiaohui
 * @version 1.00
 *
 */

public class TestCase43 extends SdbTestBase {
	private Sequoiadb sdb;
	private CollectionSpace commCS;
	private DBCollection mainCL;
	private DBCollection subCL_1;
	private String mainCLName = "mainCL43";
	private String subCLName_1 = "subCL43_1";

	@BeforeClass
	public void setUp() {
		try {
			System.out.println("the TestCase Name:" + this.getClass().getName() + ". the TestCase begin at:"
					+ new SimpleDateFormat("YYYY-MM-dd HH:mm:ss.SSS").format(new Date()));
			sdb = new Sequoiadb(coordUrl, "", "");
			commCS = sdb.getCollectionSpace(csName);
			mainCL = MySdbTools.createCL(mainCLName, commCS, "{IsMainCL:true,ShardingKey:{\"alph\":1}}");
			subCL_1 = MySdbTools.createCL(subCLName_1, commCS, "{ShardingKey:{\"tx_id\":1},ShardingType:\"hash\"}");
		} catch (Exception e) {
			tearDown();
			Assert.fail("TestCase43 setUp error, error description:" + e.getMessage());
		}
	}

	@Test
	public void test() {
		try {
			this.mainCL.attachCollection(this.subCL_1.getFullName(),
					(BSONObject) JSON.parse("{LowBound:{\"alph\":{$minKey:1}},UpBound:{\"alph\":{$maxKey:1}}}"));
		} catch (BaseException e) {
			Assert.fail(e.getMessage());
		}
	}

	@AfterClass
	public void tearDown() {
		System.out.println("the TestCase Name:" + this.getClass().getName() + ". the TestCase end at:"
				+ new SimpleDateFormat("YYYY-MM-dd HH:mm:ss.SSS").format(new Date()));
		try {
			if (commCS != null) {
				if (commCS.isCollectionExist(mainCLName)) {
					commCS.dropCollection(mainCLName);
				}
				if (commCS.isCollectionExist(subCLName_1)) {
					commCS.dropCollection(subCLName_1);
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
