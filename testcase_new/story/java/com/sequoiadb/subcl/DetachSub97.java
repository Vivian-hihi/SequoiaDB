package com.sequoiadb.subcl;

import java.text.SimpleDateFormat;
import java.util.Date;

import org.bson.BSONObject;
import org.bson.util.JSON;
import org.testng.Assert;
import org.testng.SkipException;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import com.sequoiadb.base.CollectionSpace;
import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.testcommon.CommLib;
import com.sequoiadb.testcommon.SdbTestBase;
import com.sequoiadb.testcommon.SdbThreadBase;

/**
 * 1. 创建主表、子表A、子表B 2. 一个连接重复地挂载分离子表A，同时另一个连接重复地挂载分离子表B，且子表A和子表B挂载范围不同 testlink
 * case: seqDB97
 * 
 * @author huangwenhua
 * @Date 2016.12.20
 * @version 1.00
 */
public class DetachSub97 extends SdbTestBase {
	private Sequoiadb sdb = null;
	private CollectionSpace cs;
	private CollectionSpace cs1;
	private CollectionSpace cs2;
	private String mainclName = "maincL97";
	private String subclName1 = "subcL97_1";
	private String subclName2 = "subcL97_2";
	private SimpleDateFormat df = new SimpleDateFormat(
			"YYYY-MM-dd HH:mm:ss.SSS");
	private DBCollection maincl;
	private DBCollection subcl1;
	private DBCollection subcl2;

	@BeforeClass
	public void setUp() {
		try {
			System.out.println("the TestCase: " + this.getClass().getName()
					+ " begin at:" + df.format(new Date()));
			sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "");
			CommLib lib = new CommLib();
			if (lib.isStandAlone(sdb)) {
				throw new SkipException("skip standalone");
			}

		} catch (BaseException e) {
			Assert.fail(" TestCase95 setUp error:" + e.getMessage());
		}
		createCL();
	}

	@Test
	public void checkAttach1() {
		Sequoiadb db1 = new Sequoiadb(SdbTestBase.coordUrl, "", "");
		cs1 = db1.getCollectionSpace(SdbTestBase.csName);

		BSONObject subObj = (BSONObject) JSON
				.parse("{ShardingKey:{a:1},ShardingType:\"hash\"}");
		subcl1 = cs1.createCollection(subclName1, subObj);

		CheckAttach CheckAttachThread = new CheckAttach();
		CheckAttachThread.start();
		try {
			maincl.attachCollection(subcl1.getFullName(),
					(BSONObject) JSON.parse("{LowBound:{a:1},UpBound:{a:100}}"));
			maincl.detachCollection(subcl1.getFullName());
			if (!CheckAttachThread.isSuccess()) {
				Assert.fail(CheckAttachThread.getErrorMsg());
			}
		} catch (BaseException e) {
			Assert.fail("checkAttach1 error:" + e.getMessage());
		} finally {
			CheckAttachThread.join();
		}
	}

	class CheckAttach extends SdbThreadBase {
		@Override
		public void exec() throws Exception {
			Sequoiadb db2 = null;
			try {
			    db2 = new Sequoiadb(SdbTestBase.coordUrl, "", "");
				cs2 = db2.getCollectionSpace(SdbTestBase.csName);
				BSONObject subObj = (BSONObject) JSON
						.parse("{ShardingKey:{a:1},ShardingType:\"hash\"}");
				subcl2 = cs2.createCollection(subclName2, subObj);
				maincl.attachCollection(subcl2.getFullName(), (BSONObject) JSON
						.parse("{LowBound:{a:100},UpBound:{a:200}}"));
			} catch (BaseException e) {
				throw e;
			}
			try {
				maincl.detachCollection(subcl2.getFullName());
			} catch (BaseException e) {
				if (db2 != null) {
					db2.disconnect();
				}
				
				throw e;
			}
		}
	}

	@AfterClass
	public void tearDown() {
		try {
			if (cs.isCollectionExist(mainclName)) {
				cs.dropCollection(mainclName);
			}
			if (cs.isCollectionExist(subclName1)) {
				cs.dropCollection(subclName1);
			}
			if (cs.isCollectionExist(subclName2)) {
				cs.dropCollection(subclName2);
			}
		} catch (BaseException e) {
			Assert.fail(e.getMessage());
		} finally {
			System.out.println("the TestCase Name:" + this.getClass().getName()
					+ ". the TestCase end at:" + this.df.format(new Date()));
			if (sdb != null) {
				this.sdb.disconnect();
			}
			
		}
	}

	public void createCL() {
		try {
			if (!sdb.isCollectionSpaceExist(SdbTestBase.csName)) {
				sdb.createCollectionSpace(SdbTestBase.csName);
			}
		} catch (BaseException e) {
			// -33 CS exist,ignore exceptions
			Assert.assertEquals(-33, e.getErrorCode(), e.getMessage());
		}
		try {
			cs = sdb.getCollectionSpace(SdbTestBase.csName);
			BSONObject mainObj = (BSONObject) JSON
					.parse("{IsMainCL:true,ShardingKey:{a:1}}");
			maincl = cs.createCollection(mainclName, mainObj);
		} catch (BaseException e) {
			Assert.fail("create is faild" + e.getMessage());
		}
	}
}
