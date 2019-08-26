package com.sequoiadb.basicoperation;

import java.util.Date;

import org.bson.BSONObject;
import org.bson.BasicBSONObject;
import org.bson.util.DateInterceptUtil;
import org.testng.Assert;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import com.sequoiadb.base.CollectionSpace;
import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.DBCursor;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.testcommon.SdbTestBase;

/**
 * @TestLink: seqDB-19223
 * @describe: 插入date类型数据
 * @author wangkexin
 * @Date 2019.08.26
 * @version 1.00
 */
public class TestInsertDate19223 extends SdbTestBase {
	private Sequoiadb sdb;
	private CollectionSpace commcs = null;
	private DBCollection cl = null;
	private String clName = "cl_19223";

	@BeforeClass
	private void setUp() {
		sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "");
		commcs = sdb.getCollectionSpace(SdbTestBase.csName);
		cl = commcs.createCollection(clName);
	}

	@Test
	public void test() {
		Date dataTest1 = new Date();
		DateTest("a", dataTest1);

		Date dataTest2 = DateInterceptUtil.interceptDate(new Date(), "yyyy-MM-dd HH:mm");
		DateTest("b", dataTest2);

		Date dataTest3 = DateInterceptUtil.interceptDate(new Date(), "yyyy-MM-dd HH");
		DateTest("c", dataTest3);

		Date dataTest4 = DateInterceptUtil.interceptDate(new Date(), "yyyy-MM-dd");
		DateTest("d", dataTest4);

		Date dataTest5 = DateInterceptUtil.interceptDate(new Date(), "yyyy-MM");
		DateTest("e", dataTest5);
		
		Date dataTest6 = DateInterceptUtil.interceptDate(new Date(), "yyyy");
		DateTest("f", dataTest6);
		
		Date dataTest7 = DateInterceptUtil.interceptDate(new Date(), "MM");
		DateTest("g", dataTest7);
		
		Date dataTest8 = DateInterceptUtil.interceptDate(new Date(), "dd");
		DateTest("h", dataTest8);
	}

	@AfterClass
	private void tearDown() {
		try {
			commcs.dropCollection(clName);
		} finally {
			if (sdb != null) {
				sdb.close();
			}
		}
	}

	private void DateTest(String field, Date value) {
		Date expectDate = DateInterceptUtil.interceptDate(value, "yyyy-MM-dd");
		BSONObject bsonObject = new BasicBSONObject();
		bsonObject.put(field, value);
		cl.insert(bsonObject);
		DBCursor cursor = cl.query(bsonObject, null, null, null);
		int count = 0;
		while (cursor.hasNext()) {
			BSONObject obj = cursor.getNext();
			Date actDate = (Date) obj.get(field);
			Assert.assertEquals(actDate, expectDate, "field = " + field + ", value = " + value);
			count++;
		}
		Assert.assertEquals(count, 1, "field = " + field + ", value = " + value);
	}
}
