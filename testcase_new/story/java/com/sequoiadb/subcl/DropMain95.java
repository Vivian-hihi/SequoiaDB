package com.sequoiadb.subcl;

import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.List;

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
import com.sequoiadb.testcommon.SdbThreadBase;

/**
 * 一个连接重复地创建主表、子表、attach子表、删除主表，同时另一个连接重复地插入数据 ，两个用户连不同的coord操作 testlink case:
 * seqDB95
 * 
 * @author huangwenhua
 * @Date 2016.12.20
 * @version 1.00
 */
public class DropMain95 extends SdbTestBase {
	private Sequoiadb sdb = null;
	private Sequoiadb sdb1 = null;
	private Sequoiadb sdb2 = null;
	private CollectionSpace cs;
	private DBCollection maincl;
	private DBCollection subcl;
	private String mainclName = "maincl_95";
	private String subclName = "subcl_95";
	private List<String> hostList;
	private SimpleDateFormat df = new SimpleDateFormat(
			"YYYY-MM-dd HH:mm:ss.SSS");

	@BeforeClass
	public void setUp() {
		try {
			System.out.println("the TestCase : " + this.getClass().getName()
					+ " begin at:" + df.format(new Date()));
			sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "");
			// 调用CommLib中的getNodeAddress切换不同的coord
			CommLib lib = new CommLib();
			hostList = lib.getNodeAddress(sdb, "SYSCoord");
			sdb1 = new Sequoiadb(hostList.get(0), "", "");
		} catch (BaseException e) {
			Assert.fail(" TestCase95 setUp error:" + e.getMessage());
		}
		createCL();
	}

	@Test
	public void insertData() {
		DropCl DropClThread = new DropCl();
		Sequoiadb sdb2 = null;
		try {
			sdb2 = new Sequoiadb(hostList.get(1), "", "");
			DBCollection cl1 = sdb2.getCollectionSpace(SdbTestBase.csName)
					.getCollection(mainclName);
			BSONObject bson;
			DropClThread.start();
			if (cl1 == null) {
				return;
			}
			for (int i = 0; i < 1000; i++) {
				bson = new BasicBSONObject();
				bson.put("a", 5);
				bson.put("age", i);
				cl1.insert(bson);
			}
			if (!DropClThread.isSuccess()) {
				Assert.fail(DropClThread.getErrorMsg());
			}
		} catch (BaseException e) {
			Assert.assertEquals(-23, e.getErrorCode(), e.getMessage());
			return;
		} finally {
			if (sdb2 != null) {
				sdb2.disconnect();
			}
			DropClThread.join();
		}
	}

	class DropCl extends SdbThreadBase {
		@Override
		public void exec() throws Exception {
			try {
				sdb1.getCollectionSpace(csName)
						.dropCollection(maincl.getName());
			} catch (BaseException e) {
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
			if (cs.isCollectionExist(subclName)) {
				cs.dropCollection(subclName);
			}

		} catch (BaseException e) {
			Assert.fail(e.getMessage());
		} finally {
			System.out.println("the TestCase Name:" + this.getClass().getName()
					+ ". the TestCase end at:" + this.df.format(new Date()));
			if (sdb != null) {
				sdb.disconnect();
			}
			if (sdb1 != null) {
				this.sdb1.disconnect();
			}
			if (sdb2 != null) {
				this.sdb2.disconnect();
			}
		}
	}

	public void createCL() {
		try {
			if (!sdb1.isCollectionSpaceExist(SdbTestBase.csName)) {
				sdb1.createCollectionSpace(SdbTestBase.csName);
			}
		} catch (BaseException e) {
			// -33 CS exist,ignore exceptions
			Assert.assertEquals(-33, e.getErrorCode(), e.getMessage());
		}
		try {
			cs = sdb1.getCollectionSpace(SdbTestBase.csName);
			BSONObject mainObj = (BSONObject) JSON
					.parse("{IsMainCL:true,ShardingKey:{a:1}}");
			BSONObject subObj = (BSONObject) JSON
					.parse("{ShardingKey:{a:1},ShardingType:\"hash\"}");
			maincl = cs.createCollection(mainclName, mainObj);
			subcl = cs.createCollection(subclName, subObj);
		} catch (BaseException e) {
			Assert.fail("create is faild" + e.getMessage());
		}
		try {
			maincl.attachCollection(subcl.getFullName(),
					(BSONObject) JSON.parse("{LowBound:{a:1},UpBound:{a:100}}"));
		} catch (BaseException e) {
			Assert.fail("attach error:" + e.getMessage());
		}
	}
}
