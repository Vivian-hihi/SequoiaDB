package com.sequoiadb.crud;

import org.bson.BSONObject;
import org.bson.BasicBSONObject;
import org.testng.Assert;
import org.testng.SkipException;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.crud.compress.concurrency.CompressUtils;
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.testcommon.SdbTestBase;

/**
 * test content: 事务功能关闭，执行事务操作_SD.transaction.001
 * testlink-case: seqDB-5990
 * @author wangkexin
 * @Date 2019.03.28
 * @version 1.00
 */

public class TransactiononIsFalse5990 extends SdbTestBase {
	private String clName = "cl5990";
	private Sequoiadb sdb = null;
	private DBCollection cl = null;

	@BeforeClass
	private void setup() {
		sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "");
		if (CompressUtils.isStandAlone(sdb)){
            throw new SkipException("is standalone skip testcase");
        }
		cl = sdb.getCollectionSpace(SdbTestBase.csName).createCollection(clName);
	}

	@Test
	public void test() throws Exception {
		sdb.beginTransaction();
		try {
			BSONObject rec = new BasicBSONObject();
			rec.put("_id", 5990);
			cl.insert(rec);
			Assert.fail("expect failure,but find success.");
		} catch (BaseException e) {
			Assert.assertEquals(e.getErrorCode(), -253, "unexpected error");
		}
		Assert.assertEquals(cl.getCount(), 0, "There should be no data in the collection");
	}

	@AfterClass
	private void teardown() {
		sdb.getCollectionSpace(csName).dropCollection(clName);
		sdb.close();
	}
}