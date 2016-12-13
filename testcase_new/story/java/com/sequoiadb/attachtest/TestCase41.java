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
 * @TestName:SEQDB-41 1.创建主表,shardingkey:time； 2.创建子表； 3.attach子表时区间的key不为time；
 * @author huangqiaohui
 * @version 1.00
 *
 */

public class TestCase41 extends SdbTestBase {
	private Sequoiadb sdb;
	private CollectionSpace commCS;
	private DBCollection mainCL;
	private DBCollection subCL;
	private String mainCLName = "mainCL41";
	private String subCLName = "subCL41";

	@BeforeClass
	public void setUp() {
		try {
			System.out.println("the TestCase Name:" + this.getClass().getName() + ". the TestCase begin at:"
					+ new SimpleDateFormat("YYYY-MM-dd HH:mm:ss.SSS").format(new Date()));
			sdb = new Sequoiadb(coordUrl, "", "");
			commCS = sdb.getCollectionSpace(csName);
			mainCL = MySdbTools.createCL(mainCLName, commCS, "{IsMainCL:true,ShardingKey:{\"time\":1}}");
			subCL = MySdbTools.createCL(subCLName, commCS, "{ShardingKey:{\"tx_id\":1},ShardingType:\"hash\"}");
		} catch (Exception e) {
			tearDown();
			Assert.fail("TestCase41 setUp error, error description:" + e.getMessage());
		}
	}

	@Test
	public void test() {
		try {
			mainCL.attachCollection(this.subCL.getFullName(),
					(BSONObject) JSON.parse("{LowBound:{\"alph\":100},UpBound:{\"alph\":200}}"));
		} catch (BaseException e) {
			Assert.assertEquals(e.getErrorCode(), -238, e.getMessage());
			return;
		}
		Assert.fail(
				"TestCase41 dose not pass, mainCL(shardingkey:{time:1}) attach subCL {LowBound:{\"alph\":100},UpBound:{\"alph\":200}} success");
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
				if (commCS.isCollectionExist(subCLName)) {
					commCS.dropCollection(subCLName);
				}
			}
		} catch (BaseException e) {
			// do something???
		} finally {
			if (sdb != null) {
				sdb.disconnect();
			}
		}
	}
}
