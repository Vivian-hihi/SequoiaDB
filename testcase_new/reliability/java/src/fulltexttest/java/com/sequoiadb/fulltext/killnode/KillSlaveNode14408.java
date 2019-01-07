package com.sequoiadb.fulltext.killnode;

import com.sequoiadb.commlib.CommLib;
import com.sequoiadb.base.CollectionSpace;
import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.DBCursor;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.commlib.GroupMgr;
import com.sequoiadb.commlib.GroupWrapper;
import com.sequoiadb.commlib.NodeWrapper;
import com.sequoiadb.commlib.SdbTestBase;
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.exception.ReliabilityException;
import com.sequoiadb.fault.KillNode;
import com.sequoiadb.fulltext.FullTextESUtils;
import com.sequoiadb.fulltext.FullTextUtils;
import com.sequoiadb.task.FaultMakeTask;
import com.sequoiadb.task.OperateTask;
import com.sequoiadb.task.TaskMgr;
import org.bson.BSONObject;
import org.bson.util.JSON;
import org.elasticsearch.client.Client;
import org.testng.Assert;
import org.testng.SkipException;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Date;
import java.util.List;

/**
 * @Description seqDB-14406:异常启动DB备节点不影响全文索引功能
 * @author yinzhen
 * @date 2018/11/23
 */

public class KillSlaveNode14408 extends SdbTestBase {
	private String clName = "killSlaveNode14408";
	private CollectionSpace cs;
	private DBCollection cl;
	private GroupMgr groupMgr = null;
	private Sequoiadb sdb;
	private boolean clearFlag = false;
	private String groupName;
	private Client esClient = null;
	private String fullIndexName = "fullIndex14408";

	@BeforeClass()
	public void setUp() {
		try {
			CommLib commLib = new CommLib();
			if (commLib.isStandAlone(sdb)) {
				throw new SkipException("StandAlone environment!");
			}

			System.out.println("the TestCase Name:" + this.getClass().getName() + ". the TestCase begin at:"
					+ new SimpleDateFormat("YYYY-MM-dd HH:mm:ss.SSS").format(new Date()));
			groupMgr = groupMgr.getInstance();

			// CheckBusiness(true),检测当前集群环境，若存在异常返回false，
			if (!groupMgr.checkBusiness(20)) {
				throw new SkipException("checkBusiness return false");
			}
			groupName = groupMgr.getAllDataGroupName().get(0);

			sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "");
			cs = sdb.getCollectionSpace(csName);
			cl = cs.createCollection(clName, (BSONObject) JSON.parse("{Group:'" + this.groupName + "'}"));

			esClient = FullTextESUtils.createTransportClient(SdbTestBase.esHostName,
					Integer.parseInt(SdbTestBase.esServiceName));
		} catch (ReliabilityException e) {
			if (sdb != null) {
				sdb.close();
			}
			Assert.fail(this.getClass().getName() + " setUp error, error description:" + e.getMessage() + "\r\n"
					+ this.getStackString(e));
		}
	}

	public void insertData() {
		List<BSONObject> records = new ArrayList<BSONObject>();
		try {
			for (int i = 0; i < 100; i++) {
				for (int j = 0; j < 5000; j++) {
					BSONObject record = (BSONObject) JSON.parse("{a:'a" + i + "" + j + "',g:'g" + i + "" + j + "'}");
					records.add(record);
				}
				this.cl.insert(records);
				records.clear();
			}
		} catch (BaseException e) {
			if (-321 == e.getErrorCode()) {
				throw new SkipException("---insert has an err:SEQUOIADBMAINSTREAM-3827---");
			}
		}
	}

	@Test
	public void test() {
		try {
			this.cl.createIndex(fullIndexName, "{\"a\":\"text\"}", false, false);
			this.insertData();
			FullTextUtils.checkFullSyncToES(esClient, sdb, SdbTestBase.csName, this.clName, this.fullIndexName, 500000);
			GroupWrapper subCLGroup = groupMgr.getGroupByName(groupName);
			NodeWrapper subCLGroupMaster = subCLGroup.getSlave();
			System.out.println("Kill node:" + subCLGroupMaster.hostName() + ":" + subCLGroupMaster.svcName());

			// 建立并行任务
			FaultMakeTask faultTask = KillNode.getFaultMakeTask(subCLGroupMaster.hostName(), subCLGroupMaster.svcName(),
					0);
			TaskMgr mgr = new TaskMgr(faultTask);
			mgr.execute();
			Assert.assertEquals(mgr.isAllSuccess(), true, mgr.getErrorMsg());
			Assert.assertEquals(groupMgr.checkBusiness(120), true);

			FullTextUtils.checkFullSyncToES(esClient, sdb, SdbTestBase.csName, this.clName, this.fullIndexName, 500000);

			// insert
			this.insertData();
			FullTextUtils.checkFullSyncToES(esClient, sdb, SdbTestBase.csName, this.clName, this.fullIndexName,
					1000000);

			// update
			this.cl.update("{a:'a01'}", "{'$set':{a:'helloworld'}}", "{'':null}");
			FullTextUtils.checkFullSyncToES(esClient, sdb, SdbTestBase.csName, this.clName, this.fullIndexName,
					1000000);

			// delete
			this.cl.delete("{a:'a11'}");
			FullTextUtils.checkFullSyncToES(esClient, sdb, SdbTestBase.csName, this.clName, this.fullIndexName, 999998);

			// query
			this.cl.query();
			FullTextUtils.checkFullSyncToES(esClient, sdb, SdbTestBase.csName, this.clName, this.fullIndexName, 999998);

			clearFlag = true;
		} catch (ReliabilityException e) {
			e.printStackTrace();
			Assert.fail(e.getMessage());
		} finally {
			sdb.closeAllCursors();
		}

	}

	@AfterClass
	public void tearDown() {
		try {
			if (clearFlag) {
				CollectionSpace commCS = sdb.getCollectionSpace(csName);
				commCS.dropCollection(clName);
			}
		} catch (BaseException e) {
			Assert.fail(e.getMessage() + "\r\n" + this.getStackString(e));
		} finally {
			if (sdb != null) {
				sdb.close();
			}
			System.out.println("the TestCase Name:" + this.getClass().getName() + ". the TestCase end at:"
					+ new SimpleDateFormat("YYYY-MM-dd HH:mm:ss.SSS").format(new Date()));
		}
	}

	// 获取异常的堆栈信息字串
	public static String getStackString(Exception e) {
		StringBuffer stackBuffer = new StringBuffer();
		StackTraceElement[] stackElements = e.getStackTrace();
		for (int i = 0; i < stackElements.length; i++) {
			stackBuffer.append(stackElements[i].toString()).append("\r\n");
		}
		String str = stackBuffer.toString();
		if (str.length() >= 2) {
			return str.substring(0, str.length() - 2);
		} else {
			return str;
		}
	}
}
