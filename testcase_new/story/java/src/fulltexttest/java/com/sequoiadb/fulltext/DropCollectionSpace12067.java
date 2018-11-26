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
import com.sequoiadb.base.DBCursor;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.testcommon.CommLib;
import com.sequoiadb.testcommon.SdbTestBase;

/**
 * @Description seqDB-12067: 集合空间上存在多个全文索引，删除集合空间 
 * @author yinzhen
 * @date 2018/11/20
 */
public class DropCollectionSpace12067 extends SdbTestBase {
	private Sequoiadb sdb;
	private CollectionSpace cs;
	private String csName12067 = "cs12067";
	private String fullIndexName = "fullIndex12067";
	private Client esClient = null;
	
	@BeforeClass
	public void setUp() {
		this.sdb = new Sequoiadb(SdbTestBase.coordUrl,"","");
		CommLib commLib = new CommLib();
		if (commLib.isStandAlone(sdb)) {
			throw new SkipException("StandAlone environment!");
		}
		this.cs = sdb.createCollectionSpace(csName12067);
		esClient = FullTextESUtils.createTransportClient(SdbTestBase.esHostName, Integer.parseInt(SdbTestBase.esServiceName));
	}
	
	@Test
	public void test() {
		//创建集合空间及多个集合
		String clName1 = "dropCollectionSpace12067_1";
		DBCollection cl1 = this.cs.createCollection(clName1);
		String clName2 = "dropCollectionSpace12067_2";
		DBCollection cl2 = this.cs.createCollection(clName2);
		String clName3 = "dropCollectionSpace12067_3";
		DBCollection cl3 = this.cs.createCollection(clName3);
		
		//在所有集合上均创建全文索引，并插入包含索引字段的数据
		cl1.createIndex(fullIndexName, "{\"a\":\"text\"}", false, false);
		this.insertData(cl1);
		cl2.createIndex(fullIndexName, "{\"a\":\"text\"}", false, false);
		this.insertData(cl2);
		cl3.createIndex(fullIndexName, "{\"a\":\"text\"}", false, false);
		this.insertData(cl3);
		
		//删除集合空间
		List<String> esIndexNames1 = FullTextDBUtils.getESIndexNames(sdb, csName12067, clName1, fullIndexName);
		FullTextUtils.checkFullSyncToES(esClient, sdb, csName12067, clName1, fullIndexName, 500000);
		List<String> esIndexNames2 = FullTextDBUtils.getESIndexNames(sdb, csName12067, clName2, fullIndexName);
		FullTextUtils.checkFullSyncToES(esClient, sdb, csName12067, clName2, fullIndexName, 500000);
		List<String> esIndexNames3 = FullTextDBUtils.getESIndexNames(sdb, csName12067, clName3, fullIndexName);
		FullTextUtils.checkFullSyncToES(esClient, sdb, csName12067, clName3, fullIndexName, 500000);
		FullTextDBUtils.dropCollectionSpace(sdb, csName12067);
		FullTextUtils.checkIndexNotExistInES(esClient, esIndexNames1);
		FullTextUtils.checkIndexNotExistInES(esClient, esIndexNames2);
		FullTextUtils.checkIndexNotExistInES(esClient, esIndexNames3);
	}
	
	@AfterClass
	public void tearDown() {
		try {
			sdb.dropCollectionSpace(csName12067);
		} catch (BaseException e) {
			if (-34 != e.getErrorCode()) {
				e.printStackTrace();
			}
		} finally {
			if (sdb != null) {
				sdb.close();
			}
		}
	}
	
	public void insertData(DBCollection cl) {
		List<BSONObject> records = new ArrayList<BSONObject>();
		try {
			for(int i = 0; i < 100; i++) {
				for(int j = 0; j < 5000; j++) {
					BSONObject record = (BSONObject)JSON.parse("{a:'a"+i+""+j+"',g:'g"+i+""+j+"'}");
					records.add(record);
				}
				cl.insert(records);
				records.clear();
			}
		} catch (BaseException e) {
			if (-321 == e.getErrorCode()) {
				throw new SkipException("---insert has an err:SEQUOIADBMAINSTREAM-3827---");
			}
		}
	}
}
