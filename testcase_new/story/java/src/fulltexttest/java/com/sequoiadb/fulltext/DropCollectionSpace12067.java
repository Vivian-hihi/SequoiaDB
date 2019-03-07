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
        this.sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "");
        CommLib commLib = new CommLib();
        if (commLib.isStandAlone(sdb)) {
            throw new SkipException("StandAlone environment!");
        }
        this.cs = sdb.createCollectionSpace(csName12067);
        esClient = FullTextESUtils.createTransportClient(SdbTestBase.esHostName,
                Integer.parseInt(SdbTestBase.esServiceName));
    }

    @Test
    public void test() {
        // 创建集合空间及多个集合
        String clName1 = "dropCollectionSpace12067_1";
        DBCollection cl1 = this.cs.createCollection(clName1);
        String clName2 = "dropCollectionSpace12067_2";
        DBCollection cl2 = this.cs.createCollection(clName2);
        String clName3 = "dropCollectionSpace12067_3";
        DBCollection cl3 = this.cs.createCollection(clName3);

        // 在所有集合上均创建全文索引，并插入包含索引字段的数据
        cl1.createIndex(fullIndexName,
                "{\"a\":\"text\",\"b\":\"text\",\"c\":\"text\",\"d\":\"text\",\"e\":\"text\",\"f\":\"text\"}", false,
                false);
        this.insertData(cl1, FullTextUtils.INSERT_NUMS);
        cl2.createIndex(fullIndexName,
                "{\"a\":\"text\",\"b\":\"text\",\"c\":\"text\",\"d\":\"text\",\"e\":\"text\",\"f\":\"text\"}", false,
                false);
        this.insertData(cl2, FullTextUtils.INSERT_NUMS);
        cl3.createIndex(fullIndexName,
                "{\"a\":\"text\",\"b\":\"text\",\"c\":\"text\",\"d\":\"text\",\"e\":\"text\",\"f\":\"text\"}", false,
                false);
        this.insertData(cl3, FullTextUtils.INSERT_NUMS);

        // 删除集合空间
        List<String> esIndexNames1 = FullTextDBUtils.getESIndexNames(sdb, csName12067, clName1, fullIndexName);
        FullTextUtils.checkFullSyncToES(esClient, sdb, csName12067, clName1, fullIndexName, FullTextUtils.INSERT_NUMS);
        FullTextUtils.checkConsistency(sdb, csName12067, clName1);
        List<String> esIndexNames2 = FullTextDBUtils.getESIndexNames(sdb, csName12067, clName2, fullIndexName);
        FullTextUtils.checkFullSyncToES(esClient, sdb, csName12067, clName2, fullIndexName, FullTextUtils.INSERT_NUMS);
        FullTextUtils.checkConsistency(sdb, csName12067, clName2);
        List<String> esIndexNames3 = FullTextDBUtils.getESIndexNames(sdb, csName12067, clName3, fullIndexName);
        FullTextUtils.checkFullSyncToES(esClient, sdb, csName12067, clName3, fullIndexName, FullTextUtils.INSERT_NUMS);
        FullTextUtils.checkConsistency(sdb, csName12067, clName3);
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
            Assert.assertEquals(e.getErrorCode(), -34, e.getMessage());
        } finally {
            if (sdb != null) {
                sdb.close();
            }
            if (esClient != null) {
                esClient.close();
            }
        }
    }

    public void insertData(DBCollection cl, int insertNums) {
        List<BSONObject> records = new ArrayList<BSONObject>();
        for (int i = 0; i < 100; i++) {
            for (int j = 0; j < insertNums / 100; j++) {
                BSONObject record = (BSONObject) JSON.parse("{a: 'test_12067_" + i * j + "', b: '"
                        + FullTextUtils.getRandomString(32) + "', c: '" + FullTextUtils.getRandomString(64) + "', d: '"
                        + FullTextUtils.getRandomString(64) + "', e: '" + FullTextUtils.getRandomString(128) + "', f: '"
                        + FullTextUtils.getRandomString(128) + "'}");
                records.add(record);
            }
            cl.insert(records);
            records.clear();
        }
    }
}
