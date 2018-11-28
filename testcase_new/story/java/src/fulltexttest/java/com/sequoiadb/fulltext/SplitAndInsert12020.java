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

import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.testcommon.CommLib;
import com.sequoiadb.testcommon.SdbTestBase;
/**
 * @Description seqDB-12020:hash切分表中创建全文索引并切分后再插入记录  
 * @author xiaoni Zhao
 * @date 2018/11/23
 */
public class SplitAndInsert12020 extends SdbTestBase {
	private Sequoiadb sdb = null;
	private DBCollection cl;
	private String clName = "cl_12020";
	private String fullTextIndexName = "fullIndex12020";
	private int insertNum = 500000;
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
				.parse("{ShardingType:'hash', ShardingKey:{a:1}, Group:'" +srcGroup+ "'}"));
		esClient = FullTextESUtils.createTransportClient(SdbTestBase.esHostName, Integer
				.parseInt(SdbTestBase.esServiceName));
	}
	
	@Test
    public void test() {
		cl.createIndex(fullTextIndexName, (BSONObject)JSON.parse("{a : 'text', b : 'text'}"), false, false);
		cl.split(srcGroup, desGroup, 50);
		insertData();
		FullTextUtils.checkFullSyncToES(esClient, sdb, csName, clName, fullTextIndexName, insertNum);
		
		DBCollection srcCL = sdb.getReplicaGroup(srcGroup).getMaster().connect().getCollectionSpace(csName).getCollection(clName);
		DBCollection desCL = sdb.getReplicaGroup(desGroup).getMaster().connect().getCollectionSpace(csName).getCollection(clName);
		long srcCount = srcCL.getCount();
		long desCount = desCL.getCount();
		System.out.println("srcCount:"+srcCount);
		System.out.println("desCount:"+desCount);
		long esCount = cl.getCount("{'' : {$Text : {'query' : {'match_all' :{}}}}}");
		Assert.assertEquals(esCount, srcCount+desCount, "records is wrong!");
	}
	
	@AfterClass
    public void tearDown() {
         sdb.getCollectionSpace(csName).dropCollection(clName);
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
}
