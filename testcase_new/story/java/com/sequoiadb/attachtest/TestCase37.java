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
 * @TestName:SEQDB-37 1.创建主表，子表1,子表2； 2.attach子表1和子表2时的区间有重叠；
 * @author huangqiaohui
 * @version 1.00
 *
 */

public class TestCase37 extends SdbTestBase {
	private Sequoiadb sdb;
	private CollectionSpace commCS;
	private DBCollection mainCL;
	private DBCollection subCL_1;
	private DBCollection subCL_2;
	private String mainCLName = "mainCL37";
	private String subCLName_1 = "subCL37_1";
	private String subCLName_2 = "subCL37_2";

	@BeforeClass
	public void setUp() {
		try {
			System.out.println("the TestCase Name:" + this.getClass().getName() + ". the TestCase begin at:"
					+ new SimpleDateFormat("YYYY-MM-dd HH:mm:ss.SSS").format(new Date()));
			sdb = new Sequoiadb(coordUrl, "", "");
			commCS = sdb.getCollectionSpace(csName);
			mainCL = MySdbTools.createCL(mainCLName, commCS, "{IsMainCL:true,ShardingKey:{\"alph\":1}}");
			subCL_1 = MySdbTools.createCL(subCLName_1, commCS, "{ShardingKey:{\"tx_id\":1},ShardingType:\"hash\"}");
			subCL_2 = MySdbTools.createCL(subCLName_2, commCS, "{ShardingKey:{\"tx_id\":1},ShardingType:\"hash\"}");
		} catch (Exception e) {
			tearDown();
			Assert.fail("TestCase37 setUp error, error description:" + e.getMessage());
		}
	}

	@Test
	public void test() {
		try {
			this.mainCL.attachCollection(this.subCL_1.getFullName(),
					(BSONObject) JSON.parse("{LowBound:{\"alph\":10},UpBound:{\"alph\":100}}"));
		} catch (BaseException e) {
			Assert.fail(e.getMessage());
		}
		try {
			this.mainCL.attachCollection(this.subCL_2.getFullName(),
					(BSONObject) JSON.parse("{LowBound:{\"alph\":20},UpBound:{\"alph\":110}}"));
		} catch (BaseException e) {
			Assert.assertEquals(e.getErrorCode(), -237, e.getMessage());
			return;
		}
		Assert.fail(
				"TestCase37 dose not pass, mainCL attach subCL_1{LowBound:{\"alph\":10},UpBound:{\"alph\":100}} and "
						+ "subCL_2{LowBound:{\"alph\":10},UpBound:{\"alph\":100}} success");

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
				if (commCS.isCollectionExist(subCLName_2)) {
					commCS.dropCollection(subCLName_2);
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
