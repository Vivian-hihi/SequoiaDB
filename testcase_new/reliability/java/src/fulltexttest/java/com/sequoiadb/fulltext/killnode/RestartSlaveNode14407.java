package com.sequoiadb.fulltext.killnode;

import java.util.ArrayList;
import java.util.List;

import org.bson.BSONObject;
import org.bson.util.JSON;
import org.elasticsearch.client.Client;
import org.testng.Assert;
import org.testng.SkipException;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import com.sequoiadb.base.CollectionSpace;
import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.Node;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.fulltext.FullTextESUtils;
import com.sequoiadb.fulltext.FullTextUtils;
import com.sequoiadb.testcommon.CommLib;
import com.sequoiadb.testcommon.SdbTestBase;

/**
 * @Description seqDB-14407:正常启动DB备节点不影响全文索引功能
 * @author yinzhen
 * @date 2018/11/15
 */
public class RestartSlaveNode14407 extends SdbTestBase {
	private Sequoiadb sdb;
	private DBCollection cl;
	private String clName = "restartSlaveNode14407";
	private String fullIndexName = "fullIndex14407";
	private List<String> groupNames;
	private Client esClient = null;
	
	@BeforeClass
	public void setUp() {
		this.sdb = new Sequoiadb(SdbTestBase.coordUrl,"","");
		CommLib commLib = new CommLib();
		if (commLib.isStandAlone(sdb)) {
			throw new SkipException("StandAlone environment!");
		}
		this.groupNames = commLib.getDataGroupNames(sdb);
		CollectionSpace cs = sdb.getCollectionSpace(SdbTestBase.csName);
		this.cl = cs.createCollection(clName, (BSONObject)JSON.parse("{ShardingKey:{a:1},ShardingType:'range',Group:'"+this.groupNames.get(0)+"'}"));
		esClient = FullTextESUtils.createTransportClient(SdbTestBase.esHostName, Integer.parseInt(SdbTestBase.esServiceName));
	}
	
	@Test
	public void test() {
		//创建全文索引，插入记录
		this.cl.createIndex(fullIndexName, "{\"a\":\"text\"}", false, false);
		this.insertData();
		FullTextUtils.checkFullSyncToES(esClient, sdb, SdbTestBase.csName, this.clName, this.fullIndexName, 500000);
		Node slaveNode = this.sdb.getReplicaGroup(this.groupNames.get(0)).getSlave();
		slaveNode.stop();
		slaveNode.start();
		FullTextUtils.checkFullSyncToES(esClient, sdb, SdbTestBase.csName, this.clName, this.fullIndexName, 500000);
		
		//insert
		this.insertData();
		FullTextUtils.checkFullSyncToES(esClient, sdb, SdbTestBase.csName, this.clName, this.fullIndexName, 1000000);
		
		//update
		this.cl.update("{a:'a01'}", "{'$set':{a:'helloworld'}}", "{'':null}");
		FullTextUtils.checkFullSyncToES(esClient, sdb, SdbTestBase.csName, this.clName, this.fullIndexName, 1000000);
		
		//delete
		this.cl.delete("{a:'a11'}");
		FullTextUtils.checkFullSyncToES(esClient, sdb, SdbTestBase.csName, this.clName, this.fullIndexName, 999998);
		
		//query
		this.cl.query();
		FullTextUtils.checkFullSyncToES(esClient, sdb, SdbTestBase.csName, this.clName, this.fullIndexName, 999998);
	}
	
	@AfterClass
	public void tearDown() {
		try {
			CollectionSpace cs = sdb.getCollectionSpace(SdbTestBase.csName);
			cs.dropCollection(this.clName);
		} catch (BaseException e) {
			Assert.fail(e.getMessage() + "\r\n" + this.getKeyStack(e, this));
		} finally {
			if (sdb != null) {
				sdb.close();
			}
		}
	}
	
	public void insertData() {
		List<BSONObject> records = new ArrayList<BSONObject>();
		try {
			for(int i = 0; i < 100; i++) {
				for(int j = 0; j < 5000; j++) {
					BSONObject record = (BSONObject)JSON.parse("{a:'a"+i+""+j+"',g:'g"+i+""+j+"'}");
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
	
	public String getKeyStack(Exception e, Object classObj) {
		StringBuffer stackBuffer = new StringBuffer();
		StackTraceElement[] stackElements = e.getStackTrace();
		for (int i = 0; i < stackElements.length; i++) {
			if (stackElements[i].toString().contains(classObj.getClass().getName())) {
				stackBuffer.append(stackElements[i].toString()).append("\r\n");
			}
		}
		String str = stackBuffer.toString();
		if (str.length() >= 2) {
			return str.substring(0, str.length() - 2);
		} else {
			return str;
		}
	}
}
