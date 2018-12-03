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
 * @Description seqDB-12021: range切分表中创建全文索引并切分后再插入记录 
 * @author xiaoni Zhao
 * @date 2018/11/23
 */
public class SplitAndInsert12021 extends SdbTestBase {
	private Sequoiadb sdb = null;
	private DBCollection cl;
	private String clName = "ES_cl_12021";
	private int insertNum = FullTextUtils.INSERT_NUMS;
	private String fullTextIndexName = "fullIndex12021";
	private Client esClient = null;
	private String srcGroup = null;
	private String desGroup = null;
	
	@BeforeClass
	public void setUp() {
		
		sdb = new Sequoiadb(SdbTestBase.coordUrl,"","");
		CommLib commLib = new CommLib();
		if (commLib.isStandAlone(sdb)) {
			throw new SkipException("StandAlone environment!");
		}
		ArrayList<String> groupsName = CommLib.getDataGroupNames(sdb);
        if (groupsName.size() < 2) {
             throw new SkipException("current environment less than tow groups ");
        }
        srcGroup = groupsName.get(0);
        desGroup = groupsName.get(1);
        
		cl = sdb.getCollectionSpace(csName).createCollection(clName, (BSONObject)JSON
				.parse("{ShardingType:'range', ShardingKey:{a:1}, Group:'" +srcGroup+ "'}"));
		esClient = FullTextESUtils.createTransportClient(SdbTestBase.esHostName, Integer
				.parseInt(SdbTestBase.esServiceName));
	}
	
	@Test
    public void test() {
		CollectionSpace cs = sdb.getCollectionSpace(csName);
		cl.createIndex(fullTextIndexName, (BSONObject)JSON.parse("{a : 'text', b : 'text'}"), false, false);
		cl.split(srcGroup, desGroup, (BSONObject)JSON.parse("{a : 1}"), (BSONObject)JSON.parse("{a : 1000}"));
		insertData();
		FullTextUtils.checkFullSyncToES(esClient, sdb, csName, clName, fullTextIndexName, insertNum);
		
		List<String> esIndexNames = FullTextDBUtils.getESIndexNames(sdb, csName, clName, fullTextIndexName);
		FullTextDBUtils.dropFullTextIndex(cl, fullTextIndexName);
		FullTextUtils.checkIndexNotExistInES(esClient, esIndexNames);
	}
	
	@AfterClass
    public void tearDown() {
		CollectionSpace cs = sdb.getCollectionSpace(csName);
		if(cs.isCollectionExist(clName)){
			cs.dropCollection(clName);
		}
    }
	
	public void insertData() {
		List<BSONObject> records = new ArrayList<BSONObject>();
		try {
			for(int i = 0; i < insertNum/1000; i++) {
				for(int j = 0; j < insertNum/200; j++) {
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
}
