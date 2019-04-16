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
		cl = sdb.getCollectionSpace(SdbTestBase.csName).createCollection(clName);
	}

	@Test
	public void test() throws Exception {
		try{
			sdb.beginTransaction();
		}catch(BaseException e){
			if (CompressUtils.isStandAlone(sdb)){
				//dpslocal配置参数默认为false,执行事务操作报-3错误，如dpslocal为true时，未开启事务进行事务操作会报-253，关于此参数的配置已手工验证
				if(e.getErrorCode() == -253){
					Assert.fail("check the default value of configuraion parameter 'dpslocal'");
				}
				Assert.assertEquals(e.getErrorCode(), -3, "unexpected error");
				throw new SkipException("skip StandAlone");
			}else{
				throw e;
			}
		}
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