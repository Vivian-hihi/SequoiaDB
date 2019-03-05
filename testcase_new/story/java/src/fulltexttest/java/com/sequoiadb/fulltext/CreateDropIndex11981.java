package com.sequoiadb.fulltext;

import java.util.ArrayList;
import java.util.Date;
import java.util.List;

import org.bson.BSONObject;
import org.bson.BasicBSONObject;
import org.bson.util.JSON;
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
import org.elasticsearch.client.*;

/**
 * FileName: CreateDropIndex11981.java test content: 在非空集合中创建/删除全文索引
 * 
 * @author liuxiaoxuan
 * @Date 2018.11.20
 */
public class CreateDropIndex11981 extends SdbTestBase {

	private Sequoiadb sdb = null;
	private CollectionSpace cs = null;
	private DBCollection cl = null;
	private String clName = "ES_11981";
	private Client esClient = null;

	@BeforeClass
	public void setUp() {
		esClient = FullTextESUtils.createTransportClient(esHostName, Integer.parseInt(esServiceName));
		sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "");
		if (CommLib.isStandAlone(sdb)) {
			throw new SkipException("skip StandAlone");
		}

		// create cl
		cs = sdb.getCollectionSpace(csName);
		cl = cs.createCollection(clName);
	}

	@AfterClass
	public void tearDown() {
		cs.dropCollection(clName);
		sdb.close();
		esClient.close();
	}

	@Test
	public void test() {
		// insert < 32M
		int insertNums1 = 100000; // 10w
		insertData(cl, insertNums1);

		// create fulltext
		String textIndexName = "fulltext11981";
		BSONObject indexObj = new BasicBSONObject();
		indexObj.put("a", "text");
		indexObj.put("b", "text");
		indexObj.put("c", "text");
		indexObj.put("d", "text");
		indexObj.put("e", "text");
		indexObj.put("f", "text");
		cl.createIndex(textIndexName, indexObj, false, false);

		List<String> esIndexNames = FullTextDBUtils.getESIndexNames(sdb, csName, clName, textIndexName);

		// check consistency
		FullTextUtils.checkFullSyncToES(esClient, sdb, csName, clName, textIndexName, insertNums1);
		FullTextUtils.checkConsistency(sdb, csName, clName);

		// drop fulltext
		FullTextDBUtils.dropFullTextIndex(cl, textIndexName);

		// check fulltext deleted
		FullTextUtils.checkIndexNotExistInES(esClient, esIndexNames);

		// insert > 32M, < 129M
		int insertNums2 = 100000; // new insert 10w
		insertData(cl, insertNums2);

		// create fulltext
		cl.createIndex(textIndexName, indexObj, false, false);

		// check consistency
		FullTextUtils.checkFullSyncToES(esClient, sdb, csName, clName, textIndexName, insertNums1 + insertNums2);
		FullTextUtils.checkConsistency(sdb, csName, clName);

		// drop fulltext
		FullTextDBUtils.dropFullTextIndex(cl, textIndexName);

		// check fulltext deleted
		FullTextUtils.checkIndexNotExistInES(esClient, esIndexNames);

		// insert > 32M, and > 129M
		int insertNums3 = 300000; // new insert 30w

		insertData(cl, insertNums3);

		// create fulltext
		cl.createIndex(textIndexName, indexObj, false, false);

		// check consistency
		FullTextUtils.checkFullSyncToES(esClient, sdb, csName, clName, textIndexName,
				insertNums1 + insertNums2 + insertNums3);
		FullTextUtils.checkConsistency(sdb, csName, clName);

		// drop fulltext
		FullTextDBUtils.dropFullTextIndex(cl, textIndexName);

		// check fulltext deleted
		FullTextUtils.checkIndexNotExistInES(esClient, esIndexNames);
	}

	public void insertData(DBCollection cl, int insertNums) {
		List<BSONObject> insertObjs = new ArrayList<>();
		for (int i = 0; i < 100; i++) {
			for (int j = 0; j < insertNums / 100; j++) {
				insertObjs.add((BSONObject) JSON.parse("{a: 'test_11981_" + i * j + "', b: '"
						+ FullTextUtils.getRandomString(16) + "', c: '" + FullTextUtils.getRandomString(16) + "', d: '"
						+ FullTextUtils.getRandomString(32) + "', e: '" + FullTextUtils.getRandomString(32) + "', f: '"
						+ FullTextUtils.getRandomString(128) + "'}"));
			}
			cl.insert(insertObjs, 0);
			insertObjs.clear();
		}
	}
}
