package com.sequoiadb.fulltext;

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
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.testcommon.CommLib;
import com.sequoiadb.testcommon.SdbTestBase;

/**
 * @Description seqDB-14372:无存量数据，插入记录 
 * @author yinzhen
 * @date 2018/11/19
 */
public class InsertRecords14372 extends SdbTestBase {
	private Sequoiadb sdb;
	private DBCollection cl;
	private String clName = "insertRecords14372";
	private String fullIndexName = "fullIndex14372";
	private Client esClient = null;
	
	@BeforeClass
	public void setUp() {
		this.sdb = new Sequoiadb(SdbTestBase.coordUrl,"","");
		CommLib commLib = new CommLib();
		if (commLib.isStandAlone(sdb)) {
			throw new SkipException("StandAlone environment!");
		}
		CollectionSpace cs = sdb.getCollectionSpace(SdbTestBase.csName);
		this.cl = cs.createCollection(clName);
		esClient = FullTextESUtils.createTransportClient(SdbTestBase.esHostName, Integer.parseInt(SdbTestBase.esServiceName));
		this.cl.createIndex(fullIndexName, "{\"a\":\"text\"}", false, false);
	}
	
	@Test
	public void test() {
		this.insertData();
		FullTextUtils.checkFullSyncToES(esClient, sdb, SdbTestBase.csName, this.clName, this.fullIndexName, 650000);
		this.insertData();		
		FullTextUtils.checkFullSyncToES(esClient, sdb, SdbTestBase.csName, this.clName, this.fullIndexName, 1300000);
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
			for(int i = 0; i < 65; i++) {
				for(int j = 0; j < 10000; j++) {
					BSONObject record = (BSONObject)JSON.parse("{a:'aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"+i+""+j+"',g:'gggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggg"+i+""+j+"'}");
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
