package com.sequoiadb.transactionsync;

import java.util.ArrayList;
import java.util.List;

import org.bson.BSONObject;
import org.bson.BasicBSONObject;
import org.testng.Assert;
import org.testng.SkipException;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.DBCursor;
import com.sequoiadb.base.ReplicaGroup;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.testcommon.CommLib;
import com.sequoiadb.testcommon.SdbConfTestBase;
import com.sequoiadb.testcommon.SdbTestBase;

/**
 * test content:  并发回滚事务，操作相同cl_SD.transaction.041 
 * testlink-case: seqDB-6030
 * @author wangkexin
 * @Date 2019.03.28
 * @version 1.00
 */

public class SplitToDiffDataRG11901 extends SdbConfTestBase{
	private String clName = "cl11901";
	private Sequoiadb sdb = null;
	private String rgName = "group11901";
	ArrayList<BSONObject> insertRecords = new ArrayList<BSONObject>();
	
	@Override
    protected void setNodeConf(){
        dataConf.put("transactionon", true);
    }
	
	@BeforeClass
	private void setup() {
		sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "");
		// 跳过 standAlone
        CommLib commLib = new CommLib();
        if (commLib.isStandAlone(sdb)) {
            throw new SkipException("skip StandAlone");
        }
        createRG(sdb, rgName);
	}
	
	@Test
	public void test() throws Exception {
		split();
		insertDataAndCheck(sdb);
	}
	
	@AfterClass
	private void teardown() {
		sdb.getCollectionSpace(csName).dropCollection(clName);
		sdb.removeReplicaGroup(rgName);
		sdb.close();
	}
	
	private void createRG(Sequoiadb db, String rgName) {
		ReplicaGroup rg = db.createReplicaGroup(rgName);
        String hostName=db.getReplicaGroup("SYSCatalogGroup").getMaster().getHostName();
        
    	int dataPort = SdbTestBase.reservedPortBegin + 10;
    	String dataPath = SdbTestBase.reservedDir + "/data/" + dataPort;
        boolean checkSucc = false;
        int times = 0;
		int maxRetryTimes = 10;
		do {
			try {
				rg.createNode(hostName, dataPort, dataPath);
				checkSucc = true;
			} catch (BaseException e) {
				// -145:Node already exists
				if (e.getErrorCode() == -145 || e.getErrorCode() == -290) {
					dataPort = dataPort + 10;
					dataPath = SdbTestBase.reservedDir + "/data/" + dataPort;
				} else {
					Assert.fail("create node fail! port=" + dataPort + " errorCode: " + e.getErrorCode());
				}
			}
			times++;
		} while (!checkSucc && times < maxRetryTimes);
        rg.start();
    }
	
	private void split(){
		ArrayList<String> groupsName = CommLib.getDataGroupNames(sdb);
		String srcGroupName = groupsName.get(0);
		BSONObject options = new BasicBSONObject();
		BSONObject ShardingKey = new BasicBSONObject();
		ShardingKey.put("a", 1);
		options.put("ShardingKey", ShardingKey);
		options.put("ShardingType", "range");
		options.put("Group", srcGroupName);
		options.put("ReplSize", 0);
		DBCollection cl = sdb.getCollectionSpace(SdbTestBase.csName).createCollection(clName,options);
		
		BSONObject condition = new BasicBSONObject();
		condition.put("a", 10);
		BSONObject endcondition = new BasicBSONObject();
		endcondition.put("a", 100);
		cl.split(srcGroupName, rgName, condition, endcondition);
	}
	
	private void insertDataAndCheck(Sequoiadb db) {
		db.beginTransaction();
		DBCollection cl = db.getCollectionSpace(SdbTestBase.csName).getCollection(clName);
		for (int i = 0; i < 10; i++) {
			BSONObject data = new BasicBSONObject();
			data.put("a", i);
			cl.insert(data);
			insertRecords.add(data);
		}
		db.commit();
		checkResult(db);

		db.beginTransaction();
		BSONObject data = new BasicBSONObject();
		data.put("a", 10);
		try {
			cl.insert(data);
			Assert.fail("insert data should fail! data is : { a : 10 } ");
		} catch (BaseException e) {
			Assert.assertEquals(e.getErrorCode(), -253);
		}
		checkResult(db);
	}
	
	private void checkResult(Sequoiadb db){
		DBCollection cl = db.getCollectionSpace(SdbTestBase.csName).getCollection(clName);
		List<BSONObject> actualData = new ArrayList<>();
		DBCursor queryCursor = cl.query();
		while(queryCursor.hasNext()){
			actualData.add(queryCursor.getNext());
		}
		queryCursor.close();
		Assert.assertEquals(insertRecords, actualData, "Insert data does not match expected results");
	}
}