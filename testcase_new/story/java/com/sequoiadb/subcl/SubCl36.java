package com.sequoiadb.subcl;

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
import com.sequoiadb.base.DBCursor;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.testcommon.CommLib;
import com.sequoiadb.testcommon.SdbTestBase;
import com.sequoiadb.util.MySdbTools;

/**
 * @TestName:SEQDB-36 1.创建主表，子表； 2.挂载子表时指定区间的Key为整数，长整数，浮点数，高精度数，字符串，日期，时间戳；
 *                    3.往子表内写入区间同类型的属于区间的数据； 4.验证数据写入是否成功和正确；
 * @author huangqiaohui
 * @version 1.00
 *
 */

public class SubCl36 extends SdbTestBase {
	private Sequoiadb sdb;
	private CollectionSpace commCS;
	private DBCollection mainCL;
	private DBCollection subCL;
	private String mainCLName = "mainCL36";
	private String subCLName = "subCL36";

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
			tearDown();
			Assert.fail("SubCl36 setUp error, error description:" + e.getMessage());
		}
	}

	@Test
	public void test() {
		try {
			CommLib commlib = new CommLib();
			if (commlib.isStandAlone(sdb)) {
				return;
			}
		} catch (BaseException e) {
			Assert.fail(e.getMessage());
		}
		for (int i = 0; i < 6; i++) {
			try {
				attachSelctor(i);
				dataSelector(i);
			} catch (BaseException e) {
				Assert.fail(e.getMessage());
			}
		}
	}

	public void attachSelctor(int type) throws BaseException {
		try {
			switch (type) {
			case 0:
				mainCL.attachCollection(this.subCL.getFullName(),
						(BSONObject) JSON.parse("{LowBound:{\"alph\":1},UpBound:{\"alph\":100}}"));
				break;
			case 1:
				mainCL.attachCollection(this.subCL.getFullName(),
						(BSONObject) JSON.parse("{LowBound:{\"alph\":3000000001},UpBound:{\"alph\":3000000010}}"));
				break;
			case 2:
				mainCL.attachCollection(this.subCL.getFullName(),
						(BSONObject) JSON.parse("{LowBound:{\"alph\":123.456},UpBound:{\"alph\":456.123}}"));
				break;
			case 3:
				mainCL.attachCollection(this.subCL.getFullName(), (BSONObject) JSON.parse(
						"{LowBound:{\"alph\":{$decimal:\"123.456\"}},UpBound:{\"alph\":{$decimal:\"456.123\"}}}"));
				break;
			case 4:
				mainCL.attachCollection(this.subCL.getFullName(),
						(BSONObject) JSON.parse("{LowBound:{\"alph\":\"abc\"},UpBound:{\"alph\":\"zero\"}}"));
				break;
			case 5:
				mainCL.attachCollection(this.subCL.getFullName(), (BSONObject) JSON.parse(
						"{LowBound:{\"alph\":{$date:\"2012-01-01\"}},UpBound:{\"alph\":{$date:\"2012-12-12\"}}}"));
				break;
			case 6:
				mainCL.attachCollection(this.subCL.getFullName(), (BSONObject) JSON.parse(
						"{LowBound:{\"alph\":{$timestamp:\"2012-01-01-13.14.26.124233\"}},UpBound:{\"alph\":{$timestamp:\"2012-12-12-13.14.26.124233\"}}}"));
				break;

			}
		} catch (Exception e) {
			Assert.fail(e.getMessage());
		}
	}

	public void dataSelector(int type) throws BaseException {
		BSONObject bobj = null;
		switch (type) {
		case 0:
			bobj = (BSONObject) JSON.parse("{name:\"Tom1\",alph:23}");
			break;
		case 1:
			bobj = (BSONObject) JSON.parse("{name:\"Tom2\",alph:3000000001}");
			break;
		case 2:
			bobj = (BSONObject) JSON.parse("{name:\"Tom3\",alph:124.456}");
			break;
		case 3:
			bobj = (BSONObject) JSON.parse("{name:\"Tom4\",alph:{$decimal:\"124.456\"}}");
			break;
		case 4:
			bobj = (BSONObject) JSON.parse("{name:\"Tom5\",alph:\"boy\"}");
			break;
		case 5:
			bobj = (BSONObject) JSON.parse("{name:\"Tom6\",alph:{$date:\"2012-02-01\"}}");
			break;
		case 6:
			bobj = (BSONObject) JSON.parse("{name:\"Tom7\",alph:{$timestamp:\"2012-02-01-13.14.26.124233\"}}");
			break;
		}
		try {
			mainCL.insert(bobj);
			DBCursor dbc = subCL.query();
			BSONObject actual = dbc.getNext();
			dbc.close();
			Assert.assertEquals(actual, bobj);
			mainCL.delete(bobj);
			mainCL.detachCollection(subCL.getFullName());
		} catch (BaseException e) {
			Assert.fail(e.getMessage());
			;
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
