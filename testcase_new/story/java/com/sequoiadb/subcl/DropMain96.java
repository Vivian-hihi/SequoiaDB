package com.sequoiadb.subcl;

import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.List;

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
import com.sequoiadb.testcommon.CommLib;
import com.sequoiadb.testcommon.SdbTestBase;
import com.sequoiadb.testcommon.SdbThreadBase;

/**
 * 一个连接重复地创建主表、子表、删除主表 ，同时另一个连接重复地挂载子表，两个连接使用不同的coord地址
 *  testlink case: seqDB95
 * 
 * @author huangwenhua
 * @Date 2016.12.20
 * @version 1.00
 */
public class DropMain96 extends SdbTestBase {
	private Sequoiadb sdb = null;
	private Sequoiadb sdb1 = null;
	private Sequoiadb sdb2 = null;
	private CollectionSpace cs;
	private CollectionSpace cs2;
	private String mainclName = "maincl_96";
	private String subclName = "subcl_96";
	private List<String> hostList;
	private SimpleDateFormat df = new SimpleDateFormat(
			"YYYY-MM-dd HH:mm:ss.SSS");
	private DBCollection maincl;
	private DBCollection subcl;

	@BeforeClass
	public void setUp() {
		try {
			System.out.println("the TestCase: " + this.getClass().getName()
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
	public void checkAttch() {
		CheckDrop CheckDropThread = new CheckDrop();
		try {
			CheckDropThread.start();
			maincl.attachCollection(subcl.getFullName(), (BSONObject) JSON
					.parse("{LowBound:{a:100},UpBound:{a:200}}"));
			if (!CheckDropThread.isSuccess()) {
				Assert.fail(CheckDropThread.getErrorMsg());
			}
		} catch (BaseException e) {
			Assert.assertEquals(-23, e.getErrorCode(), e.getMessage());
		}finally{
			CheckDropThread.join();
		}
	}

	class CheckDrop extends SdbThreadBase {
		@Override
		public void exec() throws BaseException {
			cs = sdb1.getCollectionSpace(SdbTestBase.csName);
			try {
				cs.dropCollection(mainclName);
			} catch (BaseException e) {
				throw e;
			}
		}
	}

	@AfterClass
	public void tearDown() {
		System.out.println("the TestCase Name:" + this.getClass().getName()
				+ ". the TestCase end at:" + this.df.format(new Date()));
		try {
			if (cs2.isCollectionExist(mainclName)) {
				cs2.dropCollection(mainclName);
			}
			if (cs2.isCollectionExist(subclName)) {
				cs2.dropCollection(subclName);
			}
		} catch (BaseException e) {
			Assert.fail(e.getMessage());
		} finally {
			if (sdb != null) {
				this.sdb.disconnect();
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
			sdb2 = new Sequoiadb(hostList.get(1), "", "");
			cs2 = sdb2.getCollectionSpace(SdbTestBase.csName);
			BSONObject mainObj = (BSONObject) JSON
					.parse("{IsMainCL:true,ShardingKey:{a:1}}");
			BSONObject subObj = (BSONObject) JSON
					.parse("{ShardingKey:{a:1},ShardingType:\"hash\"}");
			maincl = cs2.createCollection(mainclName, mainObj);
			subcl = cs2.createCollection(subclName, subObj);
		} catch (BaseException e) {
			Assert.fail("create is faild" + e.getMessage());
		}
	}
}
