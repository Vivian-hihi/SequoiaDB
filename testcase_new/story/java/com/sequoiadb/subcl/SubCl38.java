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
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.testcommon.CommLib;
import com.sequoiadb.testcommon.SdbTestBase;
import com.sequoiadb.util.MySdbTools;

/**
 * @TestName:SEQDB-38 1.创建主表，两个子表； 2.attach的两个子表的区间范围没有相连； 3.写入数据验证；
 * @author huangqiaohui
 * @version 1.00
 *
 */

public class SubCl38 extends SdbTestBase {
	private Sequoiadb sdb;
	private CollectionSpace commCS;
	private DBCollection mainCL;
	private DBCollection subCL_1;
	private DBCollection subCL_2;
	private String mainCLName = "mainCL38";
	private String subCLName_1 = "subCL38_1";
	private String subCLName_2 = "subCL38_2";

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
			Assert.fail("TestCase38 setUp error, error description:" + e.getMessage());
		}
	}

	@Test
	public void test() {
		try {
			CommLib commlib = new CommLib();
			if (commlib.isStandAlone(sdb)) {
				return;
			}
			this.mainCL.attachCollection(this.subCL_1.getFullName(),
					(BSONObject) JSON.parse("{LowBound:{\"alph\":10},UpBound:{\"alph\":100}}"));
			this.mainCL.attachCollection(this.subCL_2.getFullName(),
					(BSONObject) JSON.parse("{LowBound:{\"alph\":110},UpBound:{\"alph\":210}}"));
		} catch (BaseException e) {
			Assert.fail(e.getMessage());
		}
		BSONObject bobj = new BasicBSONObject();
		bobj.put("alph", 80);
		bobj.put("age", 89);
		bobj.put("name", "Tom");
		BSONObject bobj1 = new BasicBSONObject();
		bobj1.put("alph", 120);
		bobj1.put("age", 89);
		bobj1.put("name", "Tom");
		try {
			mainCL.insert(bobj);
			mainCL.insert(bobj1);
		} catch (Exception e) {
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
