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
 * @TestName:SEQDB-39 1.创建主表； 2.attach一个未创建的表
 * @author huangqiaohui
 * @version 1.00
 *
 */

public class TestCase39 extends SdbTestBase {
	private Sequoiadb sdb;
	private CollectionSpace commCS;
	private DBCollection mainCL;
	private String mainCLName = "mainCL39";
	private String subCLName = "subCL39";

	@BeforeClass
	public void setUp() {
		try {
			System.out.println("the TestCase Name:" + this.getClass().getName() + ". the TestCase begin at:"
					+ new SimpleDateFormat("YYYY-MM-dd HH:mm:ss.SSS").format(new Date()));
			sdb = new Sequoiadb(coordUrl, "", "");
			commCS = sdb.getCollectionSpace(csName);
			mainCL = MySdbTools.createCL(mainCLName, commCS, "{IsMainCL:true,ShardingKey:{\"alph\":1}}");
		} catch (Exception e) {
			tearDown();
			Assert.fail("TestCase39 setUp error, error description:" + e.getMessage());
		}
	}

	@Test
	public void test() {
		try {
			mainCL.attachCollection(commCS.getName() + "." + subCLName,
					(BSONObject) JSON.parse("{LowBound:{\"alph\":110},UpBound:{\"alph\":210}}"));
		} catch (BaseException e) {
			Assert.assertEquals(e.getErrorCode(), -23, e.getMessage());
			return;
		}
		Assert.fail("TestCase39 dose not pass, mainCL attach a non-existing subCL success");
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
