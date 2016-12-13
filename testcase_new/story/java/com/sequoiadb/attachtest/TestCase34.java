package com.sequoiadb.attachtest;

import java.text.SimpleDateFormat;
import java.util.Date;

import org.bson.BSONObject;
import org.bson.BasicBSONObject;
import org.bson.util.JSON;
import org.testng.Assert;
import org.testng.annotations.AfterClass;

import org.testng.annotations.BeforeClass;

import org.testng.annotations.Test;

import com.sequoiadb.base.CollectionSpace;
import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.DBCursor;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.testcommon.SdbTestBase;
import com.sequoiadb.util.MySdbTools;

/**
 * @TestName:SEQDB-34 1.创建主表，子表； 2.挂载子表时指定区间的Key为String类型；
 *                    3.往子表内写入区间同类型的属于区间的数据； 4.验证数据写入是否成功和正确；
 * @author huangqiaohui
 * @version 1.00
 *
 */

public class TestCase34 extends SdbTestBase {
	private Sequoiadb sdb;
	private CollectionSpace commCS;
	private DBCollection mainCL;
	private DBCollection subCL;
	private String mainCLName = "mainCL34";
	private String subCLName = "subCL34";

	@BeforeClass
	public void setUp() {
		try {
			System.out.println("the TestCase Name:" + this.getClass().getName() + ". the TestCase begin at:"
					+ new SimpleDateFormat("YYYY-MM-dd HH:mm:ss.SSS").format(new Date()));
			sdb = new Sequoiadb(coordUrl, "", "");
			commCS = sdb.getCollectionSpace(csName);
			mainCL = MySdbTools.createCL(mainCLName, commCS, "{IsMainCL:true,ShardingKey:{\"alph\":1}}");
			subCL = MySdbTools.createCL(subCLName, commCS, "{ShardingKey:{\"tx_id\":1},ShardingType:\"hash\"}");
		} catch (Exception e) {
			tearDown();// ��Ҫ��������Ѳ����CL
			Assert.fail("TestCase34 setUp error, error description:" + e.getMessage());
		}
	}

	@Test
	public void test() {
		try {
			mainCL.attachCollection(subCL.getFullName(),
					(BSONObject) JSON.parse("{LowBound:{\"alph\":\"boy\"},UpBound:{\"alph\":\"girl\"}}"));
			BSONObject bobj = new BasicBSONObject();
			bobj.put("alph", "boy");
			bobj.put("age", 89);
			bobj.put("name", "Tom");
			BSONObject bobj1 = new BasicBSONObject();
			bobj1.put("alph", "cat");
			bobj1.put("age", 100);
			bobj1.put("name", "jery");
			mainCL.insert(bobj);
			mainCL.insert(bobj1);
			DBCursor dc = subCL.query();
			while (dc.hasNext()) {
				BSONObject r = dc.getNext();
				Assert.assertEquals(r.equals(bobj) || r.equals(bobj1), true);
			}
			dc.close();
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
				if (commCS.isCollectionExist(subCLName)) {
					commCS.dropCollection(subCLName);
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
