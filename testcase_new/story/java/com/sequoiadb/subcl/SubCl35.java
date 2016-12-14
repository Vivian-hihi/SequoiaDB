package com.sequoiadb.subcl;

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
import com.sequoiadb.testcommon.CommLib;
import com.sequoiadb.testcommon.SdbTestBase;
import com.sequoiadb.util.MySdbTools;

/**
 * @TestName:SEQDB-35 1.创建主表，子表； 2.挂载子表时，指定左区间的Key为String类型，右区间的key为int类型；
 * @author huangqiaohui
 * @version 1.00
 *
 */

public class SubCl35 extends SdbTestBase {
	private Sequoiadb sdb;
	private CollectionSpace commCS;
	private DBCollection mainCL;
	private DBCollection subCL;
	private String mainCLName = "mainCL35";
	private String subCLName = "subCL35";

	@BeforeClass
	public void setUp() {
		try {
			System.out.println("the TestCase Name:" + this.getClass().getName() + ". the TestCase begin at:"
					+ new SimpleDateFormat("YYYY-MM-dd HH:mm:ss.SSS").format(new Date()));
			sdb = new Sequoiadb(coordUrl, "", "");
			commCS = sdb.getCollectionSpace(csName);
			mainCL = MySdbTools.createCL(mainCLName, commCS, "{IsMainCL:true,ShardingKey:{\"alph\":-1}}");
			subCL = MySdbTools.createCL(subCLName, commCS, "{ShardingKey:{\"tx_id\":1},ShardingType:\"hash\"}");
		} catch (Exception e) {
			tearDown();
			Assert.fail("SubCl35 setUp error, error description:" + e.getMessage());
		}
	}

	@Test
	public void test() {
		try {
			CommLib commlib = new CommLib();
			if(commlib.isStandAlone(sdb)){
				return;
			}
			mainCL.attachCollection(this.subCL.getFullName(),
					(BSONObject) JSON.parse("{LowBound:{\"alph\":\"boy\"},UpBound:{\"alph\":9}}"));
			BSONObject bobj = new BasicBSONObject();
			bobj.put("alph", "abc");
			bobj.put("age", 89);
			bobj.put("name", "Tom");
			mainCL.insert(bobj);
			DBCursor dbc = subCL.query();
			BSONObject resault = dbc.getNext();
			dbc.close();
			Assert.assertEquals(resault, bobj);
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
			// do something???
		} finally {
			if (sdb != null) {
				sdb.disconnect();
			}
		}
	}
}
